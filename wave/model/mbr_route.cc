#include "mbr_route.h"
#include "mbr.h"
#include "mbr-common.h"
#include "mbr-utils.h"

#include "mbr_sumomap.h"
#include "ns3/mbr-neighbor-app.h"
#include <string.h>

using namespace ns3;
using namespace mbr;

int bitcmp(uint64_t a, uint64_t b, int step)
{
	return a==b;
}


uint64_t get_geohash_this(void)
{
	if (global_mbr_status.geohash_this == 0) {
		mbr_dbg(debug_level, ANY,"get_geohash_this get zero!!\n");
	}

	return global_mbr_status.geohash_this;
}

////Eager Singleton
//MbrRoute* MbrRoute::p = new MbrRoute;
//
//MbrRoute * MbrRoute::GetInstance(void)
//{
//	return p;
//}

int MbrRoute::mbr_forward(uint8_t * to, uint8_t * relay_mac, Ptr<Node> thisnode)
{
	/**
	 * �����ھӱ�õ��õ��Geohash
	 */
	//int i;

	uint64_t nexthop_geohash = 0;
	uint64_t this_geohash = 0;
	uint64_t dst_geohash = 0;
	Vertex *intersection;
	Vertex *this_vertex;
	Vertex *dst_vertex;
	//GeoHashBits	geohashbit_tmp;
	GeoHashSetCoordinate geohashset;
//	int ret;

	MbrSumo* sumomap = MbrSumo::GetInstance();
	NS_ASSERT (sumomap->isInitialized());
	Graph* g = sumomap->getGraph();
	//Assume mbr-neighbor-app is the first application.
	Ptr<MbrNeighborApp> mbapp = DynamicCast<MbrNeighborApp> (thisnode->GetApplication(0));
//	dst_geohash = neighbor_getgeohash_frommac(to);
	dst_geohash = mbapp->getNb()->GetgeohashFromMac(to);
	if(dst_geohash == 0) {
		mbr_dbg(debug_level, ANY, "mbr_forward: nexthop does not exist in the neighbors!\n");
		return -1;
	}

	this_geohash = get_geohash_this();
	this_vertex = find_Vertex_by_VehiclePosition(g, this_geohash);
	dst_vertex = find_Vertex_by_VehiclePosition(g, dst_geohash);
	intersection = cross_vertex(this_vertex, dst_vertex);
	if(intersection != NULL)
		nexthop_geohash = intersection->geoHash;
	else
		return -1;

	/**
	 * ��Geohash����·��ת��:
	 * 1. ͨ��Geohash����ܱ��ھӿ鹲ͬ���·��
	 * 2. ��ȡ�鼯���еĽڵ���Ϣ
	 * 3. �������һ���ڵ���Ϊ�м̽ڵ�
	 */
	setIntersectionSize(&geohashset, this_vertex, dst_vertex);

	//get neighbors of center geohash block
	geohash_get_neighbors_in_set(&geohashset, nexthop_geohash, GEOHASH_STEP_BIT);

	//ret = neighbor_getnode_fromset_random(&neighbor_entry, &geohashset);

//	ret = neighbor_getnode_fromset_best(&neighbor_entry, &geohashset);
//	if(ret == 0)
//		return -1; //unmatched!
//
//	memcpy(relay_mac, neighbor_entry->mac, 6);

	return 0;
}

