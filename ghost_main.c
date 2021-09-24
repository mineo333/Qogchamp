
#include "taskutil.h"
#include "regset.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"

int init_module(void)
{
 	void* map;
  int ret;
	struct inode* i = get_file_path("/home/mineo333/Documents/GhostFops/victim/victim");
	struct page* page = find_page_inode(i, 0);
  if(!page){
    printk("Page failed\n");
    return 0;
  }
  map = kmap(page);
  printk("%c", *((char*)map));
  kunmap(map);
  if(PageDirty(page)){ //lmao fixed this. I'm stupid
    printk("Its Dirty!\n");
  }else{
    printk("Its clean!\n");
  }
  printk("%lu\n", pte_val(page));
  ret = force_writeback(i,5);

	return 0;
}

void cleanup_module(void)
{
	printk("Module cleanup\n");
	//go ghost
}
MODULE_LICENSE("GPL");
