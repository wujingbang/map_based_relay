#ifndef MBR_H
#define MBR_H

#ifdef LINUX_KERNEL
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create()銆乲thread_run()
#include <linux/err.h> //IS_ERR()銆丳TR_ERR()

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
#define SHARED_MEM_SIZE 4*4096

#define NETLINK_USER  22
#define USER_MSG    (NETLINK_USER + 1)
#define USER_PORT   50


//struct mbr_common {
//
//  int debug_mask;
//};

struct mbr_status
{
    struct dentry *dir;
    uint64_t geohash_this;
    int mbr_start;
};


typedef struct graph_deliver
{
#define DEL_NODE    1
#define NEW_NODE    2
#define NEW_EDGE    3
    short mode;     //鎿嶄綔绫诲瀷锛岃瀛楁涓�1浠ｈ〃鍒犻櫎涓�涓妭鐐癸紝2浠ｈ〃娣诲姞涓�涓妭鐐癸紝3浠ｈ〃娣诲姞涓�鏉¤竟锛�
    union
    {
        struct  //濡傛灉鏄坊鍔犳垨鑰呭垹闄や竴涓妭鐐癸紝璇ュ瓧娈佃〃绀哄緟娣诲姞鎴栬�呭緟鍒犻櫎鐨勮妭鐐筰d锛�
        {
            char vertex[25];
            uint64_t geohash;
        }vertex;    
        struct  //濡傛灉鏄坊鍔犱竴鏉¤竟锛岃瀛楁琛ㄧず娣诲姞鐨勮竟鐨勮捣濮嬪拰缁堟鑺傜偣浠ュ強璇ヨ竟鎵�鍦ㄩ亾璺殑id锛�
        {
            char from[25];
            char to[25];
            int road_id;
        }edge;
    }parameter;
}graph_deliver;
#else




#endif /* LINUX_KERNEL */
#endif /* MBR_H */
