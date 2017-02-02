
#include "neighbors.h"

/**
 * shared memory address
 */
extern unsigned char *shared_mem_neighbor;

neighbor_table	**neigh_data;
/**
 * neigh_status:
 * 	0: not busy
 * 	1: busy
 */
u8 		*neigh_status;
u32		*neigh_count;


int neigh_list_init()
{
	neigh_status = shared_mem_neighbor + NEIGH_STATUS_OFFSET;
	neigh_count = shared_mem_neighbor + NEIGH_COUNT_OFFSET;
	neigh_data = shared_mem_neighbor + NEIGH_DATA_OFFSET;
}

inline void wait_neigh_available()
{
	while(*neigh_status){}
}
/**
 * Search node's geohash from it's ip addr.
 * return
 *
 */
u64 neighbor_getgeohash_fromip(u32 ip)
{
	int i;
	wait_neigh_available();
	for(i = 0; i < *neigh_count; i++) {
		if(neigh_data[i]->ip == ip)
			return neigh_data[i]->geoHash;
	}
	return 0;
}

/**
 * get a random node which is in the range of the geohash set.
 * return 0 if unmatched.
 */
u64 neighbor_get_node_fromset_random(neighbor_table* neighbor_entry, u64 *geohashset, int size)
{
	int i,j;
	wait_neigh_available();
	for(i = 0; i < *neigh_count; i++) {
		for(j = 0; j < size; j++) {
			if(neigh_data[i]->geoHash == geohashset[j]) {
				neighbor_entry = neigh_data[i];
				return 1;
			}
		}
	}
	return 0;
}

