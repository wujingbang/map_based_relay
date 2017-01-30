
#include <linux/type.h>
#include <linux/list.h>

#include "graph.h"
#include "geohash.h"

#define GEOHASH_STEP_BIT	24

struct relay_table_list_ {
	u64 geoHash_this;
	u64 geoHash_nexthop;
	u64 geoHash_dst;
	int	isvalid;

	struct list_head	ptr;
};

typedef struct relay_table_list_ relay_table_list;
