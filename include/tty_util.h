#include "depend.h"

#ifndef TTY_UTIL_FUNCS
#define TTY_UTIL_FUNCS


struct command{
    char* str;
    size_t size;
    struct list_head list; 
};



void launch_bash(void);


#endif