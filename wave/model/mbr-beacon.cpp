#include "ns3/mbr-beacon.h"


namespace ns3 {

MbrBeacon::MbrBeacon (uint64_t us)
{
  m_header.SetBeaconIntervalUs (us);
}
void
MbrBeacon::AddInformationElement (Ptr<WifiInformationElement> ie)
{
  m_elements.AddInformationElement (ie);
}

Time
MbrBeacon::GetBeaconInterval () const
{
  return MicroSeconds (m_header.GetBeaconIntervalUs ());
}

Ptr<Packet>
MbrBeacon::CreatePacket ()
{
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (m_elements);
  packet->AddHeader (BeaconHeader ());
  return packet;
}
WifiMacHeader
MbrBeacon::CreateHeader (Mac48Address address)
{
  WifiMacHeader hdr;

  //hdr.SetBeacon ();
  hdr.SetMbrBeacon();
  hdr.SetAddr1 (Mac48Address::GetBroadcast ());
  hdr.SetAddr2 (address);
  hdr.SetAddr3 (Mac48Address::GetBroadcast ());
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();

  return hdr;
}
} // namespace ns3
