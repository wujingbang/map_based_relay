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

	char* GetPath() {
		return m_path;
	}

	void SetPath(char* path) {
		m_path = path;
	}

private:
    char *m_path;
};

}
}
#endif
