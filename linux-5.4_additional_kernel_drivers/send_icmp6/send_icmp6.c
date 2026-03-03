#include <asm-generic/errno-base.h>
#include <linux/icmpv6.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ipv6.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/printk.h>
#include <net/ip6_route.h>
#include <linux/in.h>
#include <linux/etherdevice.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/route.h>
#include <linux/net.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/in6.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/string.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhu Yihan");
MODULE_DESCRIPTION("Kernel module to send icmp6 message");


static int send_icmp6_message(u8 code) 
{
    struct sk_buff *skb = NULL;
    struct icmp6hdr *icmp6h = NULL;
    struct ipv6hdr *ipv6h = NULL;
    struct dst_entry *dst = NULL;
    struct flowi6 fl6;
    struct neighbour *neigh;
    struct in6_addr src_addr, dst_addr;
    struct net_device *my_dev = NULL;
    const char *my_ip6 = "fc01::1";
    const char *not_my_ip6 = "fc01::2";
    int res = 0;
    struct net *net = NULL;
// int flags = RT6_LOOKUP_F_HAS_SADDR;


    skb = alloc_skb(sizeof(struct icmp6hdr) + sizeof(struct ipv6hdr) + 8, GFP_KERNEL);
    if (!skb) {
        printk(KERN_INFO "alloc_skb() fault\n");
        return -ENOMEM;
    }


    skb_reserve(skb, ETH_ZLEN);
    skb->len = sizeof(struct icmp6hdr) + sizeof(struct ipv6hdr) + 8;
    skb->protocol = htons(ETH_P_IPV6);
    skb->priority = 0;
    skb->pkt_type = PACKET_OUTGOING;
    skb_set_network_header(skb, 0);
    skb_set_transport_header(skb, sizeof(struct ipv6hdr));


    ipv6h = (struct ipv6hdr *)(skb->head + skb->tail);
    ipv6h->version = 6;
    ipv6h->priority = 0;
    ipv6h->payload_len = htons(sizeof(struct icmp6hdr) + 8);
    ipv6h->nexthdr = IPPROTO_ICMPV6;
    ipv6h->hop_limit = 64;
    res = in6_pton(my_ip6, -1, src_addr.s6_addr, -1, NULL);
    res = in6_pton(not_my_ip6, -1, dst_addr.s6_addr, -1, NULL);
    ipv6h->saddr = src_addr;
    ipv6h->daddr = dst_addr;
    skb->tail += sizeof(struct ipv6hdr);

    icmp6h = (struct icmp6hdr *)(skb->head + skb->tail);
    icmp6h->icmp6_type = 220;
    icmp6h->icmp6_code = code;
    icmp6h->icmp6_cksum = 0;
    icmp6h->icmp6_dataun.u_block_msg.block_number = htons(1);
    icmp6h->icmp6_dataun.u_block_msg.packet_number = htons(1);
    icmp6h->icmp6_cksum = csum_ipv6_magic(&ipv6h->saddr, &ipv6h->daddr, sizeof(struct icmp6hdr) + 8, IPPROTO_ICMPV6, csum_partial(icmp6h, sizeof(struct icmp6hdr) + 8, 0));
    skb->tail += sizeof(struct icmp6hdr) + 8;

    my_dev = dev_get_by_name(&init_net, "ens33");
    if (my_dev) {
        printk(KERN_INFO "dev->name = %s", my_dev->name);
    } else {
        printk(KERN_WARNING "dev_get_by_name() error");
        return -EINVAL;
    }
    skb->dev = my_dev;
    net = dev_net(skb->dev);

    // set fl6
    memset(&fl6, 0, sizeof(fl6));
    fl6.flowi6_iif = skb->dev->ifindex;
    fl6.flowi6_oif = skb->dev->ifindex;
    fl6.daddr = ipv6h->daddr;
    fl6.saddr = ipv6h->saddr;
    fl6.flowlabel = ip6_flowinfo(ipv6h);
    fl6.flowi6_mark = skb->mark;
    fl6.flowi6_proto = ipv6h->nexthdr;

    dst = ip6_route_output(&init_net, NULL, &fl6);
    if (dst->error) {
        return dst->error;
    }
    skb_dst_set(skb, dst);

    neigh = neigh_lookup(&nd_tbl, &ipv6h->daddr, skb->dev);
    if (!neigh)
        neigh = neigh_create(&nd_tbl, &ipv6h->daddr, skb->dev);
    if (!IS_ERR(neigh)) {
        res = ip6_local_out(&init_net, NULL, skb);
    } else {
        res = -EFAULT;
    }
    return res;

    // dst = ip6_route_input_lookup(net, skb->dev, &fl6, skb, flags);

    // if (dst && dst->dev->flags & IFF_LOOPBACK && !dst->error) {
    //     dst_release(dst);
    //     dst = NULL;
    // }
    // skb_dst_set(skb, dst);
    // return dst_input(skb);
}

static int __init send_icmp6_init(void) {
    printk(KERN_INFO "Send icmp6 module loaded.\n");
    send_icmp6_message(1);
    return 0;
}

static void __exit send_icmp6_exit(void) {
    printk(KERN_INFO "Send icmp6 module unloaded.\n");
}

module_init(send_icmp6_init);
module_exit(send_icmp6_exit);
