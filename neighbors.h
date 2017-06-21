
#ifndef NEIGHBOR_H_
#define NEIGHBOR_H_

#include "common.h"
#include "geohash.h"
#include "debug.h"

#ifdef LINUX_KERNEL
#include <linux/list.h>
#include <linux/types.h>

#else
#include "linux_list.h"
#include <string.h>
//#include "wave-net-device.h"

#endif

/**
 * Use shared memory to store the neighbor list. Because the memory addr should be continuous,
 * the accessing of the table could use index form.
 *
 * From start of shared mem, 1byte to determine if the neighbor list is accessible (unlocked),
 * 4byte (sizeof(int)) to store the number of the neighbors.
 *
 */
#define NEIGH_STATUS_OFFSET 0
#define NEIGH_COUNT_OFFSET	1
#define NEIGH_DATA_OFFSET	5


#ifdef __cplusplus
extern "C"
{
#endif

struct neighbor_table_ {
	uint32_t ip;
	uint8_t mac[6];

	uint64_t geoHash;

	//GPS direction: 0~359, starting from North.
	int direction;
	//unsigned char idStr_section[25];//this node's closest road section
	int		isvalid;
};
typedef struct neighbor_table_ neighbor_table;

extern int neighbor_getnode_fromip(neighbor_table** neighbor_entry, uint32_t ip);
extern uint64_t neighbor_getgeohash_fromip(uint32_t ip);
extern uint64_t neighbor_getnode_fromset_random(neighbor_table** neighbor_entry, GeoHashSetCoordinate *geohashset);
extern uint64_t neighbor_getnode_fromset_best(neighbor_table** neighbor_entry, GeoHashSetCoordinate *geohashset);
extern int neigh_list_init(void);
extern void wait_neigh_available(void);

extern uint64_t neighbor_getgeohash_frommac(uint8_t* mac);

#ifdef __cplusplus
}
#endif
#endif
