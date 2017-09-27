#ifndef COMMON_H_
#define COMMON_H_

#ifdef LINUX_KERNEL
#include <linux/types.h>
#define UINT64_MAX  0xffffffffffffffff

#else
#include <stdint.h>
#endif

typedef struct {
	uint64_t ** geohashset;
	int sx;
	int sy;
} GeoHashSetCoordinate;

/**
 * coding step of longitude and latitude each (bit).
 * a geohash consists of longitude coding plus latitude coding.
 * so the step of geohash is GEOHASH_STEP_BIT * 2.
 */
//#define GEOHASH_STEP_BIT	12
#define GEOHASH_STEP_BIT	13

extern struct mbr_status global_mbr_status;
extern int debug_level;


#endif
