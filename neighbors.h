
#include <linux/type.h>
#include <linux/list.h>

#include "mbr.h"

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

struct neighbor_table_ {
	u32 ip;
	u8 mac[6];
	double longitude;
	double latitude;
	u64 geoHash;
	//unsigned char idStr_section[25];//this node's closest road section
	int		isvalid;
};
typedef struct neighbor_table_ neighbor_table;

u64 neighbor_getgeohash_fromip(u32 ip)
