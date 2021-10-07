
#include "taskutil.h"
#include "regset.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"

#ifndef VICTIM_FILE
#define VICTIM_FILE "/lib/x86_64-linux-gnu/libc-2.33.so"
#endif

#define STRINGIFY(A) #A
#define TOSTRING(A) STRINGIFY(A) //I don't why the fuck this works. If I had to take a guess, it is because when you pass in A in TOSTRING, you pass in the value of A which is then stringifyed.

int init_module(void)
{
 	void* map;
  int ret;
  struct page* page;
	struct inode* i = get_file_path(TOSTRING(VICTIM_FILE));
  if(!i){
    printk(KERN_INFO "Invalid path\n");
    return 0;
  }
  printk(KERN_INFO "Path Succeeded\n");



  if(i -> i_mapping == NULL){
    printk(KERN_INFO "This page does not have an address_space object\n");
    return 0;
  }
  printk(KERN_INFO "This page has an address_space object\n");
	//page = find_get_page(i->i_mapping,0); // nah this is fine
  page = find_page_inode(i, 0); //ok so this actually works
  if(!page){
    printk(KERN_INFO "Page failed\n");
    return 0;
  }
  map = kmap(page);
  printk(KERN_INFO "%c", *((char*)map));
  *((char*)map) = 'A';
  kunmap(map);
  //SetPageDirty(page); //this fixes it thus proving that it is not a bug in the writeback daemon

  if(PageDirty(page)){
    printk(KERN_INFO "Its Dirty!\n");
  }else{
    printk(KERN_INFO "Its clean!\n");
  }
  if(PagePrivate(page)){
    printk(KERN_INFO "It has page buffers");

  }else{
    printk(KERN_INFO "It does not have page buffers");
  }
  printk(KERN_INFO "%lu\n", page -> flags);

  ClearPageReferenced(page); //make sure to clear page references

  printk(KERN_INFO "%lu\n", page -> flags); //theres some random high bit set
  return 0;
}

void cleanup_module(void)
{
	printk("Module cleanup\n");
}
MODULE_LICENSE("GPL");
