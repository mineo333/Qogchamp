/*
This file contains all the system dependcies because I'm lazy and the project looks better this way. Every header file should include this file.
*/

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
#include <linux/mm.h>
#include <linux/highmem.h>
#include <asm/pgtable_types.h>
#include <asm/pgtable.h>
#include <linux/page-flags.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/xarray.h>
#include <linux/wait.h>
#include <linux/namei.h>
#include <linux/writeback.h>
#include <asm-generic/memory_model.h>
#include <linux/pagemap.h>
#include <vdso/limits.h>
