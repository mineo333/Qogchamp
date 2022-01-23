#include "tty_util.h"



//this qtty will interact with the e1000_hook



struct qtty qtty;

ssize_t qtty_write(struct file* f, const char __user * buf, size_t size, loff_t* off){
    printk(KERN_INFO "WRITING\n");
    char* kbuf = (char*)kzalloc(size+1, GFP_KERNEL); //make sure the string actually terminates so add 1 
    if(!access_ok(buf, size)){
        return -EINVAL;
    }

    if(copy_from_user(kbuf, buf, size)){ //copy_from_user returns 0
        return -EFAULT; //invalid buffer
    }
    
    printk(KERN_INFO "%s\n", kbuf);
    
    return size;



    
}

int qtty_open(struct inode* i, struct file* f){
    printk(KERN_INFO "TTY opened");
    return 0;
}

const struct file_operations qtty_fops = {
    .open = qtty_open,
    .write = qtty_write,
    .owner = THIS_MODULE
};



void init_qtty(void){ //init dev
    int err;
    err = register_chrdev_region(MKDEV(QTTY_MAJOR, QTTY_MINOR), DEV_COUNT , "qogchamp");//reverse major and minor numbers for use.
    
    if(err){
        printk(KERN_INFO "Failed to reserve major and minor numbers");
        return;
    }
    
    
    cdev_init(&qtty.cdev, &qtty_fops);
    err = cdev_add(&qtty.cdev, MKDEV(QTTY_MAJOR, QTTY_MINOR), 1);
    qtty.cdev.owner = THIS_MODULE;
    printk(KERN_INFO "error: %d\n", err);
    if(err){
        qtty.cdev.owner = NULL;
        printk(KERN_INFO "cdev could not be added\n");

    }

    
}

void qtty_clean_up(void){
    
   
    unregister_chrdev_region(MKDEV(QTTY_MAJOR, QTTY_MINOR), DEV_COUNT);
    
    if(!qtty.cdev.owner){
        printk(KERN_INFO "qtty not initialized\n");
        return;
    }


    cdev_del(&qtty.cdev);


}








struct task_struct* umh_tsk = NULL;

DECLARE_WAIT_QUEUE_HEAD(qogchamp_wait); //wait queue for qogchamp. This is used for waiting for umh_tsk to be populated

int init_func(struct subprocess_info* info, struct cred* new){
  //printk(KERN_INFO "old task ptr: 0x%lx, new task ptr: 0x%lx", (unsigned long)info->data, (unsigned long)current);
  umh_tsk = current; //get task_struct
  get_task_struct(umh_tsk); //increase reference counter so the task doesn't disappear from under us
  wake_up(&qogchamp_wait); 
  return 0;

}

void launch_bash(void){
  struct subprocess_info* sub_info;
  int status;
  char* argv[] = {"hello world!\xA", NULL}; //for some reason these arrays need to be null terminated. 
  DECLARE_WAITQUEUE(wait, current);
  add_wait_queue(&qogchamp_wait, &wait); //add it here so we don't hit a race condition

  char* envp[] = {
    "HOME=/",
    "TERM=linux",
    "PATH=/sbin:/bin:/usr/sbin:/usr/bin",
    NULL
  };
  sub_info = call_usermodehelper_setup("/home/mineo333/Qogchamp/testing/write_string/write_string", argv, envp, GFP_KERNEL, init_func, NULL,  (void*)current);
  call_usermodehelper_exec(sub_info, UMH_WAIT_EXEC);
  //it is possible that we get preempted here. However, because we added it to the wait_queue, it shouldn't matter despite umh_exec executing asynchronously
  while(1){ //wait for task to start
    set_current_state(TASK_UNINTERRUPTIBLE);
    if(umh_tsk != NULL)
      break;
    schedule();
  }
  __set_current_state(TASK_RUNNING);
  remove_wait_queue(&qogchamp_wait, &wait);
  printk("PID: %d\n", umh_tsk->pid);
  put_task_struct(umh_tsk);
}