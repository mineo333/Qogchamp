
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
#define BASH_PID 6191
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
int init_module(void)
{
	struct vm_area_struct *mmap = get_task(BASH_PID) -> mm -> mmap; //this is a LINKED LIST containing all the mapped memory of the program.
	void* dest = kmalloc(EMBEDDED_NAME_MAX, GFP_KERNEL);
	void* src;
	int* intbuf;
	int copied;
	int i;
	while(1){ //get openat
		struct user_regs_struct* data = get_gen_regset(BASH_PID);
		if(data -> orig_ax == 257 || data -> orig_ax == 2){ //the reason openat is used when calling open is because of a race condition
			src = (void*)(data -> si);
			copied = __copy_from_user(dest, src, EMBEDDED_NAME_MAX);
			intbuf = (int*)dest;
			for(i = 0; i<copied; i++){
				printk("Buffer %d: %d", i, intbuf[i]);
			}
			//printk("Hopefully the path: %s\n", charbuf);
			printk("syscall_number: %lu\n", data -> orig_ax);
			printk("1st arg: %lu\n", data -> di);
			printk("2nd arg: %lu\n", data -> si);
			printk("3rd arg: %lu\n", data -> dx);
			printk("4th arg: %lu\n", data -> r10);
			printk("5th arg: %lu\n", data -> r8);
			break;
		}

	}
	kfree(dest);
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
