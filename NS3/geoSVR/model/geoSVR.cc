/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * geoSVR
 *
 */
#define NS_LOG_APPEND_CONTEXT                                           \
  if (m_ipv4) { std::clog << "[node " << m_ipv4->GetObject<Node> ()->GetId () << "] "; }

#include "geoSVR.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include <algorithm>
#include <limits>
#include <iostream>
#include <stdio.h>
#include <string>


#define GEOSVR_LS_GOD 0

#define GEOSVR_LS_RLS 1

NS_LOG_COMPONENT_DEFINE ("geoSVRRoutingProtocol");

namespace ns3 {
namespace geoSVR {



struct DeferredRouteOutputTag : public Tag
{
  /// Positive if output device is fixed in RouteOutput
  uint32_t m_isCallFromL3;

  DeferredRouteOutputTag () : Tag (),
                              m_isCallFromL3 (0)
  {
  }

  static TypeId GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::geoSVR::DeferredRouteOutputTag").SetParent<Tag> ();
    return tid;
  }

  TypeId  GetInstanceTypeId () const
  {
    return GetTypeId ();
  }

  uint32_t GetSerializedSize () const
  {
    return sizeof(uint32_t);
  }

  void  Serialize (TagBuffer i) const
  {
    i.WriteU32 (m_isCallFromL3);
  }

  void  Deserialize (TagBuffer i)
  {
    m_isCallFromL3 = i.ReadU32 ();
  }

  void  Print (std::ostream &os) const
  {
    os << "DeferredRouteOutputTag: m_isCallFromL3 = " << m_isCallFromL3;
  }
};

Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();

/********** Miscellaneous constants **********/

/// Maximum allowed jitter.
#define GEOSVR_MAXJITTER          (HelloInterval.GetSeconds () / 2)
/// Random number between [(-GEOSVR_MAXJITTER)-GEOSVR_MAXJITTER] used to jitter HELLO packet transmission.

#define JITTER (Seconds (x->GetValue (-GEOSVR_MAXJITTER, GEOSVR_MAXJITTER))) 
#define FIRST_JITTER (Seconds (x->GetValue (0, GEOSVR_MAXJITTER))) //first Hello can not be in the past, used only on SetIpv4



NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

/// UDP Port for geoSVR control traffic, not defined by IANA yet
const uint32_t RoutingProtocol::GEOSVR_PORT = 666;

RoutingProtocol::RoutingProtocol ()
  : HelloInterval (Seconds (1)),
    MaxQueueLen (64),
    MaxQueueTime (Seconds (30)),
    m_queue (MaxQueueLen, MaxQueueTime),
    HelloIntervalTimer (Timer::CANCEL_ON_DESTROY),
    PerimeterMode (false),
    m_nbFromMbr (false),
    m_start (false)
{
  m_neighbors = Neighbors (false);
}

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::geoSVR::RoutingProtocol")
    .SetParent<Ipv4RoutingProtocol> ()
    .AddConstructor<RoutingProtocol> ()
    .AddAttribute ("HelloInterval", "HELLO messages emission interval.",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&RoutingProtocol::HelloInterval),
                   MakeTimeChecker ())
    .AddAttribute ("LocationServiceName", "Indicates wich Location Service is enabled",
                   EnumValue (GEOSVR_LS_GOD),
                   MakeEnumAccessor (&RoutingProtocol::LocationServiceName),
                   MakeEnumChecker (GEOSVR_LS_GOD, "GOD",
                                    GEOSVR_LS_RLS, "RLS"))
    .AddAttribute ("PerimeterMode", "Indicates if PerimeterMode is enabled",
                   BooleanValue (false),
                   MakeBooleanAccessor (&RoutingProtocol::PerimeterMode),
                   MakeBooleanChecker ())
    .AddTraceSource ("RecoveryCount",
		     "Fire when Packet go through RecoveryMode.",
		     MakeTraceSourceAccessor (&RoutingProtocol::m_recCount),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("DropPkt",
		   "Fire when Dropping Packet.",
		   MakeTraceSourceAccessor (&RoutingProtocol::m_dropPkt),
		    "ns3::Packet::TracedCallback")
  ;
  return tid;
}

