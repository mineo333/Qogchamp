

obj-m += ghost.o  #this line is for all the object files that will be built into the module
ghost-objs := memutil.o regset.o taskutil.o ghost_main.o address_space.o
BUILD_DIR := $(PWD)/bin
VICTIM_FILE := $(PWD)/victim/test_file #change this to change file
KFLAGS := -DVICTIM_FILE=$(VICTIM_FILE)
#
all:
	make clean
	$(info $(KFLAGS))
	 make CFLAGS_MODULE=$(KFLAGS) -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) src=$(PWD) modules
	mv $(PWD)/bin/ghost.ko $(PWD)
clean:
	rm -f $(PWD)/ghost.ko/home/mineo333/ghostfops/victim/test_file
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) src=$(PWD) clean
