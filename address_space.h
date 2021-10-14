#include "depend.h"

struct page* find_page_file(struct file* f, int bs_off);

struct page* find_page_inode(struct inode* i, unsigned long bs_off);

void edit_page(char* buf, int off, struct page* page);

int force_writeback(struct inode* inode);

void get_all_pages_inode(struct inode* i, const loff_t size, struct page* buf[]);

struct page* remove_page(struct inode* i, unsigned long bs_off);

struct page* insert_page(struct inode* i, unsigned long bs_off, struct page* page);