#include "address_space.h"


/*
Finds a the page containing the data at bs_off.
*/

struct page* find_page_file(struct file* f, int bs_off){
  struct address_space* mapping = f->f_mapping;
  struct xarray i_pages = mapping -> i_pages;
  int pg_off = bs_off/4096; //integer division to get offset into pages
  struct page* page_ptr = xa_load(&i_pages, pg_off);
  if(!page_ptr){
    printk(KERN_INFO "It didn't work\n");
  }else{
    printk(KERN_INFO "It worked!\n");
  }

  return page_ptr;
}

/*
The main advantage of this function is that it does not increment refcount

However, this is a high dangerous function as it could casue a race with the page reclaimer

Thus, it is recommended to utilize find_get_page()

*/
struct page* find_page_inode(struct inode* i, int bs_off){
  struct address_space* mapping = i->i_mapping;
  if(!mapping){
    return NULL;
  }
  struct xarray i_pages = mapping -> i_pages;
  int pg_off = bs_off/4096; //integer division to get offset into pages
  struct page* page_ptr = xa_load(&i_pages, pg_off);
  if(!page_ptr){
    printk(KERN_INFO "It didn't work\n");
  }else{
    printk(KERN_INFO "It worked!\n");
  }

  return page_ptr;
}


/*
Make this use buffers instead of using something on the stack cause thats cringe.
sizeof(buf) >= size % PAGE_SIZE
buf is a pointer of pointers - dereferencing (buf + n) will give the nth page in inode i

THIS FUNCTION IS UNTESTED - 09/24

TODO: update xa_load to find_get_page.

*/
void get_all_pages_inode(struct inode* i, const loff_t size, struct page** buf){
  int inc;
  struct address_space* mapping = i->i_mapping;
  struct xarray i_pages = mapping -> i_pages;
  //struct page* ret_arr[size%4096]; //declare array of size size
  for(inc = 0; inc<size%4096; inc++, buf++){
    *buf= xa_load(&i_pages, inc);

  }
}


/*
Force writeback of all pages from the address_space object of inode
*/

int force_writeback(struct inode* inode){
  struct writeback_control wb = {.sync_mode = WB_SYNC_ALL, .nr_to_write=LONG_MAX, .range_start=0, .range_end=LLONG_MAX};
  int ret;
  ret = generic_writepages(inode->i_mapping, &wb);
  printk(KERN_INFO "%d %ld\n", ret, wb.nr_to_write);
  return ret;
}
