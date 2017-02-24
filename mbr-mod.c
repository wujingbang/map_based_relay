#include "mbr.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WU Jingbang");
MODULE_DESCRIPTION("Digital map based mac relay protocol.");

int debug_level = MBR_DBG_DEFAULT;
module_param_named(debug, debug_level, uint, 0);
MODULE_PARM_DESC(debug, "Debugging mask");

struct nf_hook_ops input_filter;
struct nf_hook_ops output_filter;

//Shared memory
unsigned char *shared_mem_neighbor = NULL;
//mbr status
struct mbr_status global_mbr_status;
//netlink
static struct sock *netlinkfd;
//graph
Graph *global_graph;

static unsigned char * malloc_reserved_mem(unsigned int size){
    unsigned char *p = kmalloc(size, GFP_KERNEL);
    unsigned char *tmp = p;
    unsigned int i, n;
    if (NULL == p){
    	mbr_dbg(debug_level, ANY, "Error : malloc_reserved_mem kmalloc failed!\n");
        return NULL;
    }
    n = size / 4096 + 1;
    if (0 == size % 4096 ){
        --n;
    }
    for (i = 0; i < n; ++i){
        SetPageReserved(virt_to_page(tmp));
        tmp += 4096;
    }
    return p;
}

int mem_mmap(struct file *filp, struct vm_area_struct *vma){
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long physics = ((unsigned long )shared_mem_neighbor)-PAGE_OFFSET;
    unsigned long mypfn = physics >> PAGE_SHIFT;
    unsigned long vmsize = vma->vm_end-vma->vm_start;
    unsigned long psize = SHARED_MEM_SIZE - offset;
    mbr_dbg(debug_level, ANY, "in mem_mmap\n");
    if(vmsize > psize)
        return -ENXIO;
    if(remap_pfn_range(vma,vma->vm_start,mypfn,vmsize,vma->vm_page_prot))
        return -EAGAIN;
    return 0;
}


int open_generic(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

struct file_operations sharedMemOps = {
    .open = open_generic,
    .mmap = mem_mmap,
};

static struct miscdevice shared_mem_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = SHARED_MEM_DEVNAME,
	.fops = &sharedMemOps,
};

struct dentry *mbr_status_create_file(const char *name,
				 umode_t mode,
				 struct dentry *parent,
				 void *data,
				 const struct file_operations *fops)
{
	struct dentry *ret;

	ret = debugfs_create_file(name, mode, parent, data, fops);
	if (!ret)
		mbr_dbg(debug_level, ANY, "Could not create debugfs '%s' entry\n", name);

	return ret;
}

int status_open_generic(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

static int
status_geohash_read(struct file *filp, char __user *ubuf,
		    size_t cnt, loff_t *ppos)
{
	char buf[64];
	int r;
	struct mbr_status *st = filp->private_data;

	r = snprintf(buf, sizeof(buf), "%lld\n", st->geohash_this);
	if (r > sizeof(buf))
		r = sizeof(buf);
	return simple_read_from_buffer(ubuf, cnt, ppos, buf, r);
}

static int
status_geohash_write(struct file *filp, char __user *ubuf,
		    size_t cnt, loff_t *ppos)
{
	u64 val;
	int ret;
	struct mbr_status *st = filp->private_data;
	ret = kstrtoull_from_user(ubuf, cnt, 10, &val);
	if (ret)
		return ret;

	if(val == 0) {
		mbr_dbg(debug_level, ANY, "status_geohash_write: geohash can not be zero!!\n");
		return 0;
	}
	st->geohash_this = val;
	/* UPDATE mbr table! */
	update_mbrtable_outrange(val);

	return cnt;

}

static const struct file_operations status_geohash = {
	.open		= status_open_generic,
	.read		= status_geohash_read,
	.write		= status_geohash_write,
};

static int mbr_create_debugfs(void)
{
	struct dentry *d_status;

	if (!debugfs_initialized())
			return NULL;
	global_mbr_status.dir = debugfs_create_dir("mbr", NULL);
	global_mbr_status.geohash_this = 0;
	global_mbr_status.mbr_start = 0;
	d_status = global_mbr_status.dir;

	mbr_status_create_file("geohash", 0644, d_status,
			&global_mbr_status, &status_geohash);
	mbr_status_create_file("mbr_start", 0644, d_status,
			&global_mbr_status, &status_geohash);

}



int netlink_send_msg(char *pbuf, uint16_t len)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;

    int ret;

    nl_skb = nlmsg_new(len, GFP_ATOMIC);
    if(!nl_skb)
    {
    	mbr_dbg(debug_level, ANY,"netlink_alloc_skb error\n");
        return -1;
    }

    nlh = nlmsg_put(nl_skb, 0, 0, USER_MSG, len, 0);
    if(nlh == NULL)
    {
        printk("nlmsg_put() error\n");
        nlmsg_free(nl_skb);
        return -1;
    }
    memcpy(nlmsg_data(nlh), pbuf, len);

    ret = netlink_unicast(netlinkfd, nl_skb, USER_PORT, MSG_DONTWAIT);

    return ret;
}


