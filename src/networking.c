#include "networking.h"
#include "e1000.h"
#include "e1000_hw.h"
#include "e1000_osdep.h"
/*
Eventually a good portion of these functions will likely be moved to their own pci file
*/


/*
Random notes:
* pci_name() is useless and does not give any useful information whatsoever. Use the driver names
* In the case of the e1000, the driver data is filled with the net_dev struct
* Disabling the irq will, as expected, stop the NIC from working
* Cool resource on rx/tx and dma ring buffers:https://newbedev.com/what-is-the-relationship-of-dma-ring-buffer-and-tx-rx-ring-for-a-network-card


*/

struct e1000_frame{ //we conviently get the same view as wireshark. So, we can just copy from there.
	
	struct eth_frame* eth;


};



void* get_rx_buffer(struct e1000* e1000){
    if(!e1000){return NULL;}
    return NULL;
}

struct e1000_adapter* get_e1000_adapter(struct net_device* net_dev){
    return (struct e1000_adapter*) netdev_priv(net_dev);
}

struct pci_dev* find_pci(const char* d_name, int size){ 
    struct pci_dev* d; 
    printk(KERN_INFO "Starting find_pci with %s\n", d_name); //for some reason this function will die if this line is not here
    for_each_pci_dev(d){ //macro in pci.h
        if(!d -> driver || !(d -> driver -> name))
            continue;
        //printk(KERN_INFO "Found device: %s\n", d->driver->name);
        if(!strncmp(d->driver->name, d_name, size))
            return d;
        
        
    }
    return NULL;
}
struct net_device* get_net_dev(struct pci_dev* pd){
    return pci_get_drvdata(pd);
}
void enumerate_pci(void){
    struct pci_dev* d;
    for_each_pci_dev(d){ //macro in pci.h
        if(!(d -> driver) || !(d -> driver -> name))
            continue;
        printk(KERN_INFO "%s\n", d->driver->name); 
    }
}
const char* get_pci_name(struct pci_dev* pd){ //this gets the "true name" - the name of the driver. It can be considered the common name
    if(!pd -> driver){
        return NULL;
    }
    return pd->driver->name;
}

const char* get_dev_name(struct pci_dev* pd){ //this gets the name of the pci device on the Linux system. This is useful to search sysfs
    return pci_name(pd);
}


