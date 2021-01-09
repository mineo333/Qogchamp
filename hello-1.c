#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/types.h>
#include <linux/pid.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/regset.h>
#include <asm/user.h> //change BASH_PID PLEASE
#include <linux/mm_types.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <asm/pgtable_types.h>
#define BASH_PID 6641
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
#define PATH_MAX 4096
#define EMBEDDED_NAME_MAX	(PATH_MAX - OFFSETOF(struct filename, iname))
#define REGSET_GENERAL 0
//MAKE SURE TO FREE ALL MALLOCS
struct pid* get_pid_struct(pid_t pid){
	return find_get_pid(pid);
}
struct task_struct* get_task(pid_t pid){
	return get_pid_task(find_get_pid(pid), PIDTYPE_PID);
}
struct user_regs_struct* get_gen_regset(pid_t pid){
	int ret = 0;
	struct user_regs_struct* data;
	const struct user_regset_view* view = task_user_regset_view(get_task(pid)); //allows a view of all the regsets
	const struct user_regset* regset = &view -> regsets[REGSET_GENERAL]; // REGSET_GENERAL is from enum x86_regset in ptrace.c in the 5.4 kernel, this gets all the general regs
	data = kmalloc(sizeof(struct user_regs_struct), GFP_KERNEL);
	ret = regset -> get(get_task(pid), regset, 0, sizeof(struct user_regs_struct), data, NULL);
	if(ret != 0){
		return NULL;
	}
	return data;
}

int set_gen_regset(pid_t pid, void* data){ //data is the regset
	int ret;
	const struct user_regset_view* view = task_user_regset_view(get_task(pid)); //allows a view of all the regsets
	const struct user_regset* regset = &view -> regsets[REGSET_GENERAL]; // REGSET_GENERAL is from enum x86_regset in ptrace.c in the 5.4 kernel, this gets all the general regs
	ret = regset -> set(get_task(pid), regset, 0, sizeof(struct user_regs_struct), data, NULL);
	return ret;

}

void* copy_user(const void *from, unsigned long size){
		int ret;
		void* to = kmalloc(size, GFP_KERNEL);
		ret  = __copy_from_user(to,from,size); //copy from a user space buffer
		printk("ret: %d",ret);
		return to;
}

char* cpy_string(const char* src){
	char* dest = kmalloc(50,GFP_KERNEL);
	int len;
	len = strncpy_from_user(dest,src,50);
	printk("String length copied: %d",len);
	return dest;
}
void spam_get_gen(void){
	int i;
	for(i = 0; i<100; i++){
		struct user_regs_struct* data = get_gen_regset(BASH_PID);
		printk("syscall_number: %lu\n", data -> orig_ax);
	}
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
	return page;

}
/*
So there's a problem which is basically
*/
unsigned long page_offset(unsigned long laddr){
	return laddr & 0b111111111111;
	//return 4;
}
void read_string(char* paddr, char* dest, unsigned long laddr){
	strncpy(dest,paddr+page_offset((unsigned long)laddr),PATH_MAX);
}
char* return_true_addr(char* paddr, unsigned long laddr){
	return paddr+page_offset((unsigned long)laddr);
}

struct user_regs_struct* catch_syscall(unsigned long sys_num){
	//unsigned long caught_num;
	while(1){
		struct user_regs_struct* data = get_gen_regset(BASH_PID);
		if(data->orig_ax ==sys_num){
			return data;
			break;
		}
	}
}
int init_module(void)
{
	int i;
	struct mm_struct* mm = get_task(BASH_PID) -> mm;
	//struct vm_area_struct *mmap = mm -> mmap; //this is a LINKED LIST containing all the mapped memory of the program.
	unsigned long laddr;
	struct page* page;
	char* true_addr;
	char* paddr;
	char* string = kmalloc(PATH_MAX,GFP_KERNEL);
	struct user_regs_struct* data = catch_syscall(257);
	laddr = (unsigned long)(data -> si);
	page = page_walk_safe(laddr, mm);
	paddr = (char*)kmap(page); //paddr is grossly misnamed its actually the mapped addr.
	read_string(paddr,string,laddr);
	printk("string1:%s",string);

	true_addr = return_true_addr(paddr, laddr);
	*(true_addr)= 'Q';

	for(i = 0; i<100; i++){
		printk("%c",*(true_addr + i*sizeof(char)));

	}
	/*
	for(i = 0; i<200; i++){
		printk("%p:%d:%c",paddr + i*sizeof(char),(int)*(paddr + i*sizeof(char)),*(paddr + i*sizeof(char)));

	}*/

	/*kunmap(page);
	data = catch_syscall(257);
	laddr = (unsigned long)(data -> si);
	page = page_walk_safe(laddr, mm);
	paddr = (char*)kmap(page);
	read_string(paddr,string,laddr);
	printk("string 2:%s",string);*/
	kunmap(page);
	kfree(string);



	/*
	while(mmap -> vm_next){
		printk("<1> start:%lu, end:%lu", mmap -> vm_start, mmap -> vm_end);
		mmap = mmap -> vm_next; //https://developer.ibm.com/articles/l-kernel-memory-access/ - kernel memory shit
	}*/
	return 0;
}

void cleanup_module(void)
{
	printk("<1> Goodbye world 1.\n");
}
MODULE_LICENSE("GPL");



	//read_string(paddr);
