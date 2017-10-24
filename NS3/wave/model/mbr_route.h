#ifndef MBR_ROUTE_H
#define MBR_ROUTE_H

//#include "mbr.h"
#include "graph.h"

#include "ns3/ptr.h"
#include "ns3/node.h"

namespace ns3{
namespace mbr{
class MbrRoute{

public:
	MbrRoute();
	static int mbr_forward(Ipv4Address dest, uint8_t* tomac, uint8_t * relay_mac, Ptr<Node> thisnode, uint32_t pktsize);

      static uint32_t
      getRelayedPktNum ()
      {
	return m_relayedPktNum;
      }

      static void
      setRelayedPktNum (uint32_t relayedPktNum)
      {
	m_relayedPktNum = relayedPktNum;
      }

      static uint32_t
      getNoNeighborPktNum ()
      {
	return m_noNeighborPktNum;
      }

      static void
      setNoNeighborPktNum (uint32_t noNeighborPktNum)
      {
	m_noNeighborPktNum = noNeighborPktNum;
      }

      static uint32_t
      getMaxd ()
      {
	return m_maxd;
      }

      static void
      setMaxd (uint32_t maxd)
      {
	m_maxd = maxd;
      }

      static uint32_t
      getMind ()
      {
	return m_mind;
      }

      static void
      setMind (uint32_t mind)
      {
	m_mind = mind;
      }

      //	/**
//	* \brief Gets the topology instance
//	* \return the topology instance
//	*/
//	static MbrRoute * GetInstance();
private:
//	static MbrRoute * p;
//	//private constructor
//	MbrRoute(){};
	static uint32_t m_relayedPktNum;
	static uint32_t m_noNeighborPktNum;
	static uint32_t m_mind;
	static uint32_t m_maxd;
};
}
}
#endif
