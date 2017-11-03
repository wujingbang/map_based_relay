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
  if (m_path == NULL)
    return 4;
  else
    return 4 + m_path[0] * 4;
}
void
geoSVRTag::Serialize (TagBuffer i) const
{
  if (m_path == NULL)
    {
      i.WriteU32(0);
      return;
    }
  uint32_t temp = (uint32_t)(m_path[0]);
  i.WriteU32 (temp);
  if (m_path[0] != 0)
    {
      for (uint32_t j=1; j <= m_path[0]; j++)
	{
	  temp = (uint32_t)(m_path[j]);
	  i.WriteU32 (temp);
	}
    }
  free (m_path);//malloc in the encode_path function.
}
void
geoSVRTag::Deserialize (TagBuffer i)
{
  uint32_t len = i.ReadU32 ();
  m_path_vector.clear();
  m_path_vector.push_back(len);
  if(len != 0)
  {
      for (uint32_t j=1; j<= len; j++)
	{
	  uint32_t temp = i.ReadU32 ();
	  m_path_vector.push_back(temp);
	}

  }

}

void
geoSVRTag::Print (std::ostream &os) const
{
  //os << "m_path = " << m_path;
}

}
}


