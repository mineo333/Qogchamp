#define PATH_MAX 4096 //one page
#include "depend.h"

void read_string(char* paddr, char* dest, unsigned long laddr);

struct page* page_walk_safe(unsigned long laddr, struct mm_struct* mm);

unsigned long pg_off(unsigned long laddr);

char* return_true_addr(char* paddr, unsigned long laddr);

void print_memory_regions(struct task_struct* task);

struct vm_area_struct* search_for_mapped_file(struct task_struct* task, const char* str, size_t len);

pte_t get_pte(struct page* page);
