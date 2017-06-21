#ifndef MBR_ROUTE_H
#define MBR_ROUTE_H

//#include "mbr.h"
#include "graph.h"

#ifdef LINUX_KERNEL
#include <linux/list.h>
#include <linux/types.h>
#include <net/route.h>

#else
#include "linux_list.h"
#endif

struct relay_table_list_ {
	uint64_t geoHash_this;
	uint64_t geoHash_nexthop;
	uint64_t geoHash_dst;
	int	isvalid;

	struct list_head	ptr;
};

typedef struct relay_table_list_ relay_table_list;


int update_mbrtable_outrange(uint64_t updated_geohash);

#ifdef LINUX_KERNEL
void print_mbrtable(struct seq_file *file);
int mbr_forward(uint8_t *dst_mac, uint8_t *relay_mac, struct sk_buff *skb, Graph *g);
#else
int mbr_forward(uint8_t * to, uint8_t * relay_mac, Graph *g);
#endif


#endif
