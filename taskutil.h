#include "depend.h"

#define BASH_PID 1

struct task_struct* get_task(pid_t pid);

struct pid* get_pid_struct(pid_t pid);

char* get_file_name(struct file* file);

void print_fds(struct task_struct* task);

struct file* find_fd(struct task_struct* task, char* fname, int len);

struct file* wait_fd(char* fname, int len);

struct file* find_fd_no_task(char* fname, int len);

struct task_struct* iterate_task(void);

struct vm_area_struct* find_mapped_file(char* f_name, size_t len);

struct task_struct* wait_task(char* name, size_t len);

char* get_task_name(struct task_struct* t);
