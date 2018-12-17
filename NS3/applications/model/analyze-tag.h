/*
 * analyze-tag.h
 *
 *  Created on: Dec 17, 2018
 *      Author: wu
 */

#ifndef SRC_APPLICATIONS_MODEL_ANALYZE_TAG_H_
#define SRC_APPLICATIONS_MODEL_ANALYZE_TAG_H_


#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

namespace ns3 {

class AnalyzeTag : public Tag
{
public:
	AnalyzeTag(){
		m_timemark_ms = 0;
  }
	AnalyzeTag(uint64_t timemark_ms){
		m_timemark_ms = timemark_ms;
  }
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

	uint64_t getTimemarkMs() const {
		return m_timemark_ms;
	}

	void setTimemarkMs(uint64_t timemarkMs) {
		m_timemark_ms = timemarkMs;
	}

private:
  uint64_t m_timemark_ms;
};


}



#endif /* SRC_APPLICATIONS_MODEL_ANALYZE_TAG_H_ */
