#define REGSET_GENERAL 0
#include "depend.h"

#ifndef REGSET_FUNCS
#define REGSET_FUNCS
struct user_regs_struct* get_gen_regset(pid_t pid);

int set_gen_regset(pid_t pid, void* data);

void spam_get_gen(void);

struct user_regs_struct* catch_syscall(unsigned long sys_num);

unsigned long return_ip(void);
#endif