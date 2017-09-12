/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/gpsr-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/udp-echo-server.h"
#include "ns3/udp-echo-client.h"
#include "ns3/udp-echo-helper.h"

#include "ns3/applications-module.h"
#include "ns3/itu-r-1411-los-propagation-loss-model.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include "ns3/integer.h"
//#include "ns3/wave-bsm-helper.h"
#include "ns3/wave-helper.h"
#include "ns3/topology.h"

#include "ns3/mbr-neighbor-helper.h"
#include "ns3/flow-monitor-helper.h"

#include "ns3/aodv-module.h"

#include <iostream>
#include <cmath>

using namespace ns3;
using namespace mbr;

#define AODV 1
#define GPSR 2

NS_LOG_COMPONENT_DEFINE ("gpsr-mbr-test");

void ReceivePacket (Ptr<Socket> socket)
{
  NS_LOG_UNCOND ("Received one packet!");
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                                      socket, pktSize,pktCount-1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}


class GpsrExample
{
public:
  GpsrExample ();
  /// Configure script parameters, \return true on successful configuration
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /// Report results
  void Report (std::ostream & os);

private:
  uint32_t bytesTotal;
  uint32_t packetsReceived;
  ///\name parameters
  //\{
  /// Number of nodes
  uint32_t m_nNodes;

  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  //\}
  uint32_t m_lossModel;
  std::string m_lossModelName;
  std::string m_phyMode;
  double m_txp;
  uint32_t m_pktSize;

  std::string m_traceFile;
  bool m_loadBuildings;
  uint32_t m_nSinks;

  uint32_t m_port;
  int m_mobility;
  int m_scenario;

  bool m_mbr;
  std::string m_netFileString;
  bool m_openRelay;
  int32_t m_routingProtocol;
  std::string m_routingProtocolStr;

  NodeContainer m_nodesContainer;
  NetDeviceContainer beaconDevices;
  Ipv4InterfaceContainer beaconInterfaces;
  NetDeviceContainer dataDevices;
  Ipv4InterfaceContainer dataInterfaces;

  MbrNeighborHelper m_MbrNeighborHelper;


private:
  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
  void SetupScenario();
  void SetupAdhocMobilityNodes();
  void SetupRoutingMessages (NodeContainer & c,
                             Ipv4InterfaceContainer & adhocTxInterfaces);
  Ptr<Socket> SetupRoutingPacketReceive (Ipv4Address addr, Ptr<Node> node);
  void ReceiveRoutingPacket (Ptr<Socket> socket);
  void CheckThroughput ();
};

