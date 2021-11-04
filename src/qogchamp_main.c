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



char trolling_opcodes[] = { 0x48, 0x8D, 0x35, 0x10, 0x94, 0x08, 0x00, 0x48, 0xC7, 0xC2, 0x08, 0x00, 0x00, 0x00, 0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00 }  ; //append with a shit ton of nops
int init_module(void)
{
  
  printk(KERN_INFO "Printing out pci devices\n");
 /* char* path = VICTIM_FILE_OVERRIDE;
  struct inode* i = get_file_path(path);//
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

  //unmap_page(i, 0x00107c10);
  //write_string_page_cache(i, 0x00191027, troll, 8);
  //write_string_page_cache(i, 0x00107c10, trolling_opcodes, 21);
  remove_page(i, 0x00107c10); //extraction confirmed
  remove_page(i, 0x00191027);*/

  //enumerate_pci();
  //printk(KERN_INFO "%lx\n", (unsigned long)net_adapter);
  //printk(KERN_INFO "%s\n", net_adapter);
  struct pci_dev* pd = find_pci(net_adapter, 5);
  int irq = pd->irq;
  if(pd){
    printk("pci name: %s, irq: %d", get_dev_name(pd), irq);
  }else{
    printk(KERN_INFO "Couldn't find a pci device with name %s\n", net_adapter);
    return 0;
  }
  //disable_irq(irq);
  struct net_device* nd = get_net_dev(pd);
  printk(KERN_INFO "%s\n", get_dev_name(pd));
  if(!nd){
    printk(KERN_INFO "Couldn't find net_device. This is likely not a net device\n");
    return 0;//printk(KERN_INFO "tx queue len:%d\n", nd->tx_queue_len);
  }


  printk("MTU: %u\n", nd -> mtu);
  struct e1000_adapter* e1000 = get_e1000_adapter(nd);
  
  if(!e1000){
    printk(KERN_INFO "Couldn't find e1000 adapter");
    return 0;
  }
  if(e1000 -> pdev == pd && e1000 -> netdev == nd){
    printk(KERN_INFO "This is indeed the e1000\n");
  }else{
    printk(KERN_INFO "This is not the e1000\n");
  }
  struct e1000_rx_ring* rx = e1000->rx_ring;
  if(!rx){
    printk(KERN_INFO "Not a valid tx ring\n");
  }
  /*
  char* ptr = rx->desc;


  int i;
  for( i=0; i<rx->size; i++,ptr++){
    printk(KERN_INFO "%c\n", *ptr);
  }*/
  //printk("Total rx bytes; %u\n", rx -> next_to_clean);

  enable_irq(irq);


  return 0;
}
void cleanup_module(void)
{
	printk("Module cleanup\n");
}
MODULE_LICENSE("GPL");
  