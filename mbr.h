#ifndef MBR_H
#define MBR_H

#include "pkt_output.h"
#include "pkt_input.h"
#include "debug.h"
#include "graph.h"
#include "geohash.h"

//Shared memory page size.
#define SHARED_MEM_DEVNAME "mbr_neighbor_mem"
#define SHARED_MEM_SIZE	4*4096

#define NETLINK_USER  22
#define USER_MSG    (NETLINK_USER + 1)
#define USER_PORT   50


//struct mbr_common {
//
//	int debug_mask;
//};

struct mbr_status
{
	u64 geohash_this;
};

typedef struct graph_deliver
{
    short mode; 	//操作类型，该字段为1代表删除一个节点，2代表添加一个节点，3代表添加一条边；
    union
    {
        char vertex[25];	//如果是添加或者删除一个节点，该字段表示待添加或者待删除的节点id；
        struct 	//如果是添加一条边，该字段表示添加的边的起始和终止节点以及该边所在道路的id；
        {
            char from[25];
            char to[25];
            int road_id;
        }edge;
    }parameter;
}graph_deliver;


#endif /* MBR_H */
