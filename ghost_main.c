#include "taskutil.h"
#include "regset.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"

#ifndef VICTIM_FILE
#define VICTIM_FILE "/lib/x86_64-linux-gnu/libc-2.31.so"
#endif

#define VICTIM_FILE_OVERRIDE "/lib/x86_64-linux-gnu/libc-2.31.so"

#define STRINGIFY(A) #A
#define TOSTRING(A) STRINGIFY(A) //I don't why the fuck this works. If I had to take a guess, it is because when you pass in A in TOSTRING, you pass in the value of A which is then stringifyed.

char* troll = "trolled";


char trolling_opcodes[] = { 0x48, 0x8D, 0x35, 0x5C, 0xBE, 0x08, 0x00, 0x90, 0x90, 0x90, 0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00 }; //append with a shit ton of nops
int init_module(void)
{
 	void* map;
  void* new_page_kernel;
  void* new_page_user;
  char* ptr;
  struct page* page;
  int count;
  char* path = VICTIM_FILE_OVERRIDE;//TOSTRING(VICTIM_FILE);//TOSTRING(VICTIM_FILE);///VICTIM_FILE_OVERRIDE;//TOSTRING(VICTIM_FILE);//TOSTRING(VICTIM_FILE);
//  printk("path after stringify %d\n", linux); //stupid fucking macro nobody like you you cuck fuck you
	struct inode* i = get_file_path(path);//
  if(!i){
    printk(KERN_INFO "Invalid path\n");
    return 0;
  }
  printk(KERN_INFO "Path Succeeded\n");

  //new_page_kernel = alloc_page(GFP_KERNEL);
  //new_page_user = alloc_page(GFP_USER)

  if(i -> i_mapping == NULL){
    printk(KERN_INFO "This page does not have an address_space object - it is likely not part of the page cache\n");
    return 0;
  }
  printk(KERN_INFO "This page has an address_space object - it is likely not part of the page cache\n");
  /*
  page = find_page_inode(i, 0);
  if(!page){
    printk(KERN_INFO "Page failed\n");
    return 0;
  }
  map = kmap(page);

  *((char*)map) = 'A';
  kunmap(page);

  */

  /*
  THE CODE BELOW IS HIGHLY DESTRUCTIVE. DO NOT, UNDER ANY CIRCUMSTANCE, RUN IT AGAINST THE REAL LIBC

  */


  /*page = find_page_inode(i, 0x0019d030); //ok so this actually works
  printk("%lu\n", page->flags);
  if(!page){
    printk(KERN_INFO "Page failed\n");
    return 0;
  }
  map = kmap(page);
  ptr = (char*)map + pg_off(0x0019d030); //.rodata location
  for(count = 0; count<7; count++,ptr++){ //doin a little trolling
    *ptr = *(troll+count);
  }
  kunmap(map);
  ClearPageReferenced(page); //the page dump won't dump referenced pages so clear the bit
  page = find_page_inode(i, 0x001111d4);

  if(!page){
    printk(KERN_INFO "Page failed\n");
    return 0;
  }
  map = kmap(page);
  ptr = (char*)map + pg_off(0x001111d4);
  for(count = 0; count < 17; count++, ptr++){  //it iterativly copies the opcodes. That is problematic with synchronous access.
    *ptr = trolling_opcodes[count];
  }
  kunmap(map);
  printk("%lu\n", page->flags);
  */
  page = remove_page(i, 0x001111d4);

  if(!page){
    return 0; //don't continue into flag checking if we haven't initalized a page
  }

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
