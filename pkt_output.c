#include "pkt_output.h"
#include "debug.h"
#include <linux/skbuff.h>
#include <linux/ip.h>

extern int debug_level;

int eth_header_dirty(struct sk_buff *skb, unsigned short type,
	       const void *daddr, const void *saddr, unsigned int len)
{
	struct ethhdr *eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);

	if(eth == NULL) {
		return -1;
	}
	if (type != ETH_P_802_3 && type != ETH_P_802_2)
		eth->h_proto = htons(type);
	else
		eth->h_proto = htons(len);

	/*
	 *      Set the source hardware address.
	 */

	if (saddr)
		memcpy(eth->h_source, saddr, ETH_ALEN);

	if (daddr) {
		memcpy(eth->h_dest, daddr, ETH_ALEN);

	}
	return ETH_HLEN;
}


unsigned int output_handler(const struct nf_hook_ops *ops, struct sk_buff *skb,
		const struct net_device *in, const struct net_device *out, int (*okfn) (struct sk_buff *))
{
	int err;
	//mbr_dbg(debug_level, ANY, "")
#define GEL01 {0x54,0x27,0x1e,0xa4,0xca,0xe3}
#define GEL02 {0x54,0x27,0x1e,0x1a,0x77,0x99}
#define ETH_P_RELAY 0x6559
	unsigned char	h_source[ETH_ALEN] = GEL02;
	unsigned char	h_dest[ETH_ALEN] = GEL01;



	if(skb == NULL) {
		mbr_dbg(debug_level, ANY, "SKB is null!\n");
		return NF_ACCEPT;
	}

	__skb_pull(skb, skb_network_offset(skb));
	//err = dev_hard_header(skb, skb->dev, ETH_P_IP, h_dest, h_source, skb->len);
	err = eth_header_dirty(skb, ETH_P_IP, h_dest, h_source, skb->len);
	if (err < 0)
		mbr_dbg(debug_level, ANY, "ERROR!! %d\n", err );

	mbr_dbg(debug_level, ANY, "OUTPUT: SAddress: %pI4, DAddress: %pI4\n", &ip_hdr(skb)->saddr, &ip_hdr(skb)->daddr);

	dev_queue_xmit(skb);

	return NF_STOLEN;
}

