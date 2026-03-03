// The goal of this kernel driver module is to traverse all data packets
// in the block every 1 second, and resend any unsent packets
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
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/seg6_local.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhu Yihan");
MODULE_DESCRIPTION("Kernel module to check blocks for unsent packets and retransmit");

extern struct list_head block_list;

struct timer_list my_timer;
unsigned int timeout = 5000; // 1 second, timeout in milliseconds

// Function to retransmit packets, directly forwarding the existing skb
// int retransmit_packet(struct sk_buff *skb) {
//     struct dst_entry *dst = NULL;
//     struct flowi6 fl6;
//     struct neighbour *neigh;
//     struct ipv6hdr *ipv6h = ipv6_hdr(skb);
//     struct net_device *my_dev = NULL;
//     int res = 0;
//     struct net *net = NULL;

//     net = dev_net(skb->dev);

//     // set fl6
//     memset(&fl6, 0, sizeof(fl6));
//     fl6.flowi6_iif = skb->dev->ifindex;
//     fl6.flowi6_oif = skb->dev->ifindex;
//     fl6.daddr = ipv6h->daddr;
//     fl6.saddr = ipv6h->saddr;
//     fl6.flowlabel = ip6_flowinfo(ipv6h);
//     fl6.flowi6_mark = skb->mark;
//     fl6.flowi6_proto = ipv6h->nexthdr;

//     dst = ip6_route_output(&init_net, NULL, &fl6);
//     if (dst->error) {
//         return dst->error;
//     }
//     skb_dst_set(skb, dst);

//     neigh = neigh_lookup(&nd_tbl, &ipv6h->daddr, my_dev);
//     if (!neigh)
//         neigh = neigh_create(&nd_tbl, &ipv6h->daddr, my_dev);
//     if (!IS_ERR(neigh)) {
//         res = ip6_local_out(&init_net, NULL, skb);
//     } else {
//         res = -EFAULT;
//     }
//     return res;

//     // Directly forward the packet using dst_input
//     // return dst->input(skb);
// }

// Function to check the block for unsent packets and retransmit them
void check_and_retransmit(void) {
    struct block_info *block = NULL;
    struct packet_info *packet = NULL;

    list_for_each_entry(block, &block_list, list) {
        list_for_each_entry(packet, &block->packets_list, list) {
            // You can add additional logic here, such as checking for timeout
            printk(KERN_INFO "Retransmitting packet: %u\n", packet->packet_number);
            dst_input(skb_copy(packet->buff_skb, GFP_KERNEL));
            // retransmit_packet(packet->buff_skb); // Directly retransmit using the skb in the packet
        }
    }
}

// Timer callback function
static void my_timer_callback(struct timer_list *timer) {
    check_and_retransmit();
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(timeout));
}

// Module initialization function
static int __init my_module_init(void) {
    printk(KERN_INFO "Retransmit module loaded.\n");
    timer_setup(&my_timer, my_timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(timeout));
    return 0;
}

// Module exit function
static void __exit my_module_exit(void) {
    del_timer(&my_timer);
    printk(KERN_INFO "Retransmit module unloaded.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