int main (int argc, char **argv)
{
  GpsrExample test;
  if (! test.Configure(argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();
  test.Report (std::cout);
  return 0;
}

//-----------------------------------------------------------------------------
GpsrExample::GpsrExample () :
  bytesTotal(0),
  packetsReceived(0),
  // Number of Nodes
  m_nNodes (100),

  // Simulation time
  totalTime (30),
  // Generate capture files for each node
  pcap (true),
  m_lossModel (3),
  m_lossModelName (""),
  m_phyMode ("OfdmRate12MbpsBW10MHz"),
  m_txp (10),
  m_pktSize (1400),
  m_traceFile(""),
  m_loadBuildings(true),
  m_nSinks(1),
  m_port(9),
  m_mobility(2),
  m_scenario(2),
  m_mbr(false),
  m_netFileString(""),
  m_openRelay(false),
  m_routingProtocol(AODV)
{
}

bool
GpsrExample::Configure (int argc, char **argv)
{
  // Enable GPSR logs by default. Comment this if too noisy
  // LogComponentEnable("GpsrRoutingProtocol", LOG_LEVEL_ALL);

  SeedManager::SetSeed(12345);
  CommandLine cmd;

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("size", "Number of nodes.", m_nNodes);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);

  cmd.AddValue ("txp", "tx power db", m_txp);
  cmd.AddValue ("phyMode", "Wifi Phy mode for Data channel", m_phyMode);
  cmd.AddValue ("pktsize", "udp packet size", m_pktSize);

  cmd.AddValue ("buildings", "Load building (obstacles)", m_loadBuildings);
  cmd.AddValue ("sinks", "Number of routing sinks", m_nSinks);

  cmd.AddValue ("scen", "scenario", m_scenario);
  cmd.AddValue ("relay", "open relay", m_openRelay);
  cmd.AddValue ("mbrnb", "use MBR Neighbor", m_mbr);

  cmd.AddValue ("routing", "name of routing protocol", m_routingProtocolStr);

  cmd.Parse (argc, argv);

  if (m_routingProtocolStr == "aodv")
    m_routingProtocol = AODV;
  else if (m_routingProtocolStr == "gpsr")
    m_routingProtocol = GPSR;
  else
    {
      NS_LOG_UNCOND("Routing Protocol ERROR !!!!!!!!!!!");
      return false;
    }

  return true;
}

void
GpsrExample::Run ()
{
  //Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  SetupScenario();

  CreateNodes ();
  SetupAdhocMobilityNodes();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();

  if(m_routingProtocol == GPSR)
    {
      GpsrHelper gpsr;
      gpsr.Install (m_mbr);
    }

  SetupRoutingMessages(m_nodesContainer, dataInterfaces);
  
  //	Flow	monitor
//  Ptr<FlowMonitor>	flowMonitor;
//  FlowMonitorHelper	flowHelper;
//  flowMonitor = flowHelper.InstallAll();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  CheckThroughput();

  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();

//  flowMonitor->SerializeToXmlFile("gpsr-mbr-test-flow.xml",true,true);

  Simulator::Destroy ();
}

void
GpsrExample::Report (std::ostream &)
{
}

void
GpsrExample::CreateNodes ()
{
  std::cout << "Creating " << (unsigned)m_nNodes << " nodes " << "\n";
  m_nodesContainer.Create (m_nNodes);
  // Name nodes
  for (uint32_t i = 0; i < m_nNodes; ++i)
     {
       std::ostringstream os;
       // Set the Node name to the corresponding IP host address
       os << "node-" << i+1;
       Names::Add (os.str (), m_nodesContainer.Get (i));
     }

}

void
GpsrExample::CreateDevices ()
{
  if (m_lossModel == 1)
    {
      m_lossModelName = "ns3::FriisPropagationLossModel";
    }
  else if (m_lossModel == 2)
    {
      m_lossModelName = "ns3::ItuR1411LosPropagationLossModel";
    }
  else if (m_lossModel == 3)
    {
      m_lossModelName = "ns3::TwoRayGroundPropagationLossModel";
    }
  else if (m_lossModel == 4)
    {
      m_lossModelName = "ns3::LogDistancePropagationLossModel";
    }
  else
    {
      // Unsupported propagation loss model.
      // Treating as ERROR

    }
  double freq = 5.9e9;

//  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
//  wifiMac.SetType ("ns3::AdhocWifiMac");
//  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
//  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
//  wifiPhy.SetChannel (wifiChannel.Create ());
//  WifiHelper wifi;
//  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
//  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("DsssRate11Mbps"), "RtsCtsThreshold", UintegerValue (1560));
//  beaconDevices = wifi.Install (wifiPhy, wifiMac, m_nodesContainer);

  /**
   * Data channel
   */

  YansWifiChannelHelper wifiChannel2;
  wifiChannel2.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  if (m_lossModel == 3)
    {
      // two-ray requires antenna height (else defaults to Friss)
      wifiChannel2.AddPropagationLoss (m_lossModelName, "Frequency", DoubleValue (freq), "HeightAboveZ", DoubleValue (1.5));
    }
  else
    {
      wifiChannel2.AddPropagationLoss (m_lossModelName, "Frequency", DoubleValue (freq));
    }

  if (m_loadBuildings == 1) {
    wifiChannel2.AddPropagationLoss ("ns3::ObstacleShadowingPropagationLossModel", "ForBeacon", UintegerValue(0));
    //wifiChannel2.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
  }
  else
    wifiChannel2.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
  Ptr<YansWifiChannel> channel = wifiChannel2.Create ();
  YansWifiPhyHelper wifiPhy2 =  YansWifiPhyHelper::Default ();
  wifiPhy2.SetChannel (channel);
  wifiPhy2.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();

  // Setup 802.11p stuff
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                       "DataMode",StringValue (m_phyMode),
                                       "ControlMode",StringValue (m_phyMode),
				       "NonUnicastMode", StringValue ("OfdmRate3MbpsBW10MHz"));
  // Set Tx Power
  wifiPhy2.Set ("TxPowerStart",DoubleValue (m_txp));
  wifiPhy2.Set ("TxPowerEnd", DoubleValue (m_txp));

  dataDevices = wifi80211p.Install (wifiPhy2, wifi80211pMac, m_nodesContainer);

//  WifiHelper wifi;
//  WifiMacHelper wifiMac;
//  wifiMac.SetType ("ns3::AdhocWifiMac");
//  YansWifiPhyHelper wifiPhydata = YansWifiPhyHelper::Default ();
//  YansWifiChannelHelper wifiChanneldata = YansWifiChannelHelper::Default ();
//  wifiPhydata.SetChannel (wifiChanneldata.Create ());
//  dataDevices = wifi.Install (wifiPhydata, wifiMac, m_nodesContainer);
//



  /**
    * Beacon channel
    */
  if (m_mbr)
    {
      YansWifiChannelHelper wifiChannel;
      wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
      // two-ray requires antenna height (else defaults to Friss)
      wifiChannel.AddPropagationLoss (m_lossModelName, "Frequency", DoubleValue (freq), "HeightAboveZ", DoubleValue (1.5));
      wifiChannel.AddPropagationLoss ("ns3::ObstacleShadowingPropagationLossModel", "ForBeacon", UintegerValue(0), "IsSub1G", UintegerValue(0));
      Ptr<YansWifiChannel> channel0 = wifiChannel.Create ();
      YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
      wifiPhy.SetChannel (channel0);
      wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
      NqosWaveMacHelper wifi80211pMacBeacon = NqosWaveMacHelper::Default ();
      Wifi80211pHelper wifi80211pBeacon = Wifi80211pHelper::Default ();

      // Setup 802.11p stuff
      wifi80211pBeacon.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
					  "DataMode",StringValue ("OfdmRate3MbpsBW10MHz"),
					  "ControlMode",StringValue ("OfdmRate3MbpsBW10MHz"),
					  "NonUnicastMode", StringValue ("OfdmRate3MbpsBW10MHz"));
      // Set Tx Power
      wifiPhy.Set ("TxPowerStart",DoubleValue (m_txp));
      wifiPhy.Set ("TxPowerEnd", DoubleValue (m_txp));

      beaconDevices = wifi80211pBeacon.Install (wifiPhy, wifi80211pMacBeacon, m_nodesContainer);
    }



  // Enable Captures, if necessary
  if (pcap)
    {
//      wifiPhy.EnablePcapAll (std::string ("gpsr-pcap"));
//      wifiPhy2.EnablePcapAll (std::string ("gpsr-pcap"));
    }

}

