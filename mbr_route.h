
#include <linux/type.h>
#include <linux/list.h>

#include "graph.h"
#include "geohash.h"

struct relay_table_list_ {
	uint8_t geoHash_this[3];
	uint8_t geoHash_nexthop[3];
	uint8_t geoHash_dst[3];
	int		isvalid;

	struct list_head	ptr;
};

typedef struct relay_table_list_ relay_table_list;