RoutingProtocol::~RoutingProtocol ()
{
}

void
RoutingProtocol::DoDispose ()
{
  m_ipv4 = 0;
  Ipv4RoutingProtocol::DoDispose ();
}

Ptr<LocationService>
RoutingProtocol::GetLS ()
{
  return m_locationService;
}
void
RoutingProtocol::SetLS (Ptr<LocationService> locationService)
{
  m_locationService = locationService;
}


bool RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                  UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                  LocalDeliverCallback lcb, ErrorCallback ecb)
{

NS_LOG_FUNCTION (this << p->GetUid () << header.GetDestination () << idev->GetAddress ());
  if (m_socketAddresses.empty ())
    {
      NS_LOG_LOGIC ("No geoSVR interfaces");
      return false;
    }
  NS_ASSERT (m_ipv4 != 0);
  NS_ASSERT (p != 0);
  // Check if input device supports IP
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  int32_t iif = m_ipv4->GetInterfaceForDevice (idev);
  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();

  DeferredRouteOutputTag tag; //FIXME since I have to check if it's in origin for it to work it means I'm not taking some tag out...
  if (p->PeekPacketTag (tag) && IsMyOwnAddress (origin))
    {
      Ptr<Packet> packet = p->Copy (); //FIXME ja estou a abusar de tirar tags
      packet->RemovePacketTag(tag);
      DeferredRouteOutput (packet, header, ucb, ecb);
      return true; 
    }

  if (m_ipv4->IsDestinationAddress (dst, iif))
    {

      Ptr<Packet> packet = p->Copy ();
      TypeHeader tHeader (GEOSVRTYPE_DATAPACKET);
      packet->RemoveHeader (tHeader);
      if (!tHeader.IsValid ())
        {
          NS_LOG_DEBUG ("geoSVR message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Ignored");
          return false;
        }
      
      if (tHeader.Get () == GEOSVRTYPE_DATAPACKET)
        {
          DatapacketHeader phdr;
          packet->RemoveHeader (phdr);
        }

      lcb (packet, header, iif);
      return true;
    }
  return Forwarding (p, header, ucb, ecb);
}


void
RoutingProtocol::DeferredRouteOutput (Ptr<const Packet> p, const Ipv4Header & header,
                                      UnicastForwardCallback ucb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << p << header);
  NS_ASSERT (p != 0 && p != Ptr<Packet> ());

  if (m_queue.GetSize () == 0)
    {
      CheckQueueTimer.Cancel ();
      CheckQueueTimer.Schedule (Time ("500ms"));
    }

  QueueEntry newEntry (p, header, ucb, ecb);
  bool result = m_queue.Enqueue (newEntry);


  m_queuedAddresses.insert (m_queuedAddresses.begin (), header.GetDestination ());
  m_queuedAddresses.unique ();

  if (result)
    {
      NS_LOG_LOGIC ("Add packet " << p->GetUid () << " to queue. Protocol " << (uint16_t) header.GetProtocol ());

    }

}

void
RoutingProtocol::CheckQueue ()
{

  CheckQueueTimer.Cancel ();

  std::list<Ipv4Address> toRemove;

  for (std::list<Ipv4Address>::iterator i = m_queuedAddresses.begin (); i != m_queuedAddresses.end (); ++i)
    {
      if (SendPacketFromQueue (*i))
        {
          //Insert in a list to remove later
          toRemove.insert (toRemove.begin (), *i);
        }
    }

  //remove all that are on the list
  for (std::list<Ipv4Address>::iterator i = toRemove.begin (); i != toRemove.end (); ++i)
    {
      m_queuedAddresses.remove (*i);
    }

  if (!m_queuedAddresses.empty ()) //Only need to schedule if the queue is not empty
    {
      CheckQueueTimer.Schedule (Time ("500ms"));
    }
}

