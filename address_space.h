#include "depend.h"

struct page* find_page(struct file* f, int bs_off);

void edit_page(char* buf, int off, struct page* page);
