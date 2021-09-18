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
    printk("It didn't work\n");
  }else{
    printk("It worked!\n");
  }

  return page_ptr;
}
struct page* find_page_inode(struct inode* i, int bs_off){
  struct address_space* mapping = i->i_mapping;
  struct xarray i_pages = mapping -> i_pages;
  int pg_off = bs_off/4096; //integer division to get offset into pages
  struct page* page_ptr = xa_load(&i_pages, pg_off);
  if(!page_ptr){
    printk("It didn't work\n");
  }else{
    printk("It worked!\n");
  }

  return page_ptr;
}

void edit_page(char* buf, int off, struct page* page){

}
