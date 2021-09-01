#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <stdio.h>
#define PID 9845 //please change this

int main(){

  struct user_regs_struct regs;
  while(1){
    ptrace(PTRACE_ATTACH, PID, NULL, NULL);
    ptrace(PTRACE_GETREGS, PID, NULL, &regs);
  //  if(regs.orig_rax == 2 || regs.orig_rax == 257){
      printf(" %llu %llu\n", regs.orig_rax,regs.rax);
      //break;
    //}
    ptrace(PTRACE_DETACH, PID, NULL, NULL);

  }


  return 0;
}
