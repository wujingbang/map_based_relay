
#include <linux/type.h>
#include <linux/list.h>

#include "mbr.h"

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