void
GpsrExample::InstallInternetStack ()
{
  InternetStackHelper stack;
  switch (m_routingProtocol)
  {
    case (AODV):
    {
      AodvHelper aodv;
      // you can configure AODV attributes here using aodv.Set(name, value)
      stack.SetRoutingHelper (aodv); // has effect on the next Install ()
      stack.Install (m_nodesContainer);
      break;
    }
    case (GPSR):
    {
      GpsrHelper gpsr;
      // you can configure GPSR attributes here using gpsr.Set(name, value)
      stack.SetRoutingHelper (gpsr);
      stack.Install (m_nodesContainer);
      break;
    }
  }


  Ipv4AddressHelper addressAdhocData;
  addressAdhocData.SetBase ("10.1.0.0", "255.255.0.0");
  dataInterfaces = addressAdhocData.Assign (dataDevices);

  Ipv4AddressHelper address;
  address.SetBase ("10.2.0.0", "255.255.0.0");
  beaconInterfaces = address.Assign (beaconDevices);
}

void
GpsrExample::InstallApplications ()
{
  if (!m_mbr)
    return;

  m_MbrNeighborHelper.Install(beaconInterfaces,
			      dataInterfaces,
			      beaconDevices,
			      dataDevices,
			      Seconds (totalTime),//Seconds(4),//
			      100,//m_wavePacketSize,
			      Seconds (0.1),//m_waveInterval
			      // GPS accuracy (i.e, clock drift), in number of ns
			      40,//m_gpsAccuracyNs,
			      // tx max delay before transmit, in ms
			      MilliSeconds (10),//m_txMaxDelayMs
			      m_netFileString,
			      m_openRelay);
  // fix random number streams
  m_MbrNeighborHelper.AssignStreams (m_nodesContainer, 0);


}

