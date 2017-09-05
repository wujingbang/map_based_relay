#include "mbr_route.h"
#include "mbr.h"
#include "mbr-common.h"
#include "mbr-utils.h"

#include "mbr_sumomap.h"
#include "ns3/mbr-neighbor-app.h"
#include <string.h>
#include "ns3/core-module.h"

using namespace ns3;
using namespace mbr;

NS_LOG_COMPONENT_DEFINE ("MbrRoute");

MbrRoute::MbrRoute()
{
	NS_LOG_FUNCTION (this);
}
int bitcmp(uint64_t a, uint64_t b, int step)
{
	return a==b;
}

void vertexlist_free(vertexlist *head)
{
	vertexlist *temp;
	while(head!=NULL)
	{
		temp = head;
		head = head->next;
		mbr_free(temp);
	}
}

////Eager Singleton
//MbrRoute* MbrRoute::p = new MbrRoute;
//
//MbrRoute * MbrRoute::GetInstance(void)
//{
//	return p;
//}

int MbrRoute::mbr_forward(Ipv4Address dest, uint8_t * to_mac, uint8_t * relay_mac, Ptr<Node> thisnode)
{
	/**
	 * �����ھӱ�õ��õ��Geohash
	 */
	//int i;

	uint64_t nexthop_geohash = 0;
	uint64_t this_geohash = 0;
	uint64_t dst_geohash = 0;
	vertexlist *intersectionlist;
	Vertex *this_vertex;
	Vertex *dst_vertex;
	//GeoHashBits	geohashbit_tmp;
	GeoHashSetCoordinate geohashset;
	int ret;
	double x_this,y_this;
	double x_dst,y_dst;
	MbrSumo* sumomap = MbrSumo::GetInstance();

	if (!sumomap->isInitialized())
		return -1;

	Graph* g = sumomap->getGraph();
	Ptr<MbrNeighborApp> nbapp;
	for (uint32_t j = 0; j < thisnode->GetNApplications (); j++)
	  {
	    nbapp = DynamicCast<MbrNeighborApp> (thisnode->GetApplication(j));
	    if (nbapp)
	      break;
	  }
	if (!nbapp) return -1;
//	dst_geohash = neighbor_getgeohash_frommac(to);
	dst_geohash = nbapp->getNb()->GetGeohashFromIpInNb(dest, to_mac, &x_dst, &y_dst);
	if(dst_geohash == 0) {
		mbr_dbg(debug_level, ANY, "mbr_forward: nexthop does not exist in the neighbors!\n");
		NS_LOG_LOGIC("mbr_forward: nexthop does not exist in the neighbors!\n");
		return -1;
	}

	this_geohash = sumomap->GetNodeCurrentGeohash(thisnode);
	sumomap->GetNodeCurrentXY(thisnode, &x_this, &y_this);
	this_vertex = MbrGraph::find_Vertex_by_VehiclePosition(g, this_geohash, x_this, y_this);
	dst_vertex = MbrGraph::find_Vertex_by_VehiclePosition(g, dst_geohash, x_dst, y_dst);
	intersectionlist = MbrGraph::cross_vertex(dst_vertex,this_vertex);//Be careful of the vertex order !

	/**
	* MBR: Check whether this node and "to" are in the same road,
	* if not, MBR should be activated.
	*/
	if(intersectionlist == NULL)
	  {
	    NS_LOG_LOGIC("intersectionlist is NULL ! this_v: "<<this_vertex->idStr << " dst_v: "<<dst_vertex->idStr);
	    return -1;
	  }

	/**
	 * ��Geohash����·��ת��:
	 * 1. ͨ��Geohash����ܱ��ھӿ鹲ͬ���·��
	 * 2. ��ȡ�鼯���еĽڵ���Ϣ
	 * 3. �������һ���ڵ���Ϊ�м̽ڵ�
	 */
	vertexlist *temp = intersectionlist;

	while(temp != NULL)
	{
		nexthop_geohash = temp->v->geoHash;

		MbrGraph::setIntersectionSize(&geohashset, this_vertex, dst_vertex);

	//get neighbors of center geohash block
		geohash_get_neighbors_in_set(&geohashset, nexthop_geohash, GEOHASH_STEP_BIT);

	//ret = neighbor_getnode_fromset_random(&neighbor_entry, &geohashset);
		Mac48Address relaymac;

		ret = nbapp->getNb()->GetnbFromsetBest(&relaymac, &geohashset);
		if(ret == 0)
		{
			temp = temp->next;
			continue;
		} //unmatched!

		NS_LOG_LOGIC ("node="<< thisnode->GetId() <<
									", This Area " << this_vertex->idStr <<
									", Dest Area " << dst_vertex->idStr <<
									", Relay Area " << temp->v->idStr <<
											", Dest IP "<< dest);
		relaymac.CopyTo(relay_mac);
		vertexlist_free(intersectionlist);
		return 0;
	}
	vertexlist_free(intersectionlist);
	return -1;
}

