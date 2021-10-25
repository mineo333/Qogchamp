#include "memutil.h"
#include "taskutil.h"
MODULE_LICENSE("GPL");

void read_string(char* kmaddr, char* dest, unsigned long laddr){ //reads string from kmapped page
	strncpy(dest, return_true_addr(kmaddr, laddr),PATH_MAX);
}

struct page* page_walk_safe(unsigned long laddr, struct mm_struct* mm){
	pgd_t* pgd;
	p4d_t* p4d;
	pud_t* pud;
	pmd_t* pmd;
	pte_t* pte;
	struct page* page; //5 level page walk - the wikipedia artcile has a nice diagram
	pgd = pgd_offset(mm, laddr); //get page global directory from cr3 and offset
	if(pgd_none(*pgd) || pgd_bad(*pgd)){
		return NULL;
	}
	p4d = p4d_offset(pgd, laddr); //get 4th level page dir
	if(p4d_none(*p4d) || p4d_bad(*p4d)){
		return NULL;
	}
	pud = pud_offset(p4d, laddr); //get page upper directory
	if(pud_none(*pud) || pud_bad(*pud)){
		return NULL;
	}
	pmd = pmd_offset(pud, laddr); //get page middle directory
	if(pmd_none(*pmd) || pmd_bad(*pmd)){
		return NULL;
	}
	pte = pte_offset_map(pmd, laddr); //get page table entry from pmd
	if(pte == NULL){
		return NULL;
	}
	page = pte_page(*pte); //get associated page struct
	if(page == NULL){
		return NULL;
	}
	pte_unmap(pte);
	return page;

}

pte_t get_pte(struct page* page){
	pgprot_t pgp = {.pgprot = 0xFFF};
	unsigned long pfn = page_to_pfn(page);
	pte_t pte = pfn_pte(pfn, pgp);
	return pte;
}

unsigned long pg_off(unsigned long laddr){ //pretty simply. gets the last 12 bits of a linear address which happens to be the offset into the page frame
	return laddr & 0xfff;
}


char* return_true_addr(char* kmaddr, unsigned long laddr){ //takes a kmapped pointer kmaddr and uses the offset found in the laddr to get the location of the data at laddr in physical memor
	return kmaddr+pg_off(laddr);
}

void print_memory_regions(struct task_struct* task){
	struct vm_area_struct* vma;// = task -> mm -> mmap;
	struct mm_struct* mm;
	if(task->mm){
		mm = task ->mm;
	}else{
		return;
	}

	if(mm -> mmap){
		vma = mm -> mmap;
	}else{
		return;
	}
	while(vma -> vm_next){
		if(vma -> vm_file){//it is mapped to a file
			printk(KERN_INFO "start:%lu, end:%lu, perms: %lu, mapping status: mapped, file name: %s", vma -> vm_start, vma -> vm_end, vma -> vm_flags & VM_EXEC, get_file_name(vma->vm_file));
		} else {
			printk(KERN_INFO "start:%lu, end:%lu, perms: %lu, mapping status: not mapped", vma -> vm_start, vma -> vm_end, vma -> vm_flags & VM_EXEC);
		}

		vma = vma -> vm_next; //https://developer.ibm.com/articles/l-kernel-memory-access/ - kernel memory shit
	}
}

struct vm_area_struct* search_for_mapped_file(struct task_struct* task, const char* str, size_t len){
	struct vm_area_struct* vma;// = task -> mm -> mmap;
	struct mm_struct* mm;
	if(task->mm){
		mm = task ->mm;
	}else{
		return NULL;
	}

	if(mm -> mmap){
		vma = mm -> mmap;
	}else{
		return NULL;
	}

	//error checking complete
	while(vma -> vm_next){
		if(vma->vm_file){
			if(!strncmp(get_file_name(vma -> vm_file), str, len)){ //check if this is it.
				return vma;
			}
		}
		vma = vma -> vm_next; //iterate through linked list
	}
	return NULL;
}
