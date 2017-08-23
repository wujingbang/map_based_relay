#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

namespace ns3 {
namespace mbr {
class MbrTag : public Tag
{
public:
  MbrTag(){
	  m_relayflag = 0;
  }
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

	uint8_t* getRelayMac() {
		return m_relay_mac;
	}

	uint8_t isRelaying() const {
		return m_relayflag;
	}

	void setRelayflag(bool relayflag) {
		m_relayflag = relayflag;
	}

private:
  uint8_t m_relayflag;
  uint8_t m_relay_mac[6];
};

}
}
