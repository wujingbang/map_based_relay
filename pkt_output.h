#include <linux/netdevice.h>

unsigned int output_handler(const struct nf_hook_ops *ops, struct sk_buff *skb,
                            const struct net_device *in, const struct net_device *out, int (*okfn) (struct sk_buff *));
