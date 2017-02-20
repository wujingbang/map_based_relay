#include "mbr_route.h"
#include <net/route.h>

LIST_HEAD(mbrtable);

extern struct mbr_status global_mbr_status;
extern int debug_level;

int bitcmp(u64 a, u64 b, int step)
{
	return a==b;
}
/**
 * Search mbr table for the GeoHash of next-hop.
 * input: dest node's geohash
 * return: valid next-hop's geohash if exist.
 */
u64 mbrtable_get_nexthop_geohash(u64 dstGeoHash)
{
	relay_table_list *entry;
	list_for_each_entry(entry, &mbrtable, ptr){
		if(bitcmp(entry->geoHash_dst, dstGeoHash, GEOHASH_STEP_BIT) && entry->isvalid)
			return entry->geoHash_nexthop;
	}
	return 0;
}

u64 get_geohash_this(void)
{
	if (global_mbr_status.geohash_this == 0) {
		mbr_dbg(debug_level, ANY,"get_geohash_this get zero!!\n");
	}

	return global_mbr_status.geohash_this;
}

/**
 * 当本节点的geohash发生变化时，要对mbr中继表进行更新
 */
int update_mbrtable_outrange(u64 updated_geohash)
{
	relay_table_list *entry;
	list_for_each_entry(entry, &mbrtable, ptr){
		if(bitcmp(entry->geoHash_this, updated_geohash, GEOHASH_STEP_BIT)) {

			entry->isvalid = 0;

		}
	}
	return 0;
}

/**
 * 首先要搜索到dst_geohash的记录，有的话就进行更新，没有的话就要新增一条记录
 */
int update_mbrtable(u64 this_geohash, u64 dst_geohash, u64 nexthop_geohash)
{
	relay_table_list *entry;

	list_for_each_entry(entry, &mbrtable, ptr){
		if(bitcmp(entry->geoHash_dst, dst_geohash, GEOHASH_STEP_BIT)) {
			entry->geoHash_nexthop = nexthop_geohash;
			entry->isvalid = 1;
			goto out;
		}
	}
	entry = (relay_table_list *)kmalloc(sizeof(relay_table_list), GFP_KERNEL);
	entry->geoHash_dst = dst_geohash;
	entry->geoHash_nexthop = nexthop_geohash;
	entry->geoHash_this = this_geohash;
	entry->isvalid = 1;
	list_add(entry, &mbrtable);
out:
	return 0;
}

int mbr_forward(u8 *dst_mac, u8 *relay_mac, struct sk_buff *skb, Graph *g)
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
	GeoHashBits	geohashbit_tmp;
	neighbor_table* neighbor_entry;
	u64 geohashset[9];
	int ret;

	/**
	 * 从邻居表中找到下一跳ip（注意此时的“下一跳ip”的是网络路由指定的下一跳ip，还不是中继节点的ip）对应的geohash
	 */
	nexthop = (__force u32) rt_nexthop(rt, ip_hdr(skb)->daddr);
	ret = neighbor_getnode_fromip(neighbor_entry, nexthop);
	if(ret == 0) {
		nexthop_geohash = neighbor_entry->geoHash;
		dst_mac = neighbor_entry->mac;
	} else {
		mbr_dbg(debug_level, ANY, "mbr_forward: nexthop does not exist in the neighbors!\n");
		return -1;
	}
	/**
	 * 找中继表得到中继geohash，得到的结果已经是“中继节点”的geohash
	 */
	nexthop_geohash = mbrtable_get_nexthop_geohash(nexthop_geohash);

	if(nexthop_geohash == 0){ //miss
		/**
		 * 计算中继区域，并更新中继表
		 */
		this_geohash = get_geohash_this();
		this_vertex = find_Vertex_by_VehiclePosition(g, this_geohash);
		nexthop_vertex = find_Vertex_by_VehiclePosition(g, nexthop_geohash);
		intersection = cross_vertex(this_vertex, nexthop_vertex);
		nexthop_geohash = intersection->geoHash;
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
	geohash_get_neighbors_in_set(geohashbit_tmp, geohashset);
	ret = neighbor_getnode_fromset_random(neighbor_entry, geohashset, 9);
	if(ret == 0)
		return -1; //unmatched!
	relay_mac = neighbor_entry->mac;

	return 0;
}

