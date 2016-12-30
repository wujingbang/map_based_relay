
#include <linux/type.h>

#include "graph.h"
#include "geohash.h"

struct relay_table_list_ {
	uint8_t geoHash_this;
	uint8_t geoHash_nexthop;
	uint8_t geoHash_dst;
	int		isvalid;

	struct list_head	ptr;
};

typedef struct relay_table_list_ relay_table_list;
