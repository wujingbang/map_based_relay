#include "geoSVR-packet-tag.h"

namespace ns3 {
namespace geoSVR {

TypeId
geoSVRTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::geoSVRTag")
    .SetParent<Tag> ()
    .AddConstructor<geoSVRTag> ()
  ;
  return tid;
}

TypeId
geoSVRTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
geoSVRTag::GetSerializedSize (void) const
{
  return 4;
}
void
geoSVRTag::Serialize (TagBuffer i) const
{
  uint32_t path;
  memcpy(&path, &m_path, 4);
  i.WriteU32 (path);
}
void
geoSVRTag::Deserialize (TagBuffer i)
{
	uint32_t path;
  path = i.ReadU32 ();
	memcpy(&m_path, &path, 4);
}
void
geoSVRTag::Print (std::ostream &os) const
{
  //os << "m_path = " << m_path;
}

}
}


