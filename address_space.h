#include "depend.h"

struct page* find_page_file(struct file* f, int bs_off);

struct page* find_page_inode(struct inode* i, int bs_off);

void edit_page(char* buf, int off, struct page* page);

int force_writeback(struct inode* inode, int num_pages);
