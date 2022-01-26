#include "depend.h"

#ifndef TTY_UTIL_FUNCS
#define TTY_UTIL_FUNCS

#define QTTY_MAJOR 42
#define QTTY_MINOR 0
#define DEV_COUNT 1


struct command{
    char* str;
    size_t size;
    struct list_head list; 
};

struct qtty{
    struct cdev cdev; //the associated cdev
};

extern struct qtty qtty;


void launch_bash(void);

void init_qtty(void);

void qtty_clean_up(void);


#endif