#ifndef MBR_H
#define MBR_H

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create()、kthread_run()
#include <linux/err.h> //IS_ERR()、PTR_ERR()

#include <net/sock.h>
#include <linux/netlink.h>

#include <linux/debugfs.h>
#include <linux/types.h>

#include <linux/slab.h>

#include "pkt_output.h"
#include "pkt_input.h"
#include "debug.h"
#include "graph.h"
#include "geohash.h"
#include "mbr_route.h"
#include "neighbors.h"

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
	struct dentry *dir;
	u64 geohash_this;
};


typedef struct graph_deliver
{
#define DEL_NODE	1
#define NEW_NODE	2
#define NEW_EDGE	3
    short mode; 	//操作类型，该字段为1代表删除一个节点，2代表添加一个节点，3代表添加一条边；
    union
    {
        struct  //如果是添加或者删除一个节点，该字段表示待添加或者待删除的节点id；
        {
            char vertex[25];
            u64 geohash;
        }vertex;	
        struct 	//如果是添加一条边，该字段表示添加的边的起始和终止节点以及该边所在道路的id；
        {
            char from[25];
            char to[25];
            int road_id;
        }edge;
    }parameter;
}graph_deliver;


#endif /* MBR_H */
