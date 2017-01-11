#include "mbr_route.h"

LIST_HEAD(relay_table)
/**
 * Search routing table for the GeoHash of next-hop.
 * return:
 * 	0:
 * 	-1:
 */
uint8_t* search_rtable(uint8_t* dstGeoHash)
{
	relay_table_list *entry;
	list_for_each_entry(entry, &relay_table, ptr){

	}

}

int update_rtable()
{

}
