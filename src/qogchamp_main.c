#include "taskutil.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"
#include "networking.h"
#include "e1000.h"
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
  /*char* path = VICTIM_FILE_OVERRIDE;//TOSTRING(VICTIM_FILE);//;//VICTIM_FILE_OVERRIDE;////VICTIM_FILE_OVERRIDE;//TOSTRING(VICTIM_FILE);//VICTIM_FILE_OVERRIDE;//TOSTRING(VICTIM_FILE);//VICTIM_FILE_OVERRIDE;//TOSTRING(VICTIM_FILE)
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
  if(pd){
    printk("pci name: %s, irq: %d", get_dev_name(pd), pd->irq);
  }
  struct net_device* nd = get_net_dev(pd);

  if(nd){
    printk(KERN_INFO "tx queue len:%d\n", nd->tx_queue_len);
  }
  return 0;
}
void cleanup_module(void)
{
	printk("Module cleanup\n");
}
MODULE_LICENSE("GPL");
  