void
GpsrExample::SetupScenario()
{
  if (m_scenario == 1)
    {
      m_traceFile = "src/wave/examples/Raleigh_Downtown50.ns2";
      m_lossModel = 3; // two-ray ground
      m_nSinks = 10;
      m_nNodes = 50;
      totalTime = 30;
      m_mobility = 1;
      if (m_loadBuildings != 0)
	{
	  std::string bldgFile = "src/wave/examples/Raleigh_Downtown.buildings.xml";
	  NS_LOG_UNCOND ("Loading buildings file " << bldgFile);
	  Topology::LoadBuildings(bldgFile);
	}
    }
  else if (m_scenario == 2)
    {
      m_mobility = 2; //static relay
      m_nNodes = 3;
      totalTime = 10;
      m_nSinks = 1;
      m_lossModel = 3; // two-ray ground

      //m_mbr = true;
      m_netFileString = "/home/wu/workspace/ns-3/ns-3.26/src/wave/examples/20170831/output.net.xml";

      if (m_loadBuildings != 0)
        {
          std::string bldgFile = "/home/wu/workspace/ns-3/ns-3.26/src/wave/examples/20170831/buildings.xml";
          NS_LOG_UNCOND ("Loading buildings file " << bldgFile);
          Topology::LoadBuildings(bldgFile);
        }

    }
  else if (m_scenario == 3)
    {
      m_traceFile = "/home/wu/workspace/ns-3/ns-3.26/src/wave/examples/20170827/20170827.ns2";

      m_mobility = 1;
      m_nNodes = 344;
      totalTime = 30;
      m_nSinks = 40;
      m_lossModel = 3; // two-ray ground
      //m_mbr = true;
      m_netFileString = "/home/wu/workspace/ns-3/ns-3.26/src/wave/examples/20170827/output.net.xml";
      if (m_loadBuildings != 0)
        {
          std::string bldgFile = "/home/wu/workspace/ns-3/ns-3.26/src/wave/examples/20170827/buildings.xml";
          NS_LOG_UNCOND ("Loading buildings file " << bldgFile);
          Topology::LoadBuildings(bldgFile);
        }

    }
  else if (m_scenario == 4)
    {
      m_traceFile = "";
      m_mobility = 3;
      m_nNodes = 26;
      totalTime = 10;
      //m_nSinks = 1;
      m_lossModel = 3; // two-ray ground
      //m_mbr = true;
      m_netFileString = "/home/wu/workspace/ns-3/ns-3.26/src/wave/examples/20170831/output.net.xml";
      if (m_loadBuildings != 0)
        {
          std::string bldgFile = "/home/wu/workspace/ns-3/ns-3.26/src/wave/examples/20170831/buildings.xml";
          NS_LOG_UNCOND ("Loading buildings file " << bldgFile);
          Topology::LoadBuildings(bldgFile);
        }

    }
}
void
GpsrExample::SetupAdhocMobilityNodes ()
{
  if (m_mobility == 1)
    {
      // Create Ns2MobilityHelper with the specified trace log file as parameter
      Ns2MobilityHelper ns2 = Ns2MobilityHelper (m_traceFile);
      ns2.Install (); // configure movements for each node, while reading trace file
    }
  else if (m_mobility == 2)
    {
      MobilityHelper mobility;
      // place two nodes at specific positions (100,0) and (0,100)
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
      positionAlloc->Add (Vector (3, 467, 0)); //0
      positionAlloc->Add (Vector (0, 586, 0));
//      positionAlloc->Add (Vector (30, 588, 0));
//      positionAlloc->Add (Vector (85, 593, 0));
//      positionAlloc->Add (Vector (154, 596, 0));
      positionAlloc->Add (Vector (249, 600, 0));

//      positionAlloc->Add (Vector (386, 453, 0));

      mobility.SetPositionAllocator(positionAlloc);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (m_nodesContainer);
    }
  else if (m_mobility == 3)
    {
      MobilityHelper mobility;
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
      positionAlloc->Add (Vector (3, 550, 0)); //0

      positionAlloc->Add (Vector (0, 586, 0));
      positionAlloc->Add (Vector (30, 588, 0));

      positionAlloc->Add (Vector (85, 593, 0));
      positionAlloc->Add (Vector (154, 596, 0));
      positionAlloc->Add (Vector (249, 600, 0));
      positionAlloc->Add (Vector (293, 608, 0));

      positionAlloc->Add (Vector (325, 612, 0));
      positionAlloc->Add (Vector (373, 612, 0));//re 8
      positionAlloc->Add (Vector (375, 576, 0));

      positionAlloc->Add (Vector (375, 544, 0));
      positionAlloc->Add (Vector (386, 453, 0));
      positionAlloc->Add (Vector (396, 398, 0));
      positionAlloc->Add (Vector (401, 333, 0)); //13
      positionAlloc->Add (Vector (408, 240, 0));
      positionAlloc->Add (Vector (415, 106, 0));

      positionAlloc->Add (Vector (416, 60, 0));
      positionAlloc->Add (Vector (417, 25, 0));

      positionAlloc->Add (Vector (455, 21, 0));
      positionAlloc->Add (Vector (540, 22, 0));
      positionAlloc->Add (Vector (610, 22, 0));
      positionAlloc->Add (Vector (675, 20, 0));  //21

      positionAlloc->Add (Vector (725, 14, 0));
      positionAlloc->Add (Vector (797, 21, 0));

      positionAlloc->Add (Vector (795, 57, 0));
      positionAlloc->Add (Vector (780, 120, 0)); //25
      mobility.SetPositionAllocator(positionAlloc);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (m_nodesContainer);
    }
}

