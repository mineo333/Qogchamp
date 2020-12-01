/*
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/fs.h>
#include <linux/uaccess.h>
int init_module(void)
{
	char* file_path = "/home/mineo333/Documents/module/file";
	//struct kstat *stat = vmalloc(sizeof(struct kstat));
	//loff_t size;
	char* buf;
	char* read_buf;
	struct file *filp;
	mm_segment_t oldfs = get_fs();
//	printk("<1> Hello World 1.\n");
	set_fs(KERNEL_DS);
	filp = filp_open(file_path, O_RDONLY, 0);
//	vfs_stat(file_path, stat);
	set_fs(oldfs);
	//printk("<1> %llu",stat->size);
	//size = stat -> size;
	buf = vmalloc(1000*sizeof(char));
	read_buf = vmalloc(sizeof(char));
	kernel_read(filp, buf, 100, &(filp -> f_pos));
	printk("<1> %llu",filp -> f_pos);
	printk("<1> %s",buf);
	vfree(read_buf);
	vfree(buf);
	return 0;
}

void cleanup_module(void)
{
	printk("<1> Goodbye world 1.\n");
}
MODULE_LICENSE("GPL");
