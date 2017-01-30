#include "mbr_route.h"

LIST_HEAD(mbrtable)

int geohash_compare()
{

}


/**
 * Search routing table for the GeoHash of next-hop.
 * input: dest node's geohash
 * return: valid next-hop's geohash if exist.
 */
u64 mbrtable_get_nexthop_geohash(u64 dstGeoHash)
{
	relay_table_list *entry;
	list_for_each_entry(entry, &mbrtable, ptr){
		if(!bitcmp(entry->geoHash_dst, dstGeoHash, GEOHASH_STEP_BIT) && entry->isvalid)
			return entry->geoHash_nexthop;
	}
	return 0;
}

int update_mbrtable(u64 this_geohash, u64 dst_geohash, u64 nexthop_geohash)
{

}

int mbr_forward(struct sk_buff *skb, Graph *g)
{
	/**
	 * 从skb中nexthop查找邻居表得到该点的Geohash
	 */
	struct dst_entry *dst = skb_dst(skb);
	struct rtable *rt = (struct rtable *)dst;
	u32 nexthop; //存储skb中包含的路由下一跳的ip
	u64 nexthop_geohash = 0;
	u64 this_geohash = 0;
	u64 dst_geohash = 0;
	Vertex *intersection;
	Vertex *this_vertex;
	Vertex *nexthop_vertex;
	GeoHashNeighbors geohashset;
	GeoHashBits	geohashbit_tmp;

	nexthop = (__force u32) rt_nexthop(rt, ip_hdr(skb)->daddr);

	nexthop_geohash = neighbor_getgeohash_fromip(nexthop);
	if(nexthop_geohash == 0){ //miss
		/**
		 * 计算中继区域，并更新中继表
		 */
		this_geohash = get_geohash_this();
		this_vertex = find_Vertex_by_VehiclePosition(g, this_geohash);
		nexthop_vertex = find_Vertex_by_VehiclePosition(g, nexthop_geohash);
		intersection = cross_vertex(this_vertex, nexthop_vertex);
		nexthop_geohash = intersection.geoHash;
		dst_geohash = neighbor_getgeohash_fromip(ip_hdr(skb)->daddr);
		update_mbrtable(this_geohash, dst_geohash, nexthop_geohash);
	}
	/**
	 * 按Geohash进行路由转发:
	 * 1. 通过Geohash获得周边邻居块共同组成路口
	 * 2. 获取块集合中的节点信息
	 * 3. 随机挑出一个节点作为中继节点
	 */
	geohashbit_tmp.step = GEOHASH_STEP_BIT;
	geohashbit_tmp.bits = nexthop_geohash;
	geohash_get_neighbors(geohashbit_tmp, geohashset);

}

int
