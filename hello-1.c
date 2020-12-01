/*
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/types.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/regset.h>
#include <asm/user.h> //change BASH_PID PLEASE
#include <linux/mm_types.h>
#include <linux/uaccess.h>
#define BASH_PID 5798
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
		void* to = kmalloc(size, GFP_KERNEL);
		__copy_from_user(to,from,size); //copy from a user space buffer
		return to;
}


int init_module(void)
{
	int count = 0;
	struct vm_area_struct *mmap = get_task(BASH_PID) -> mm -> mmap; //this is a LINKED LIST containing all the pages of a program.
	void* from;
	char* to;
	while(1){ //get open or openat
		struct user_regs_struct* data = get_gen_regset(BASH_PID);
		if(data -> orig_ax == 257){ //openat
			from = (void*)(data -> si);
			to = (char*)(copy_user(from, 100));

			printk("path_addr: %lu\n", data -> si);
			printk("hopefully a string: %s\n", to);
			if(count >= 5){
				break;
			}
			count++;
		}
	}
	while(mmap -> vm_next){
		printk("<1> start:%lu, end:%lu", mmap -> vm_start, mmap -> vm_end);
		mmap = mmap -> vm_next; //https://developer.ibm.com/articles/l-kernel-memory-access/ - kernel memory shit
	}
	return 0;
}

void cleanup_module(void)
{
	printk("<1> Goodbye world 1.\n");
}
MODULE_LICENSE("GPL");
