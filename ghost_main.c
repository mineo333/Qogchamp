
#include "taskutil.h"
#include "regset.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"

int init_module(void)
{
 	void* map;
  int ret;
	struct inode* i = get_file_path("/home/mineo333/ghostfops/victim/test_file");
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
