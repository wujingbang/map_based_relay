
#define NS_LOG_APPEND_CONTEXT                                   \
  if (m_ipv4) { std::clog << "[node " << m_ipv4->GetObject<Node> ()->GetId () << "] "; } 

#include "god.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/wifi-mac.h"
#include "ns3/wifi-net-device.h"

NS_LOG_COMPONENT_DEFINE ("GodLocationService");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED (GodLocationService);


GodLocationService::GodLocationService (Time tableLifeTime)
{}

GodLocationService::GodLocationService ()
{}


GodLocationService::~GodLocationService ()
{}

void
GodLocationService::DoDispose ()
{
  return;
}

void
GodLocationService::Start ()
{
  return;
}


//FIX THIS
Vector
GodLocationService::GetPosition(Ipv4Address adr)
{
  uint32_t n = NodeList().GetNNodes ();
  uint32_t i;
  Ptr<Node> node;
  
  //NS_LOG_UNCOND("Position of " << adr);
  
  for(i = 0; i < n; i++)
    {
      node = NodeList().GetNode (i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

      //NS_LOG_UNCOND("Have " << ipv4->GetAddress (1, 0).GetLocal ());     
      if(ipv4->GetAddress (1, 0).GetLocal () == adr)
	{
	  return (*node->GetObject<MobilityModel>()).GetPosition ();
	}
    }
  Vector v;
  return v;
}

Vector
GodLocationService::GetPosition (Mac48Address macadr)
{
  uint32_t n = NodeList().GetNNodes ();
  uint32_t i, j;
  Ptr<Node> node;
  Ptr<WifiMac> mac;
  Ptr<NetDevice> device;
  uint32_t deviceSize;

  //NS_LOG_UNCOND("Position of " << adr);
  
  for(i = 0; i < n; i++)
    {
      node = NodeList().GetNode (i);
      deviceSize = node->GetNDevices();
      for(j = 0; j < deviceSize; j++)
      {
    	  device = node->GetDevice(j);
    	  mac = (DynamicCast<WifiNetDevice>(device))->GetMac();
    	  if(mac->GetAddress() == macadr)
    	  {
    		  return (*node->GetObject<MobilityModel>()).GetPosition ();
    	  }
      }
    }

  Vector v;
  return v;
}

  bool
  GodLocationService::HasPosition(Ipv4Address adr)
  {
    return true;
  }
  
  bool
  GodLocationService::IsInSearch(Ipv4Address adr)
  {
    return false;
  }

  void 
  GodLocationService::SetIpv4 (Ptr<Ipv4> ipv4)
  {
    return;
  }

  Vector 
  GodLocationService::GetInvalidPosition ()
  {
    return Vector(-1, -1, 0);
  }

  Time
  GodLocationService::GetEntryUpdateTime (Ipv4Address id)
  {
    return Simulator::Now ();
  }

  void 
  GodLocationService::AddEntry (Ipv4Address id, Vector position)
  {
    return;
  }

  void 
  GodLocationService::DeleteEntry (Ipv4Address id)
  {
    return;
  }
  

  void 
  GodLocationService::Purge ()
  {
    return;
  }

  void 
  GodLocationService::Clear ()
  {
    return;
  }


}
