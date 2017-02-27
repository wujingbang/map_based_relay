
#include "neighbors.h"

/**
 * shared memory address
 */
extern unsigned char *shared_mem_neighbor;

neighbor_table	*neigh_data;
/**
 * neigh_status:
 * 	0: not busy
 * 	1: busy
 */
u8 		*neigh_status;
u32		*neigh_count;


int neigh_list_init(void)
{
	neigh_status = (u8*)(shared_mem_neighbor + NEIGH_STATUS_OFFSET);
	neigh_count = (u32*)(shared_mem_neighbor + NEIGH_COUNT_OFFSET);
	neigh_data = (neighbor_table*)(shared_mem_neighbor + NEIGH_DATA_OFFSET);
}

inline void wait_neigh_available(void)
{
	while(*neigh_status){}
}
/**
 * Search node's geohash from it's ip addr.
 * return
 *
 */
int neighbor_getnode_fromip(neighbor_table** neighbor_entry, u32 ip)
{
	int i;
	neighbor_table	*temp = neigh_data;
	wait_neigh_available();
	for(i = 0; i < *neigh_count; i++) {
		temp = neigh_data + i;
		if(temp->ip == ip) {
			*neighbor_entry = temp;
			return 0;
		}
	}
	return -1;
}

/**
 * Search node's geohash from it's ip addr.
 * return
 *
 */
u64 neighbor_getgeohash_fromip(u32 ip)
{
	int i;
	neighbor_table	*temp = neigh_data;
	wait_neigh_available();
	for(i = 0; i < *neigh_count; i++) {
		temp = neigh_data + i;
		if(temp->ip == ip)
			return temp->geoHash;
	}
	return 0;
}

/**
 * get a random node which is in the range of the geohash set.
 * return 0 if unmatched.
 */
u64 neighbor_getnode_fromset_random(neighbor_table** neighbor_entry, u64 *geohashset, int size)
{
	int i,j;
	neighbor_table *temp = neigh_data;
	wait_neigh_available();
	for(i = 0; i < *neigh_count; i++) {
		temp = neigh_data + i;
		for(j = 0; j < size; j++) {
			if(temp->geoHash == geohashset[j]) {
				*neighbor_entry = temp;
				return 1;
			}
		}
	}
	return 0;
}

