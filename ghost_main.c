
#include "taskutil.h"
#include "regset.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"

#ifndef TEST_FILE_PATH
#define TEST_FILE_PATH "/home/mineo333/ghostfops/victim/test_file"
#endif

#define STRINGIFY(A) #A
#define TOSTRING(A) STRINGIFY(A) //I don't why the fuck this works. If I had to take a guess, it is because when you pass in A in TOSTRING, you pass in the value of A which is then stringifyed.

int init_module(void)
{
 	void* map;
  int ret;

	struct inode* i = get_file_path(TOSTRING(TEST_FILE_PATH));
  if(!i){
    printk(KERN_INFO "Invalid path\n");
    return 0;
  }
	struct page* page = find_page_inode(i,0);
	//struct page* page = find_get_page(i->i_mapping, 0);//find_page_inode(i, 0); - this is bad code. Use find_get_page instead
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
  printk("%lu\n", get_pte(page).pte);
  ret = force_writeback(i); //ensure writeback. Without PG_dirty this should do nothing

  return 0;
}

void cleanup_module(void)
{
	printk("Module cleanup\n");
}
MODULE_LICENSE("GPL");
