#include "regset.h"
#include "taskutil.h"
MODULE_LICENSE("GPL");
/*
For some reason, every time get_gen_regset is called it always seems to be within the libc shared object. Why? I have no idea. Someone please answer this question. I'd really like to know the answer.


Regset interception is really unpredictable and I really do not recomemnd it. But, it exists if you want. Also I don't know if anything in this library is memory safe.

*/

/*

Gets general-purpose registers (ax, bx, cx, dx, etc.) for a task with PID pid. This should probably be paired with scheulder
abuse to get the most value.

*/
struct user_regs_struct* get_gen_regset(pid_t pid){ //This function gets the general regset for a given pid
	int ret = 0;
	struct user_regs_struct* data = kmalloc(sizeof(struct user_regs_struct), GFP_KERNEL);
  	struct membuf buf = {.p = (void*)data, .left = sizeof(struct user_regs_struct)};


	const struct user_regset_view* view = task_user_regset_view(get_task(pid)); //allows a view of all the regsets
	const struct user_regset* regset = &view -> regsets[REGSET_GENERAL]; // REGSET_GENERAL is from enum x86_regset in ptrace.c in the 5.4 kernel, this gets all the general regs
	ret = regset -> regset_get(get_task(pid), sizeof(struct user_regs_struct), buf);
	if(ret != 0){
		return NULL;
	}
	return data;
}

/*

Sets the general registers for pid. This has never been tested nor will it ever be tested

*/

int set_gen_regset(pid_t pid, void* data){ //data is the regset
	int ret;
	const struct user_regset_view* view = task_user_regset_view(get_task(pid)); //allows a view of all the regsets
	const struct user_regset* regset = &view -> regsets[REGSET_GENERAL]; // REGSET_GENERAL is from enum x86_regset in ptrace.c in the 5.4 kernel, this gets all the general regs
	ret = regset -> set(get_task(pid), regset, 0, sizeof(struct user_regs_struct), data, NULL);
	return ret;

}

void spam_get_gen(void){ //pretty much a utility function to spam get_general_regset
	int i;
	for(i = 0; i<100; i++){
		struct user_regs_struct* data = get_gen_regset(BASH_PID);
		printk("Syscall number: %lu\n", data -> orig_ax);
    kfree(data);
	}
}

/*

Highly unpredictable function. This is just here for posterity. 

This function goes into a busy loop - :( - until it gets an orig_ax of sys_num. This requires quite a bit of luck for it to work out

*/

struct user_regs_struct* catch_syscall(unsigned long sys_num){ //we do a little spamming
	while(1){
		struct user_regs_struct* data = get_gen_regset(BASH_PID);
		if(data->orig_ax ==sys_num){
			return data;
			break;
		}
	}
}
