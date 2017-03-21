
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
u64 neighbor_getnode_fromset_random(neighbor_table** neighbor_entry, GeoHashSetCoordinate *geohashset)
{
	int i,j,k;

	if(unlikely(geohashset->sx != 3 && geohashset->sy != 3)) {
		mbr_dbg(debug_level, ANY, "neighbor_getnode_fromset_random: does not support sx: %d, sy: %d!\n",
				geohashset->sx, geohashset->sy);
		return -1;
	}

	neighbor_table *temp = neigh_data;
	wait_neigh_available();
	for(i = 0; i < *neigh_count; i++) {
		temp = neigh_data + i;
		for(j = 0; j < geohashset->sy; j++) {
			for(k=0; k < geohashset->sx; k++) {
				if(temp->geoHash == geohashset->geohashset[j][k]) {
					*neighbor_entry = temp;
					return 1;
				}
			}
		}
	}
	return 0;
}

int get_dtime(neighbor_table *node_info, GeoHashSetCoordinate *geohashset)
{
	int i, j;
	int direct = node_info->direction;

	if(unlikely(geohashset->sx != 3 && geohashset->sy != 3)) {
		mbr_dbg(debug_level, ANY, "get_dtime: does not support sx: %d, sy: %d!\n",
				geohashset->sx, geohashset->sy);
		return -1;
	}

	for(i = 0; i < geohashset->sy; i++) {
		for(j=0; j < geohashset->sx; j++) {
			if(node_info->geoHash == geohashset->geohashset[i][j]) goto direct;
		}
	}

direct:

	if(45 <= direct && direct < 135) {
		return j % geohashset->sx;
	}
	if(225 <= direct && direct < 315) {
		return j + 1;
	}
	if(135 <= direct && direct < 225) {
		return i % geohashset->sy;
	}
	if(315 <= direct || direct < 45) {
		return i + 1;
	}

}

/**
 * get the best node which is in the range of the geohash set based on the "relay zone dwelling time".
 * return 0 if unmatched.
 */
u64 neighbor_getnode_fromset_best(neighbor_table** neighbor_entry, GeoHashSetCoordinate *geohashset)
{
	int i;
	int dtime_best = 0;
	int dtime_curr = 0;
	int dtime_max = (geohashset->sx > geohashset->sy) ? geohashset->sx : geohashset->sy;
	neighbor_table *temp = neigh_data;
	neighbor_table *best;

	if(unlikely(geohashset->sx != 3 && geohashset->sy != 3)) {
		mbr_dbg(debug_level, ANY, "neighbor_getnode_fromset_best: does not support sx: %d, sy: %d!\n",
				geohashset->sx, geohashset->sy);
		return -1;
	}

	wait_neigh_available();
	for(i = 0; i < *neigh_count; i++) {
		temp = neigh_data + i;
		dtime_curr = get_dtime(temp, geohashset);
		if ( dtime_curr == dtime_max ) {
			*neighbor_entry = temp;
			return 0;
		}
		else {
			dtime_best = (dtime_curr > dtime_best) ? dtime_curr : dtime_best;
			best = temp;
		}
	}
	*neighbor_entry = best;
	return 0;
}