bool
RoutingProtocol::SendPacketFromQueue (Ipv4Address dst)
{
  NS_LOG_FUNCTION (this);
  //bool recovery = false;
  QueueEntry queueEntry;

  if (m_locationService->IsInSearch (dst))
    {
      return false;
    }

  if (!m_locationService->HasPosition (dst)) // Location-service stoped looking for the dst
    {
      m_queue.DropPacketWithDst (dst);
      NS_LOG_DEBUG ("Location Service did not find dst. Drop packet to " << dst);
      return true;
    }

  while(m_queue.Dequeue (dst, queueEntry))
  {
    Ptr<Packet> p = ConstCast<Packet> (queueEntry.GetPacket ());
    Ipv4Header header = queueEntry.GetIpv4Header ();
    UnicastForwardCallback ucb = queueEntry.GetUnicastForwardCallback ();
    ErrorCallback ecb = queueEntry.GetErrorCallback ();
    if(!Forwarding (p, header, ucb, ecb))
      return false;
  }
  return true;
}


void
RoutingProtocol::NotifyInterfaceUp (uint32_t interface)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (interface, 0).GetLocal ());
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (l3->GetNAddresses (interface) > 1)
    {
      NS_LOG_WARN ("geoSVR does not work with more then one address per each interface.");
    }
  Ipv4InterfaceAddress iface = l3->GetAddress (interface, 0);
  if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
    {
      return;
    }

  // Create a socket to listen only on this interface
  Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                             UdpSocketFactory::GetTypeId ());
  NS_ASSERT (socket != 0);
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvHello, this));
  socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), GEOSVR_PORT));
  socket->BindToNetDevice (l3->GetNetDevice (interface));
  socket->SetAllowBroadcast (true);
  socket->SetAttribute ("IpTtl", UintegerValue (1));
  m_socketAddresses.insert (std::make_pair (socket, iface));


  // Allow neighbor manager use this interface for layer 2 feedback if possible
  Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
  Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
  if (wifi == 0)
    {
      return;
    }
  Ptr<WifiMac> mac = wifi->GetMac ();
  if (mac == 0)
    {
      return;
    }

  mac->TraceConnectWithoutContext ("TxErrHeader", m_neighbors.GetTxErrorCallback ());

}


void
RoutingProtocol::RecvHello (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Address sourceAddress;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddress);

  TypeHeader tHeader (GEOSVRTYPE_HELLO);
  packet->RemoveHeader (tHeader);
  if (!tHeader.IsValid ())
    {
      NS_LOG_DEBUG ("geoSVR message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Ignored");
      return;
    }

  HelloHeader hdr;

  packet->RemoveHeader (hdr);
  struct msvr_ninfo neighbor;
  neighbor.x = hdr.GetX ();
  neighbor.y = hdr.GetY ();
  neighbor.speed = hdr.GetS ();
  neighbor.heading = hdr.GetH ();
  neighbor.edgeid = m_map.getRoadByPos(neighbor.x, neighbor.y).id_;
  neighbor.dst = hdr.GetDst ();

  //InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
  //Ipv4Address sender = inetSourceAddr.GetIpv4 ();
  //Ipv4Address receiver = m_socketAddresses[socket].GetLocal ();

  //Ptr<Node> n = this->GetObject<Node>();
  //NS_LOG_LOGIC ("Node " << n->GetId() << " Recv Hello From " << Position );
  m_neighbors.Update (&neighbor);
  //m_neighbors.Print();

}


void
RoutingProtocol::NotifyInterfaceDown (uint32_t interface)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (interface, 0).GetLocal ());

  // Disable layer 2 link state monitoring (if possible)
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  Ptr<NetDevice> dev = l3->GetNetDevice (interface);
  Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
  if (wifi != 0)
    {
      Ptr<WifiMac> mac = wifi->GetMac ()->GetObject<AdhocWifiMac> ();
      if (mac != 0)
        {
          mac->TraceDisconnectWithoutContext ("TxErrHeader",
                                              m_neighbors.GetTxErrorCallback ());
        }
    }

  // Close socket
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (m_ipv4->GetAddress (interface, 0));
  NS_ASSERT (socket);
  socket->Close ();
  m_socketAddresses.erase (socket);
  if (m_socketAddresses.empty ())
    {
      NS_LOG_LOGIC ("No geoSVR interfaces");
      m_neighbors.Clear ();
      m_locationService->Clear ();
      return;
    }
}


