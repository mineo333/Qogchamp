cmd_/home/mineo333/SyscallInterception/modules.order := {   echo /home/mineo333/SyscallInterception/hello-1.ko; :; } | awk '!x[$$0]++' - > /home/mineo333/SyscallInterception/modules.order
