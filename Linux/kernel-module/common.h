#ifndef COMMON_H_
#define COMMON_H_

#include <linux/types.h>

typedef struct {
	u64 ** geohashset;
	int sx;
	int sy;
} GeoHashSetCoordinate;

/**
 * coding step of longitude and latitude each (bit).
 * a geohash consists of longitude coding plus latitude coding.
 * so the step of geohash is GEOHASH_STEP_BIT * 2.
 */
#define GEOHASH_STEP_BIT	12


extern struct mbr_status global_mbr_status;
extern int debug_level;


#endif