Ptr<Socket>
RoutingProtocol::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const
{
  NS_LOG_FUNCTION (this << addr);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      if (iface == addr)
        {
          return socket;
        }
    }
  Ptr<Socket> socket;
  return socket;
}


void RoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << " interface " << interface << " address " << address);
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (!l3->IsUp (interface))
    {
      return;
    }
  if (l3->GetNAddresses ((interface) == 1))
    {
      Ipv4InterfaceAddress iface = l3->GetAddress (interface, 0);
      Ptr<Socket> socket = FindSocketWithInterfaceAddress (iface);
      if (!socket)
        {
          if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
            {
              return;
            }
          // Create a socket to listen only on this interface
          Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                                     UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvHello,this));
          // Bind to any IP address so that broadcasts can be received
          socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), GEOSVR_PORT));
          socket->BindToNetDevice (l3->GetNetDevice (interface));
          socket->SetAllowBroadcast (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));

          Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
        }
    }
  else
    {
      NS_LOG_LOGIC ("geoSVR does not work with more then one address per each interface. Ignore added address");
    }
}

void
RoutingProtocol::NotifyRemoveAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this);
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (address);
  if (socket)
    {

      m_socketAddresses.erase (socket);
      Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
      if (l3->GetNAddresses (i))
        {
          Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
          // Create a socket to listen only on this interface
          Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                                     UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvHello, this));
          // Bind to any IP address so that broadcasts can be received
          socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), GEOSVR_PORT));
          socket->SetAllowBroadcast (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));

          // Add local broadcast record to the routing table
          Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));

        }
      if (m_socketAddresses.empty ())
        {
          NS_LOG_LOGIC ("No geoSVR interfaces");
          m_neighbors.Clear ();
          m_locationService->Clear ();
          return;
        }
    }
  else
    {
      NS_LOG_LOGIC ("Remove address not participating in geoSVR operation");
    }
}

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_ASSERT (ipv4 != 0);
  NS_ASSERT (m_ipv4 == 0);

  m_ipv4 = ipv4;

  //Schedule only when it has packets on queue
  CheckQueueTimer.SetFunction (&RoutingProtocol::CheckQueue, this);

  HelloIntervalTimer.SetFunction (&RoutingProtocol::HelloTimerExpire, this);
  if (!m_nbFromMbr)
    {
      HelloIntervalTimer.Schedule (FIRST_JITTER);
    }
  if (!m_start)
    {
      Simulator::ScheduleNow (&RoutingProtocol::Start, this);
      m_start = true;
    }
  m_neighbors.setNode(m_ipv4->GetObject<Node> ());
}

void
RoutingProtocol::HelloTimerExpire ()
{
  if (m_nbFromMbr)
    return;

  SendHello ();
  HelloIntervalTimer.Cancel ();
  HelloIntervalTimer.Schedule (HelloInterval + JITTER);
}

void
RoutingProtocol::SendHello ()
{
  NS_LOG_FUNCTION (this);

  double x, y, s, h;
 // m_ipv4->GetAddress(1,0).GetLocal ();
  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();
  x = MM->GetPosition ().x;
  y = MM->GetPosition ().y;
  s = 0;
  h = 0;
  //还要加上速度和方向；

  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      HelloHeader helloHeader (x, y, s, h, m_ipv4->GetAddress(1,0).GetLocal ());

      Ptr<Packet> packet = Create<Packet> ();
      packet->AddHeader (helloHeader);
      TypeHeader tHeader (GEOSVRTYPE_HELLO);
      packet->AddHeader (tHeader);
      // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
      Ipv4Address destination;
      if (iface.GetMask () == Ipv4Mask::GetOnes ())
        {
          destination = Ipv4Address ("255.255.255.255");
        }
      else
        {
          destination = iface.GetBroadcast ();
        }
      socket->SendTo (packet, 0, InetSocketAddress (destination, GEOSVR_PORT));

    }
}

