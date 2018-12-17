/*
 * analyze-tag.cc
 *
 *  Created on: Dec 17, 2018
 *      Author: wu
 */

#include "analyze-tag.h"


using namespace ns3;


TypeId
AnalyzeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AnalyzeTag")
    .SetParent<Tag> ()
    .AddConstructor<AnalyzeTag> ()
  ;
  return tid;
}

TypeId
AnalyzeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
AnalyzeTag::GetSerializedSize (void) const
{
  return 8;
}
void
AnalyzeTag::Serialize (TagBuffer i) const
{
  i.WriteU64 (m_timemark_ms);
}
void
AnalyzeTag::Deserialize (TagBuffer i)
{
	m_timemark_ms = i.ReadU64 ();
}
void
AnalyzeTag::Print (std::ostream &os) const
{
  os << "v=" << m_timemark_ms;
}



