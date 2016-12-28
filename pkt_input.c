#include "pkt_input.h"
#include "debug.h"
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/ip.h>

extern int debug_level;

unsigned int input_handler(const struct nf_hook_ops *ops, struct sk_buff *skb,
		const struct net_device *in, const struct net_device *out, int (*okfn) (struct sk_buff *))
{
	struct ethhdr * mac_header = eth_hdr(skb);
	mbr_dbg(debug_level, ANY, "type: 0x%x, smac: %x:%x:%x:%x:%x:%x,  dmac: %x:%x:%x:%x:%x:%x\n",
			mac_header->h_proto,
			mac_header->h_source[0], mac_header->h_source[1], mac_header->h_source[2],
			mac_header->h_source[3], mac_header->h_source[4], mac_header->h_source[5],
			mac_header->h_dest[0],mac_header->h_dest[1],mac_header->h_dest[2],
			mac_header->h_dest[3],mac_header->h_dest[4],mac_header->h_dest[5]);

	struct iphdr *ip= ip_hdr(skb);

	mbr_dbg(debug_level, ANY, "SAddress: %pI4, DAddress: %pI4\n", &ip->saddr, &ip->daddr);

	return NF_ACCEPT;
}
