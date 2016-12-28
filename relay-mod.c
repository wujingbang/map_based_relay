#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>

#include "pkt_output.h"
#include "pkt_input.h"
#include "debug.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WU Jingbang");
MODULE_DESCRIPTION("Digital map based mac relay protocol.");

int debug_level = MBR_DBG_DEFAULT;
module_param_named(debug, debug_level, uint, 0);
MODULE_PARM_DESC(debug, "Debugging mask");

struct nf_hook_ops input_filter;
struct nf_hook_ops output_filter;


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
	return 0;
}

static void __exit cleanup_relay_module(void)
{
	nf_unregister_hook(&input_filter);
	nf_unregister_hook(&output_filter);

}

module_init(init_relay_module);
module_exit(cleanup_relay_module);