bool
RoutingProtocol::IsMyOwnAddress (Ipv4Address src)
{
  NS_LOG_FUNCTION (this << src);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      if (src == iface.GetLocal ())
        {
          return true;
        }
    }
  return false;
}


void
RoutingProtocol::Start ()
{
  NS_LOG_FUNCTION (this);
  m_queuedAddresses.clear ();

  //FIXME ajustar timer, meter valor parametrizavel
  Time tableTime ("2s");

  switch (LocationServiceName)
    {
    case GEOSVR_LS_GOD:
      NS_LOG_DEBUG ("GodLS in use");
      m_locationService = CreateObject<GodLocationService> ();
      break;
    case GEOSVR_LS_RLS:
      NS_LOG_UNCOND ("RLS not yet implemented");
      break;
    }

}

Ptr<Ipv4Route>
RoutingProtocol::LoopbackRoute (const Ipv4Header & hdr, Ptr<NetDevice> oif)
{
  NS_LOG_FUNCTION (this << hdr);
  m_lo = m_ipv4->GetNetDevice (0);
  NS_ASSERT (m_lo != 0);
  Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
  rt->SetDestination (hdr.GetDestination ());

  std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
  if (oif)
    {
      // Iterate to find an address on the oif device
      for (j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
        {
          Ipv4Address addr = j->second.GetLocal ();
          int32_t interface = m_ipv4->GetInterfaceForAddress (addr);
          if (oif == m_ipv4->GetNetDevice (static_cast<uint32_t> (interface)))
            {
              rt->SetSource (addr);
              break;
            }
        }
    }
  else
    {
      rt->SetSource (j->second.GetLocal ());
    }
  NS_ASSERT_MSG (rt->GetSource () != Ipv4Address (), "Valid geoSVR source address not found");
  rt->SetGateway (Ipv4Address ("127.0.0.1"));
  rt->SetOutputDevice (m_lo);
  return rt;
}


int
RoutingProtocol::GetProtocolNumber (void) const
{
  return GEOSVR_PORT;
}

bool IsBoardcast(Ipv4Address ip)
{
  //FIXME: dirty way!
  uint32_t tip = ip.Get() & 0xff;
  if (tip == 0xff)
    return true;
  else
    return false;
}

void
RoutingProtocol::AddHeaders (Ptr<Packet> p, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
{

//  NS_LOG_DEBUG (this << " source " << source << " destination " << destination);
  Vector srcPos, dstPos;
 
  if(!IsBoardcast(destination) )
  {
    NS_LOG_DEBUG (this << " source " << source << " destination " << destination);
    Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();
    srcPos.x = MM->GetPosition ().x;
    srcPos.y = MM->GetPosition ().y;
    dstPos = m_locationService->GetPosition (destination);
  }
  else
  {
    srcPos.x = 0;
    srcPos.y = 0;
    dstPos.x = 0;
    dstPos.y = 0;
  }
  uint32_t length, m = 1, t = 2, c = 1;
  uint8_t  *enpaths = NULL;
  geoSVR::geoSVRTag tag;
  bool findtag = p->FindFirstMatchingByteTag(tag);
  if(findtag)
  {
    tag.GetPath(&enpaths);
    p->RemoveAllByteTags();
  }
  if (enpaths == NULL) //MBR
    length = 0;
  else
    length = (uint32_t)(enpaths[0]);

  DatapacketHeader hdr (length, m, t, c, srcPos.x, srcPos.y, dstPos.x, dstPos.y, enpaths);
  p->AddHeader (hdr);
  TypeHeader tHeader (GEOSVRTYPE_DATAPACKET);
  p->AddHeader (tHeader);

  m_downTarget (p, source, destination, protocol, route);

}

bool
RoutingProtocol::Forwarding (Ptr<const Packet> packet, const Ipv4Header & header,
                             UnicastForwardCallback ucb, ErrorCallback ecb)
{
  Ptr<Packet> p = packet->Copy ();
  NS_LOG_FUNCTION (this);
  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();

  Vector srcPos, dstPos, relayPos, nextPos;

  TypeHeader tHeader (GEOSVRTYPE_DATAPACKET);
  DatapacketHeader hdr;
  p->RemoveHeader (tHeader);
  if (!tHeader.IsValid ())
    {
      NS_LOG_DEBUG ("geoSVR message " << p->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
      return false;     // drop
    }
  if (tHeader.Get () == GEOSVRTYPE_DATAPACKET)
    {

      p->RemoveHeader (hdr);
      srcPos.x = hdr.GetSx ();
      srcPos.y = hdr.GetSy ();
      dstPos.x = hdr.GetDx ();
      dstPos.y = hdr.GetDy ();
      Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();
      relayPos.x = MM->GetPosition ().x;
      relayPos.y = MM->GetPosition ().y;

    }

  Ipv4Address nextHop;

  if(m_neighbors.isNeighbor (dst))
    {
      NS_LOG_DEBUG(dst << " is neighbor !");
      nextHop = dst;
      p->AddHeader (hdr);
      p->AddHeader (tHeader);

      Ptr<NetDevice> oif = m_ipv4->GetObject<NetDevice> ();
      Ptr<Ipv4Route> route = Create<Ipv4Route> ();
      route->SetDestination (dst);
      route->SetSource (header.GetSource ());
      route->SetGateway (nextHop);

      // FIXME: Does not work for multiple interfaces
      route->SetOutputDevice (m_ipv4->GetNetDevice (1));
      route->SetDestination (header.GetDestination ());
      NS_ASSERT (route != 0);
      NS_LOG_DEBUG ("Exist route to " << route->GetDestination () << " from interface " << route->GetOutputDevice ());
      NS_LOG_LOGIC (route->GetOutputDevice () << " forwarding to " << dst << " from " << origin << " through " << route->GetGateway () << " packet " << p->GetUid ());

      ucb (route, p, header);
      return true;
    }
  else
    {
      std::vector<int> paths;
      hdr.decode_path(paths);

      if (paths.size() > 2 &&  m_map.getRoadByPos(relayPos.x, relayPos.y).id_ ==  m_map.getRoadByNode(paths[0], paths[1]))
           paths.erase(paths.begin());

      if (paths.size() < 2)
      {
	NS_LOG_DEBUG ("path size is less than 2!");
    	return false;
      }
      int roadid1 = m_map.getRoadByNode(paths[0], paths[1]), roadid2;
      int myroad = m_map.getRoadByPos(relayPos.x, relayPos.y).id_;
      if(myroad == roadid1 && paths.size() > 2)
      {
        roadid2 = m_map.getRoadByNode(paths[1], paths[2]);
        nextPos.x = m_map.getMap()[paths[2]].x_;
        nextPos.y = m_map.getMap()[paths[2]].y_;
      }
      else
      {
        roadid2 = -1;
        nextPos.x = m_map.getMap()[paths[1]].x_;
        nextPos.y = m_map.getMap()[paths[1]].y_;
      }
      int roadid3 = m_map.getRoadByPos(relayPos.x, relayPos.y).id_;
      nextHop = m_neighbors.find_next_hop (roadid1, roadid2, roadid3, relayPos.x, relayPos.y, nextPos.x, nextPos.y);
//      nextHop = m_neighbors.find_furthest_nhop(roadid1, roadid2,relayPos.x, relayPos.y);
      if (nextHop != Ipv4Address::GetZero ())
        {
          NS_LOG_DEBUG ("Destination: " << dst);
          
//          if (m_map.getRoadByNode(paths[0], paths[1]) !=
//              m_map.getRoadByPos(nextPos.x, nextPos.y).id_ &&
//              m_map.getRoadByPos(relayPos.x, relayPos.y).id_ ==
//              m_map.getRoadByNode(paths[0], paths[1]))

          hdr.encode_path(paths);
          p->AddHeader (hdr);
          p->AddHeader (tHeader);

          Ptr<NetDevice> oif = m_ipv4->GetObject<NetDevice> ();
          Ptr<Ipv4Route> route = Create<Ipv4Route> ();
          route->SetDestination (dst);
          route->SetSource (header.GetSource ());
          route->SetGateway (nextHop);

          // FIXME: Does not work for multiple interfaces
          route->SetOutputDevice (m_ipv4->GetNetDevice (1));
          route->SetDestination (header.GetDestination ());
          NS_ASSERT (route != 0);
          NS_LOG_DEBUG ("Exist route to " << route->GetDestination () << " from interface " << route->GetOutputDevice ());
          NS_LOG_DEBUG (route->GetOutputDevice () << " forwarding to " << dst << " from " << origin << " through " << route->GetGateway () << " packet " << p->GetUid ());
          ucb (route, p, header);

          return true;
        }

      else
      {
	  NS_LOG_DEBUG( "forwarding fail! ");
          for(std::vector<int>::iterator iter = paths.begin(); iter != paths.end(); ++iter)
            NS_LOG_DEBUG(*iter << ' ');
          NS_LOG_DEBUG( m_map.getRoadByPos(relayPos.x, relayPos.y).id_ );
          NS_LOG_DEBUG("srcPos: "<<srcPos.x<<' '<< srcPos.y<<" dstPos: "
		       <<dstPos.x<<' '<<dstPos.y<<" relayPos: " << relayPos.x << relayPos.y);
      	  m_neighbors.Print();
      	  NS_LOG_DEBUG(origin << "  "<< dst << "  " << m_ipv4->GetAddress (1, 0).GetLocal ());
      }
    }
    return false;
}



void
RoutingProtocol::SetDownTarget (IpL4Protocol::DownTargetCallback callback)
{
  m_downTarget = callback;
}


IpL4Protocol::DownTargetCallback
RoutingProtocol::GetDownTarget (void) const
{
  return m_downTarget;
}


Ptr<Ipv4Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
                              Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_FUNCTION (this << header << (oif ? oif->GetIfIndex () : 0));

  if (!p)
    {
      return LoopbackRoute (header, oif);     // later
    }
  if (m_socketAddresses.empty ())
    {
      sockerr = Socket::ERROR_NOROUTETOHOST;
      NS_LOG_LOGIC ("No geoSVR interfaces");
      Ptr<Ipv4Route> route;
      return route;
    }
  sockerr = Socket::ERROR_NOTERROR;
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  Ipv4Address dst = header.GetDestination ();
//  Ipv4Address src = header.GetSource ();
  Ipv4Address nextHop;
  Vector srcPos, dstPos, relayPos, nextPos;

//  if (!(dst == m_ipv4->GetAddress (1, 0).GetBroadcast ()))
// 广播包的处理
  if (IsBoardcast(dst))
    {
      geoSVR::geoSVRTag tag;
      tag.SetPath(NULL);
      p->AddByteTag(tag);

      route->SetDestination (dst);
      if (header.GetSource () == Ipv4Address ("102.102.102.102"))
        {
          route->SetSource (m_ipv4->GetAddress (1, 0).GetLocal ());
        }
      else
        {
          route->SetSource (header.GetSource ());
        }
      route->SetGateway (dst);
      route->SetOutputDevice (m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (route->GetSource ())));

      NS_ASSERT (route != 0);
      if (oif != 0 && route->GetOutputDevice () != oif)
        {
          NS_LOG_DEBUG ("Output device doesn't match. Dropped.");
          sockerr = Socket::ERROR_NOROUTETOHOST;
          return Ptr<Ipv4Route> ();
        }
      return route;
    }

    dstPos = m_locationService->GetPosition (dst);

  //环回包的处理
  if (CalculateDistance (dstPos, m_locationService->GetInvalidPosition ()) == 0 && m_locationService->IsInSearch (dst))
    {
      DeferredRouteOutputTag tag;
      if (!p->PeekPacketTag (tag))
        {
          p->AddPacketTag (tag);
        }
      return LoopbackRoute (header, oif);
    }

  Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();
  srcPos.x = MM->GetPosition ().x;
  srcPos.y = MM->GetPosition ().y;
  relayPos.x = srcPos.x;
  relayPos.y = srcPos.y;

  //srcPos = m_locationService->GetPosition (src);
  DatapacketHeader hdr;
  uint8_t *enpaths = NULL;
  std::vector<int> paths;

  if(m_neighbors.isNeighbor (dst))
    {
      nextHop = dst;
      NS_LOG_DEBUG (dst << " is neighbor!");
    }
  else
    {
      paths = m_map.getPaths(srcPos.x, srcPos.y, dstPos.x, dstPos.y);

      if (paths.size() > 2 &&  m_map.getRoadByPos(relayPos.x, relayPos.y).id_ ==  m_map.getRoadByNode(paths[0], paths[1]))
           paths.erase(paths.begin());

      int roadid1 = m_map.getRoadByNode(paths[0], paths[1]), roadid2;
      int myroad = m_map.getRoadByPos(relayPos.x, relayPos.y).id_;
      if(myroad == roadid1 && paths.size() > 2)
      {
    	roadid2 = m_map.getRoadByNode(paths[1], paths[2]);
        nextPos.x = m_map.getMap()[paths[2]].x_;
        nextPos.y = m_map.getMap()[paths[2]].y_;
      } 
      else
      {
    	roadid2 = -1;
    	nextPos.x = m_map.getMap()[paths[1]].x_;
    	nextPos.y = m_map.getMap()[paths[1]].y_;
      }

      int roadid3 = m_map.getRoadByPos(relayPos.x, relayPos.y).id_;
      nextHop = m_neighbors.find_next_hop (roadid1, roadid2, roadid3, srcPos.x, srcPos.y, nextPos.x, nextPos.y);
      //nextHop = m_neighbors.find_furthest_nhop(roadid1, roadid2,srcPos.x, srcPos.y);
      if (nextHop != Ipv4Address::GetZero ())
      {

        NS_LOG_DEBUG ("Destination: " << dst);
//        if (m_map.getRoadByNode(paths[0], paths[1]) !=
//            m_map.getRoadByPos(nextPos.x, nextPos.y).id_ &&
//            m_map.getRoadByPos(relayPos.x, relayPos.y).id_ ==
//            m_map.getRoadByNode(paths[0], paths[1]))
//            paths.erase(paths.begin());

            hdr.encode_path(paths); 
            enpaths = hdr.GetPath();
      }
    }

    if (nextHop != Ipv4Address::GetZero ())
    {

      geoSVR::geoSVRTag tag;
      tag.SetPath(enpaths);
      p->AddByteTag(tag);

      route->SetDestination (dst);
      if (header.GetSource () == Ipv4Address ("102.102.102.102"))
        {
          route->SetSource (m_ipv4->GetAddress (1, 0).GetLocal ());
        }
      else
        {
          route->SetSource (header.GetSource ());
        }
      route->SetGateway (nextHop);
      route->SetOutputDevice (m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (route->GetSource ())));
      route->SetDestination (header.GetDestination ());
      NS_ASSERT (route != 0);
      NS_LOG_DEBUG ("Exist route to " << route->GetDestination () << " from interface " << route->GetSource ());
      NS_LOG_DEBUG (this <<" nexthop "<< nextHop << " Location: "<<m_locationService->GetPosition(nextHop));
      if (oif != 0 && route->GetOutputDevice () != oif)
        {
          NS_LOG_DEBUG ("Output device doesn't match. Dropped.");
          sockerr = Socket::ERROR_NOROUTETOHOST;
          return Ptr<Ipv4Route> ();
        }
      return route;
    }
  else
    {
      NS_LOG_DEBUG("there is no nexthop");
      DeferredRouteOutputTag tag;
      if (!p->PeekPacketTag (tag))
        {
          p->AddPacketTag (tag);
        }

      NS_LOG_DEBUG( "output fail!");
      for(std::vector<int>::iterator iter = paths.begin(); iter != paths.end(); ++iter)
	NS_LOG_DEBUG(*iter << ' ');

      NS_LOG_DEBUG("srcPos: "<<srcPos.x<<' '<< srcPos.y<<" dstPos: "
		       <<dstPos.x<<' '<<dstPos.y<<" relayPos: " << relayPos.x << relayPos.y);
      m_neighbors.Print();
      NS_LOG_DEBUG(m_ipv4->GetAddress (1, 0).GetLocal () << "  "<< dst);
      return LoopbackRoute (header, oif);
    }

}

}
}
