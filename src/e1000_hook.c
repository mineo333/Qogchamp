#include "e1000_hook.h"
#include "e1000_hw.h"
#include "e1000_osdep.h"
#include "networking.h"

struct e1000_adapter* get_e1000_adapter(struct net_device* net_dev){
    return (struct e1000_adapter*) netdev_priv(net_dev);
}


//THE FUNCTIONS BELOW ARE ALL COPIED FROM THE E1000 DRIVER


#define COPYBREAK_DEFAULT 256
unsigned int copybreak __read_mostly = COPYBREAK_DEFAULT;



#define E1000_HEADROOM (NET_SKB_PAD + NET_IP_ALIGN)
unsigned int e1000_frag_len(const struct e1000_adapter *a)
{
	return SKB_DATA_ALIGN(a->rx_buffer_len + E1000_HEADROOM) +
		SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
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
	skb->protocol = eth_type_trans(skb, adapter->netdev);

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
	rx_desc = E1000_RX_DESC(*rx_ring, i);
	buffer_info = &rx_ring->buffer_info[i];

	while (rx_desc->status & E1000_RXD_STAT_DD) {
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
			//


			//printk("Didn't copybreak. Copybreak: %d, Length: %d, Frag Len: %d", copybreak, length, frag_len);	
			skb = build_skb(data - E1000_HEADROOM, frag_len);
			//
			/*
			Upon building the SKB, we get the following state:
			This information can be found in build_skb -> __build_skb -> __build_skb_around
			head --> data --> tail --> ___________________________
									   |						  |
									   |						  |
									   | E1000_HEADROOM (64 bytes)|
									   |						  |
									   |						  |
									   |__________________________|
									   |						  |
									   |						  |
									   |						  |
									   |						  |
									   |	Actual data			  |
									   |True length: rx_desc ->len|
									   |Max length: frag_len(1522)|
									   |						  |
									   |__________________________|
									   |						  |
									   |						  |
									   |  sizeof(skb_shared_info) |
									   |						  |
									   |						  |
								end--> |__________________________|
					
			
			*/

			if (!skb) {
				adapter->alloc_rx_buff_failed++;
				break;
			}

			skb_reserve(skb, E1000_HEADROOM);
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
		total_rx_bytes += (length - 4); /* don't count FCS */
		total_rx_packets++;

		if (likely(!(netdev->features & NETIF_F_RXFCS)))
			/* adjust length to remove Ethernet CRC, this must be
			 * done after the TBI_ACCEPT workaround above
			 */
			length -= 4;

		if (buffer_info->rxbuf.data == NULL) //not copybreak skb. Note the last line in the !skb block
			skb_put(skb, length); 
		else /* copybreak skb */
			skb_trim(skb, length);



		/* Receive Checksum Offload */
		e1000_rx_checksum(adapter,
				  (u32)(status) |
				  ((u32)(rx_desc->errors) << 24),
				  le16_to_cpu(rx_desc->csum), skb);

		e1000_receive_skb(adapter, status, rx_desc->special, skb);

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