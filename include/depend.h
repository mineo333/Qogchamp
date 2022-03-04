/*
This file contains all the system dependcies because I'm lazy and the project looks better this way. Every system header file should include this file.
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
#include <linux/gfp.h>
#include <asm-generic/errno-base.h>
#include <linux/swap.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/page_ref.h>
#include <linux/interrupt.h>
#include <linux/skbuff.h>
#include <linux/dma-mapping.h>
#include <linux/prefetch.h>
#include <asm-generic/barrier.h>
#include <linux/byteorder/generic.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/memcontrol.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/udp.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/tcp.h>
#include <linux/cdev.h>
#include <linux/rwlock.h>
#include <asm-generic/atomic-long.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/file.h>
#include <uapi/asm-generic/fcntl.h>
#include <linux/cred.h>
#include <linux/rcupdate.h>
#include <linux/anon_inodes.h>