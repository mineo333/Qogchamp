#include "address_space.h"
#include "memutil.h"
/*
TBH this is hardly for address_space anymore and is more of a series of wrapper functions for xarray

*/


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
The main advantage of this function is that it does not increment refcount. So, use this at your own risk because a reference can disappear under you.

This is a temporary patch over using find_get_page. find_get_page does something with zoning that prevents dumping the page cache


*/
struct page* find_page_inode(struct inode* i, unsigned long bs_off){
  struct xarray i_pages;
  struct address_space* mapping = i->i_mapping;
  struct page* page_ptr;
  int pg_off;
  if(!mapping){
    return NULL;
  }
  i_pages = mapping -> i_pages;
  pg_off = bs_off/4096; //integer division to get offset into pages
  page_ptr = xa_load(&i_pages, pg_off); 
  if(!page_ptr){
    printk(KERN_INFO "page retrieval at %lx failed\n", bs_off);
  }else{
    printk(KERN_INFO "page retrieval at %lx succeeded\n", bs_off);
  }

  return page_ptr;
}

/*
Removes page from a page cache mapping. This is a forcible version of unmap page and should be used
for pages that cannot be swapped out due to reference counts
*/

struct page* remove_page(struct inode* i, unsigned long bs_off){
  struct page* ret;
  struct address_space* mapping = i->i_mapping;
  struct xarray i_pages;
  int pg_off = bs_off/4096; //integer division to get offset into pages
  if(!mapping){
    return NULL;
  }
  i_pages = mapping -> i_pages; 
  ret = xa_erase(&i_pages, pg_off); //xa_erase handles locking and rcu for us
  if(!ret){
    printk(KERN_INFO "Failed to remove\n");
  }
  return ret;
}


void insert_page(struct inode* i, unsigned long bs_off, struct page* page){ //replace the old page with the new page
  struct address_space* mapping = i->i_mapping;
  int pg_off;
  int error;
  if(!mapping){
    printk(KERN_INFO "Insertion failed\n");
    return;
  }
  pg_off = bs_off/4096; //integer division to get offset into pages
  error = add_to_page_cache_lru(page, mapping, pg_off, mapping_gfp_constraint(mapping, GFP_KERNEL));
  printk("%d\n", error);

}

void replace_page(struct page* old, struct page* new){
  lock_page(old);
  lock_page(new); //replace_page_cache_page requires the pages to be locked.
  replace_page_cache_page(old,new);
  unlock_page(old);
  unlock_page(new);
  lru_cache_add(new);
}


/*
Force writeback of all pages from the address_space object of inode. This will only writeback pages that are marked dirty!
*/
int force_writeback(struct inode* inode){
  struct writeback_control wb = {.sync_mode = WB_SYNC_ALL, .nr_to_write=LONG_MAX, .range_start=0, .range_end=LLONG_MAX};
  int ret;
  ret = generic_writepages(inode->i_mapping, &wb);
  printk(KERN_INFO "%d %ld\n", ret, wb.nr_to_write);
  return ret;
}

void write_string_page_cache(struct inode* i, unsigned long bs_off, char* buf, int len, struct page* new_page, struct page* old_page){
  char* new_page_ptr;
  char* old_page_ptr; //these are kmapped addresses
  char* ptr;
  int count;
  old_page = find_page_inode(i, bs_off);
  if(!old_page){
    printk(KERN_INFO "Invalid page \n");
    return; 
  }
  new_page = alloc_page(GFP_KERNEL);
  new_page_ptr = kmap(new_page);
  old_page_ptr = kmap(old_page);
  memcpy(new_page_ptr, old_page_ptr, 4096); 

  for(count = 0, ptr = new_page_ptr + pg_off(bs_off); count<len; count++, ptr++){
    *ptr = *(buf+count);
  }
  kunmap(new_page);
  kunmap(old_page); 
  SetPageMappedToDisk(new_page); //this might not be necessary
  SetPageUptodate(new_page); //this is very important as the read syscall will overwrite pages that are not set uptodate
  replace_page(old_page, new_page); //do page replacement

}

/*

This function iteratively adds a string to a page in the page cache. This is not safe for code that is actively being read or used.

However, this can be dropped by dropping the page cache because it does not manipulate the reference counter

*/
void write_string_page_cache_iter(struct inode* i, unsigned long bs_off, char* buf, int len){
  struct page* page;
  char* ptr;
  char* map;
  int count;
  page = find_page_inode(i, bs_off);
  if(!page){
    return; //if the page is null we can't do shit
  }
  map = kmap(page);
  if(!map){
    return; //if kmap failed, do nothing, return
  }
  ptr = (char*)map + pg_off(bs_off);
  for(count = 0; count<len; count++,ptr++){
    *ptr = *(buf+count);
  }
  kunmap(page);
  ClearPageReferenced(page); //the page dump won't dump referenced pages so clear the bit

}

void unmap_page(struct inode* i, unsigned long bs_off){//unmap page at bs_off
  struct page* p = find_page_inode(i, bs_off);
  if(!p){
    return;
  }
  ClearPageReferenced(p); //just in case, clear references.
  ClearPageUptodate(p); //on next read the page will be flushed out by the read daemon. 

}