void
GpsrExample::SetupRoutingMessages (NodeContainer & c,
                           Ipv4InterfaceContainer & adhocTxInterfaces)
{
  // Setup routing transmissions
  OnOffHelper onoff1 ("ns3::UdpSocketFactory",Address (),true, dataDevices);
  onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

  Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
  int64_t stream = 2;
  var->SetStream (stream);
  if (m_scenario != 4 && m_scenario!=2)
    {
      for (uint32_t i = 0; i < m_nSinks; i++)
	{

	  Ptr<Socket> sink = SetupRoutingPacketReceive (adhocTxInterfaces.GetAddress (i), c.Get (i));


	  AddressValue remoteAddress (InetSocketAddress (adhocTxInterfaces.GetAddress (i), m_port));
	  onoff1.SetAttribute ("Remote", remoteAddress);

	  ApplicationContainer temp = onoff1.Install (c.Get (i + m_nSinks));
	  temp.Start (Seconds (var->GetValue (1.0,2.0)));
	  temp.Stop (Seconds (totalTime));
	}
    }
  else if (m_scenario == 4 || m_scenario == 2)
    {
//      for (uint32_t i = 1; i <= m_nSinks; i++)
//	{
//	  Ptr<Socket> sink = SetupRoutingPacketReceive (adhocTxInterfaces.GetAddress (i), c.Get (i));
//	  AddressValue remoteAddress (InetSocketAddress (adhocTxInterfaces.GetAddress (i), m_port));
//	  onoff1.SetAttribute ("Remote", remoteAddress);
//	  onoff1.SetAttribute ("PacketSize", UintegerValue (m_pktSize));
//	  ApplicationContainer temp = onoff1.Install (c.Get (0));
//	  temp.Start (Seconds (var->GetValue (1.0,2.0)));
//	  temp.Stop (Seconds (totalTime));
//	}

	Ptr<Socket> sink = SetupRoutingPacketReceive (adhocTxInterfaces.GetAddress (m_nSinks), c.Get (m_nSinks));
	AddressValue remoteAddress (InetSocketAddress (adhocTxInterfaces.GetAddress (m_nSinks), m_port));
	onoff1.SetAttribute ("Remote", remoteAddress);
	onoff1.SetAttribute ("PacketSize", UintegerValue (m_pktSize));

	ApplicationContainer temp = onoff1.Install (c.Get (0));
	temp.Start (Seconds (var->GetValue (1.0,2.0)));
	temp.Stop (Seconds (totalTime));

//      UdpEchoServerHelper echoServer (9);
//    // 0 --- 2 --- 1
//      ApplicationContainer serverApps = echoServer.Install (c.Get (13));
//      serverApps.Start (Seconds (1.0));
//      serverApps.Stop (Seconds (10.0));
//
//      UdpEchoClientHelper echoClient (adhocTxInterfaces.GetAddress (13), 9); //Data interface
//      echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
//      echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
//      echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
//
//      ApplicationContainer clientApps = echoClient.Install (c.Get (0));
//      clientApps.Start (Seconds (6.0));
//      clientApps.Stop (Seconds (10.0));
    }
}
Ptr<Socket>
GpsrExample::SetupRoutingPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, m_port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&GpsrExample::ReceiveRoutingPacket, this));

  return sink;
}

void
GpsrExample::CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;
  bytesTotal = 0;

  std::ofstream out ("gpst-test-csv", std::ios::app);

  out << (Simulator::Now ()).GetSeconds () << ","
      << kbs << ","
      << packetsReceived << ","
      << m_nSinks << ","
      << m_txp << ""
      << std::endl;

  out.close ();
  packetsReceived = 0;
  Simulator::Schedule (Seconds (1.0), &GpsrExample::CheckThroughput, this);
}

void
GpsrExample::ReceiveRoutingPacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address srcAddress;
  while ((packet = socket->RecvFrom (srcAddress)))
    {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
      //NS_LOG_UNCOND (PrintReceivedPacket (socket, packet, senderAddress));
    }
}
