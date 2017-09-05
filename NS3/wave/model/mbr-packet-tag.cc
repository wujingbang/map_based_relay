#include "mbr-packet-tag.h"

using namespace ns3;
using namespace mbr;

TypeId
MbrTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MbrTag")
    .SetParent<Tag> ()
    .AddConstructor<MbrTag> ()
//    .AddAttribute ("SimpleValue",
//                   "A simple value",
//                   EmptyAttributeValue (),
//                   MakeUintegerAccessor (&MyTag::GetSimpleValue),
//                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}
//uint8_t m_relayflag;
//uint8_t m_relay_mac[6];
TypeId
MbrTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
MbrTag::GetSerializedSize (void) const
{
  return 7;
}
void
MbrTag::Serialize (TagBuffer i) const
{
  int j;
  i.WriteU8 (m_relayflag);
  for (j=0; j<6; j++)
	  i.WriteU8(m_relay_mac[j]);
}
void
MbrTag::Deserialize (TagBuffer i)
{
	int j;
	m_relayflag = i.ReadU8 ();
	for (j=0; j<6; j++)
		m_relay_mac[j] = i.ReadU8 ();
}
void
MbrTag::Print (std::ostream &os) const
{
  os << "v=" << (uint32_t)m_relayflag;
}


