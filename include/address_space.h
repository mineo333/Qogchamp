#include "depend.h"

#ifndef ADDRESS_SPACE_FUNCS
#define ADDRESS_SPACE_FUNCS
struct page* find_page_file(struct file* f, int bs_off);

struct page* find_page_inode(struct inode* i, unsigned long bs_off);

void edit_page(char* buf, int off, struct page* page);

int force_writeback(struct inode* inode);

struct page* remove_page(struct inode* i, unsigned long bs_off);

void insert_page(struct inode* i, unsigned long bs_off, struct page* page);

void replace_page(struct page* old, struct page* new);

void write_string_page_cache(struct inode* i, unsigned long bs_off, const char* buf, int len, struct page** new_page, struct page** old_page);

void write_string_page_cache_iter(struct inode* i, unsigned long bs_off, char* buf, int len); //don't use this. please.

void unmap_page(struct inode* i, unsigned long bs_off);

void fuckery(struct inode* i, unsigned long bs_off);


#endif