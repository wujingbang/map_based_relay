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

////Eager Singleton
//MbrRoute* MbrRoute::p = new MbrRoute;
//
//MbrRoute * MbrRoute::GetInstance(void)
//{
//	return p;
//}

void vertexlist_free(vertexlist *head)
{
	vertexlist *temp;
	while(head!=NULL)
	{
		temp = head;
		head = head->next;
		kfree(temp);
	}
}

int MbrRoute::mbr_forward(uint8_t * to, uint8_t * relay_mac, Ptr<Node> thisnode)
{
	/**
	 * �����ھӱ�õ��õ��Geohash
	 */
	//int i;

	uint64_t nexthop_geohash = 0;
	uint64_t this_geohash = 0;
	uint64_t dst_geohash = 0;
	vertexlist *intersection;
	Vertex *this_vertex;
	Vertex *dst_vertex;
	//GeoHashBits	geohashbit_tmp;
	GeoHashSetCoordinate geohashset;
	int ret;
	double x_this,y_this;
	double x_dst,y_dst;
	MbrSumo* sumomap = MbrSumo::GetInstance();
	NS_ASSERT (sumomap->isInitialized());
	Graph* g = sumomap->getGraph();
	//Assume mbr-neighbor-app is the first application.
	Ptr<Application> app = thisnode->GetApplication(0);
	Ptr<MbrNeighborApp> nbapp = DynamicCast<MbrNeighborApp> (app);
//	dst_geohash = neighbor_getgeohash_frommac(to);
	dst_geohash = nbapp->getNb()->GetGeohashFromMacInNb(to, &x_dst, &y_dst);
	if(dst_geohash == 0) {
		mbr_dbg(debug_level, ANY, "mbr_forward: nexthop does not exist in the neighbors!\n");
		return -1;
	}

	this_geohash = sumomap->GetNodeCurrentGeohash(thisnode);
	sumomap->GetNodeCurrentXY(thisnode, &x_this, &y_this);
	this_vertex = MbrGraph::find_Vertex_by_VehiclePosition(g, this_geohash, x_this, y_this);
	dst_vertex = MbrGraph::find_Vertex_by_VehiclePosition(g, dst_geohash, x_dst, y_dst);
	//intersection = MbrGraph::cross_vertex(this_vertex, dst_vertex);
	intersection = MbrGraph::cross_vertex(dst_vertex,this_vertex);
	/**
	* MBR: Check whether this node and "to" are in the same road,
	* if not, MBR should be activated.
	*/
	if(intersection == NULL)
		return -1;

	/**
	 * ��Geohash����·��ת��:
	 * 1. ͨ��Geohash����ܱ��ھӿ鹲ͬ���·��
	 * 2. ��ȡ�鼯���еĽڵ���Ϣ
	 * 3. �������һ���ڵ���Ϊ�м̽ڵ�
	 */
	vertexlist *temp = intersection;

	while(temp != NULL)
	{	
		nexthop_geohash = temp->v->geoHash;

		MbrGraph::setIntersectionSize(&geohashset, this_vertex, dst_vertex);

	//get neighbors of center geohash block
		geohash_get_neighbors_in_set(&geohashset, nexthop_geohash, GEOHASH_STEP_BIT);

	//ret = neighbor_getnode_fromset_random(&neighbor_entry, &geohashset);
		Mac48Address relaymac;

		ret = nbapp->getNb()->GetnbFromsetBest(&relaymac, &geohashset);
//	ret = neighbor_getnode_fromset_best(&neighbor_entry, &geohashset);
		if(ret == 0)
		{
			temp = temp->next;
			continue;
		} //unmatched!		
		relaymac.CopyTo(relay_mac);
		vertexlist_free(intersection);
		return 0;
	}
	vertexlist_free(intersection);
	return -1;
}

