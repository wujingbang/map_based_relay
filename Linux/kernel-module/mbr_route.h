#ifndef MBR_ROUTE_H
#define MBR_ROUTE_H

//#include "mbr.h"
#include "graph.h"

#include <linux/list.h>
#include <linux/types.h>

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
void print_mbrtable(struct seq_file *file);

#endif