static void netlink_recv_cb(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    void *data = NULL;
    graph_deliver *d;
    printk("skb->len:%u\n", skb->len);
    if(skb->len >= nlmsg_total_size(0))
    {
        nlh = nlmsg_hdr(skb);
        data = NLMSG_DATA(nlh);
        d=(graph_deliver*)data;

        switch(d->mode) {
        case DEL_NODE:
        	graph_remove_vertex_undirect(global_graph, getVertex(global_graph, d->parameter.vertex.vertex));
        	break;
        case NEW_NODE:
        	graph_add_vertex(global_graph, vertex_create(d->parameter.vertex.vertex,d->parameter.vertex.geohash));
        	break;
        case NEW_EDGE:
        	vertex_add_edge_to_vertex_undirect(getVertex(global_graph, d->parameter.edge.from),getVertex(global_graph, d->parameter.edge.to), d->parameter.edge.road_id);
        	break;
        default:
        	mbr_dbg(debug_level, ANY, "netlink_recv_cb: graph cmd error!\n");
        }

        mbr_dbg(debug_level, ANY, "kernel receive data: %hd ", d->mode);
        netlink_send_msg(data, nlmsg_len(nlh));
    }
}


struct netlink_kernel_cfg cfg =
{
    .input = netlink_recv_cb,
};

static int __init init_mbr_module(void)
{
	/**
	 * Initial shared memory for neighbor.
	 * Initial neighbor list
	 */
	shared_mem_neighbor = malloc_reserved_mem(SHARED_MEM_SIZE);
	misc_register(&shared_mem_misc);
	mbr_dbg(debug_level, ANY, SHARED_MEM_DEVNAME" initialized\n");
	neigh_list_init();

	/**
	 * initial netlink for graph
	 * Initial graph
	 */
    netlinkfd = netlink_kernel_create(&init_net, USER_MSG, &cfg);
    if(!netlinkfd)
    {
    	mbr_dbg(debug_level, ANY, "can not create a netlink socket!\n");
        return -1;
    }
    global_graph = graph_create();

    /**
     * initial debugfs for geohash update
     */
    mbr_create_debugfs();

	//netfilter stuff
	// input hook
	input_filter.list.next = NULL;
	input_filter.list.prev = NULL;
	input_filter.hook = input_handler;
	input_filter.pf = PF_INET; // IPv4
	input_filter.hooknum = NF_INET_PRE_ROUTING; //UNTESTED!!!!!!!!

	//output hook
	output_filter.list.next = NULL;
	output_filter.list.prev = NULL;
	output_filter.hook = output_handler;
	output_filter.pf = PF_INET; // IPv4
	output_filter.hooknum = NF_INET_POST_ROUTING;

	nf_register_hook(&output_filter);
	nf_register_hook(&input_filter);

	return 0;
}

static void __exit cleanup_mbr_module(void)
{
	nf_unregister_hook(&input_filter);
	nf_unregister_hook(&output_filter);
	misc_deregister(&shared_mem_misc);
	kfree(shared_mem_neighbor);
	debugfs_remove_recursive(global_mbr_status.dir);
	sock_release(netlinkfd->sk_socket);
	graph_free(global_graph);
}

module_init(init_mbr_module);
module_exit(cleanup_mbr_module);
