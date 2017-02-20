#ifndef MBR_ROUTE_H
#define MBR_ROUTE_H

//#include "graph.h"
#include "mbr.h"
#include <linux/list.h>

/**
 * coding step of longitude and latitude each (bit).
 * a geohash consists of longitude coding plus latitude coding.
 * so the step of geohash is GEOHASH_STEP_BIT * 2.
 */
#define GEOHASH_STEP_BIT	12

struct relay_table_list_ {
	u64 geoHash_this;
	u64 geoHash_nexthop;
	u64 geoHash_dst;
	int	isvalid;

	struct list_head	ptr;
};

typedef struct relay_table_list_ relay_table_list;


int update_mbrtable_outrange(u64 updated_geohash);
int mbr_forward(u8 *dst_mac, u8 *relay_mac, struct sk_buff *skb, Graph *g);

#endif
