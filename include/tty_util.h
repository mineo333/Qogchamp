#include "depend.h"

#ifndef TTY_UTIL_FUNCS
#define TTY_UTIL_FUNCS

#define QTTY_MAJOR 42
#define QTTY_MINOR 0


void launch_bash(void);

struct qtty{
    struct cdev cdev;
};


#endif