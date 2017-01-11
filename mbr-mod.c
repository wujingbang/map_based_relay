#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>

#include "pkt_output.h"
#include "pkt_input.h"
#include "debug.h"
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

static unsigned char * malloc_reserved_mem(unsigned int size){
    unsigned char *p = kmalloc(size, GFP_KERNEL);
    unsigned char *tmp = p;
    unsigned int i, n;
    if (NULL == p){
        printk("Error : malloc_reserved_mem kmalloc failed!\n");
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
    mbr_dbg(debug_level, ANY, "in mem_mmap\n");
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long physics = ((unsigned long )mem_msg_buf)-PAGE_OFFSET;
    unsigned long mypfn = physics >> PAGE_SHIFT;
    unsigned long vmsize = vma->vm_end-vma->vm_start;
    unsigned long psize = gsize - offset;
    if(vmsize > psize)
        return -ENXIO;
    if(remap_pfn_range(vma,vma->vm_start,mypfn,vmsize,vma->vm_page_prot))
        return -EAGAIN;
    return 0;
}

struct file_operations sharedMemOps = {
    .read = char_read,
    .write = char_write,
    .open = char_open,
    .release = char_release,
    .mmap = mem_mmap,
};

static struct miscdevice shared_mem_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = SHARED_MEM_DEVNAME,
	.fops = &sharedMemOps,
};

static int __init init_relay_module(void)
{
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

	/**
	 * Initial shared memory for neighbor.
	 */
	shared_mem_neighbor = malloc_reserved_mem(SHARED_MEM_SIZE);
	ret = misc_register(&shared_mem_misc);
	mbr_dbg(debug_level, ANY, SHARED_MEM_DEVNAME" initialized\n");

	return 0;
}

static void __exit cleanup_relay_module(void)
{
	nf_unregister_hook(&input_filter);
	nf_unregister_hook(&output_filter);
	misc_deregister(&shared_mem_misc);
	kfree(mem_msg_buf);

}

module_init(init_relay_module);
module_exit(cleanup_relay_module);