void e1000_dump(struct e1000_adapter *adapter) //yoinked from e1000_main.c
{
	/* this code doesn't handle multiple rings */
	struct e1000_tx_ring *tx_ring = adapter->tx_ring;
	struct e1000_rx_ring *rx_ring = adapter->rx_ring;
	int i;

	/*if (!netif_msg_hw(adapter)){
        pr_info("Failure\n");
		return;
    }*/

	/* Print Registers */
	//e1000_regdump(adapter);

	/* transmit dump */
	pr_info("TX Desc ring0 dump\n");

	/* Transmit Descriptor Formats - DEXT[29] is 0 (Legacy) or 1 (Extended)
	 *
	 * Legacy Transmit Descriptor
	 *   +--------------------------------------------------------------+
	 * 0 |         Buffer Address [63:0] (Reserved on Write Back)       |
	 *   +--------------------------------------------------------------+
	 * 8 | Special  |    CSS     | Status |  CMD    |  CSO   |  Length  |
	 *   +--------------------------------------------------------------+
	 *   63       48 47        36 35    32 31     24 23    16 15        0
	 *
	 * Extended Context Descriptor (DTYP=0x0) for TSO or checksum offload
	 *   63      48 47    40 39       32 31             16 15    8 7      0
	 *   +----------------------------------------------------------------+
	 * 0 |  TUCSE  | TUCS0  |   TUCSS   |     IPCSE       | IPCS0 | IPCSS |
	 *   +----------------------------------------------------------------+
	 * 8 |   MSS   | HDRLEN | RSV | STA | TUCMD | DTYP |      PAYLEN      |
	 *   +----------------------------------------------------------------+
	 *   63      48 47    40 39 36 35 32 31   24 23  20 19                0
	 *
	 * Extended Data Descriptor (DTYP=0x1)
	 *   +----------------------------------------------------------------+
	 * 0 |                     Buffer Address [63:0]                      |
	 *   +----------------------------------------------------------------+
	 * 8 | VLAN tag |  POPTS  | Rsvd | Status | Command | DTYP |  DTALEN  |
	 *   +----------------------------------------------------------------+
	 *   63       48 47     40 39  36 35    32 31     24 23  20 19        0
	 */
	/*pr_info("Tc[desc]     [Ce CoCsIpceCoS] [MssHlRSCm0Plen] [bi->dma       ] leng  ntw timestmp         bi->skb\n");
	pr_info("Td[desc]     [address 63:0  ] [VlaPoRSCm1Dlen] [bi->dma       ] leng  ntw timestmp         bi->skb\n");

	if (!netif_msg_tx_done(adapter))
		goto rx_ring_summary;*/

	for (i = 0; tx_ring->desc && (i < tx_ring->count); i++) {
		struct e1000_tx_desc *tx_desc = E1000_TX_DESC(*tx_ring, i);
		struct e1000_tx_buffer *buffer_info = &tx_ring->buffer_info[i];
		struct my_u { __le64 a; __le64 b; };
		struct my_u *u = (struct my_u *)tx_desc;
		const char *type;

		if (i == tx_ring->next_to_use && i == tx_ring->next_to_clean)
			type = "NTC/U";
		else if (i == tx_ring->next_to_use)
			type = "NTU";
		else if (i == tx_ring->next_to_clean)
			type = "NTC";
		else
			type = "";

		pr_info("T%c[0x%03X]    %016llX %016llX %016llX %04X  %3X %016llX %p %s\n",
			((le64_to_cpu(u->b) & (1<<20)) ? 'd' : 'c'), i,
			le64_to_cpu(u->a), le64_to_cpu(u->b),
			(u64)buffer_info->dma, buffer_info->length,
			buffer_info->next_to_watch,
			(u64)buffer_info->time_stamp, buffer_info->skb, type);
	}

rx_ring_summary:
	/* receive dump */
	pr_info("\nRX Desc ring dump\n");

	/* Legacy Receive Descriptor Format
	 *
	 * +-----------------------------------------------------+
	 * |                Buffer Address [63:0]                |
	 * +-----------------------------------------------------+
	 * | VLAN Tag | Errors | Status 0 | Packet csum | Length |
	 * +-----------------------------------------------------+
	 * 63       48 47    40 39      32 31         16 15      0
	 */
	//pr_info("R[desc]      [address 63:0  ] [vl er S cks ln] [bi->dma       ] [bi->skb]\n");

	/*if (!netif_msg_rx_status(adapter))
		goto exit;*/

	for (i = 0; rx_ring->desc && (i < rx_ring->count); i++) {
		struct e1000_rx_desc *rx_desc = E1000_RX_DESC(*rx_ring, i);
		struct e1000_rx_buffer *buffer_info = &rx_ring->buffer_info[i];
		struct my_u { __le64 a; __le64 b; };
		struct my_u *u = (struct my_u *)rx_desc;
		const char *type;

		if (i == rx_ring->next_to_use)
			type = "NTU";
		else if (i == rx_ring->next_to_clean)
			type = "NTC";
		else
			type = "";

		pr_info("R[0x%03X]     %016llX %016llX %016llX %p %s\n",
			i, le64_to_cpu(u->a), le64_to_cpu(u->b),
			(u64)buffer_info->dma, buffer_info->rxbuf.data, type);
	} /* for */

	/* dump the descriptor caches */
	/* rx */
	pr_info("Rx descriptor cache in 64bit format\n");
	for (i = 0x6000; i <= 0x63FF ; i += 0x10) {
		pr_info("R%04X: %08X|%08X %08X|%08X\n",
			i,
			readl(adapter->hw.hw_addr + i+4),
			readl(adapter->hw.hw_addr + i),
			readl(adapter->hw.hw_addr + i+12),
			readl(adapter->hw.hw_addr + i+8));
	}
	/* tx */
	pr_info("Tx descriptor cache in 64bit format\n");
	for (i = 0x7000; i <= 0x73FF ; i += 0x10) {
		pr_info("T%04X: %08X|%08X %08X|%08X\n",
			i,
			readl(adapter->hw.hw_addr + i+4),
			readl(adapter->hw.hw_addr + i),
			readl(adapter->hw.hw_addr + i+12),
			readl(adapter->hw.hw_addr + i+8));
	}
exit:
	return;
}
