#include "taskutil.h"
#include "memutil.h"
MODULE_LICENSE("GPL");

/*

Right now, a lot of these functions are running on infinite loops which will very easily hang the system. This needs to be fixed

*/



/*
Gets the pid_struct of a particular task with pid pid. Helper function for get_task

*/

struct pid* get_pid_struct(pid_t pid){
	return find_get_pid(pid);
}

/*
Gets the task_struct for a task with pid pid

*/

struct task_struct* get_task(pid_t pid){
	return get_pid_task(find_get_pid(pid), PIDTYPE_PID);
}

/*
Iterates over all tasks runnning on the system and prints their PID and name.
*/

struct task_struct* iterate_task(void){
	struct task_struct *task_list;
	for_each_process(task_list){
		char* name = get_task_name(task_list);
		printk(KERN_INFO "pid: %d name: %s\n",task_list -> pid, name);
		kfree(name);
	}
	return NULL;
}

/*
Iterates over all tasks and finds an mmap'd file with name f_name
*/

struct vm_area_struct* find_mapped_file(char* f_name, size_t len){ //search through all processes for an mmapped file.
	struct task_struct *task_list;
	struct vm_area_struct* ret;
	for_each_process(task_list){

		if(task_list == NULL){
			continue;
		}
		ret = search_for_mapped_file(task_list, f_name, len);
		if(ret != NULL){
			return ret;
		}
	}
	return NULL;
}

/*
Gets the name of a file
*/

char* get_file_name(struct file* file){
	return file->f_path.dentry->d_iname;
}

/*
Print all file descriptiors in a task
*/

void print_fds(struct task_struct* task){
	struct files_struct* files = task->files;
	struct fdtable*  fdt;
	struct file** fd;
	if(!files){
		return; //no file struct
	}
	fdt = files->fdt;
	if(!fdt){
		return;
	}
	fd = fdt -> fd;
	while(*fd){ //while the file double array is actually pointing to something, iterate
		printk(KERN_INFO "%s\n",get_file_name(*fd));
		fd++;
	}


}




/*
Find an fd with name fname in the task task
*/


struct file* find_fd(struct task_struct* task, char* fname, int len){
	struct files_struct* files = task->files;
	struct fdtable*  fdt;
	struct file** fd;
	if(!files){
		return NULL; //no file struct
	}
	fdt = files->fdt;
	if(!fdt){
		return NULL;
	}
	fd = fdt -> fd;

	//error checking is complete. Begin the file trace.


	while(*fd){
		if(strncmp(get_file_name(*fd),fname, len) == 0){
			return *fd;
		}
		fd++;
	}
	return NULL;

}


/*
Iterates through every process on the system and looks for a task with name name
*/

struct task_struct* search_task(char* name, size_t len){
	struct task_struct* task_list;
	for_each_process(task_list){
		char* task_name;
		task_name = get_task_name(task_list);
		if(strncmp(task_name, name, len) == 0){
			return task_list;
		}
		kfree(task_name); //free because of the buffer decalred in get_task_name
	}
	return NULL;
}

/*
Wait for a task with name to be run and return the task_struct*. Wait task can also be used to get a task by name if it is already running.

DON'T FUCKING USE THIS FUNCTION. IT IS AWFUL
*/

struct task_struct* wait_task(char* name, size_t len){
	struct task_struct *task_ret;
	do{ //fuck it busy loop. I tried implementing a wait queue but idk how to wake it up properly. Someone dm if they know tho.
		task_ret = search_task(name, len);
	}while(task_ret == NULL);

	printk(KERN_INFO "Found %s\n", name);
	return task_ret;
}

char* get_task_name(struct task_struct* t){
	char* buf = kmalloc(TASK_COMM_LEN, GFP_KERNEL); //yep. this is stupid
	__get_task_comm(buf, TASK_COMM_LEN, t);
	return buf;
}

/*
Returns an inode with a given path

*/

struct inode* get_file_path(const char* path_name){
	struct path path;
	kern_path(path_name, LOOKUP_FOLLOW, &path);
	return path.dentry->d_inode;
}
