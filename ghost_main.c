
#include "taskutil.h"
#include "regset.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"

int init_module(void)
{
 	void* map;
	//char* buf = kmalloc(1000, GFP_KERNEL);

	struct inode* i = get_file_path("/home/mineo333/Documents/GhostFops/victim/victim");
	struct page* page = find_page_inode(i, 0);
  if(!page){
    printk("Page failed\n");
    return 0;
  }
  map = kmap(page);
  *((char*)map) = 'Q';
  kunmap(map);
  if(page->flags & PG_dirty){
    printk("Its Dirty!");
  }else{
    printk("Its clean!");
  }
  printk("%ld\n", page -> flags);

	return 0;
}

void cleanup_module(void)
{
	printk("Module cleanup\n");
	//go ghost
}
MODULE_LICENSE("GPL");
