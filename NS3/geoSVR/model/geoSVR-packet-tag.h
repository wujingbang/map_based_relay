#ifndef GEOSVRPACKETTAG_H
#define GEOSVRPACKETTAG_H

#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

namespace ns3 {
namespace geoSVR {

class geoSVRTag : public Tag
{
public:
  geoSVRTag(){
	  m_path = NULL;
  }
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

	void GetPath(uint8_t** path) {
	  *path = (uint8_t*)malloc(m_path_vector.size() + 1);
	  for (uint32_t i=0; i< m_path_vector.size(); i++)
	    (*path)[i] = (uint8_t)(m_path_vector[i]);
	}

	void SetPath(uint8_t* path) {
		m_path = path;
	}

private:
uint8_t *m_path;
  std::vector<int> m_path_vector;
};

}
}
#endif
