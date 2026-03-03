#include <linux/icmpv6.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ipv6.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/seg6.h>
#include <linux/slab.h>
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/seg6_local.h>
#include <linux/printk.h>
#include <net/ip6_route.h>
#include <net/seg6.h>
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
#include <linux/if_ether.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhu Yihan");
MODULE_DESCRIPTION("Kernel module to print skb info");

extern struct list_head block_list;

void print_info(void){
    struct block_info *block = NULL;
    struct packet_info *packet = NULL;
    int i = 0;
    int offset = 0;
    char addr_str[INET6_ADDRSTRLEN];

    list_for_each_entry(block, &block_list, list) {
        snprintf(addr_str, INET6_ADDRSTRLEN, "%pI6c", &block->nhaddr);
        printk(KERN_INFO "Block Number: %u, Packets: %u, Address: %s\n", block->block_number, block->packets_number, addr_str);
        list_for_each_entry(packet, &block->packets_list, list) {
            printk(KERN_INFO "Packet Number: %u\n", packet->packet_number);
        }

    }
}

static int __init print_skb_info_init(void){
    printk(KERN_INFO "Print skb info module loaded\n");
    print_info();
    return 0;
}

static void __exit print_skb_info_exit(void){
    printk(KERN_INFO "Print skb info module unloaded\n");
}

module_init(print_skb_info_init);
module_exit(print_skb_info_exit);

