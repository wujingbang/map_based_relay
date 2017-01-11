
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
uint8_t 		*neigh_status;
u_int32_t		*neigh_count;


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
 * 	0: Search succeed. Result stored in the resGeoHash.
 * 	-1: Search failed.
 */
uint8_t * search_neigh_for_geohash(u_int32_t ip)
{
	int i;
	wait_neigh_available();
	for(i = 0; i < *neigh_count; i++) {
		if(neigh_data[i]->ip == ip)
			return neigh_data[i]->geoHash;
	}
	return NULL;
}

