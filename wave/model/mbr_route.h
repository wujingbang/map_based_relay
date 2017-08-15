#ifndef MBR_ROUTE_H
#define MBR_ROUTE_H

//#include "mbr.h"
#include "graph.h"

#include "ns3/ptr.h"
#include "ns3/node.h"

namespace ns3{
namespace mbr{
class MbrRoute {

public:
	static int mbr_forward(uint8_t * to, uint8_t * relay_mac, Ptr<Node> thisnode);

//	/**
//	* \brief Gets the topology instance
//	* \return the topology instance
//	*/
//	static MbrRoute * GetInstance();
//private:
//	static MbrRoute * p;
//	//private constructor
//	MbrRoute(){};
};
}
}
#endif
