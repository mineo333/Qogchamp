# GhostFops

GhostFops is a kernel post expoitation module that exploits the mm subsystem's design philosphy in regards to kernel page cache management.


## Purpose
GhostFops is just that - Ghost File Operations. The goal is to provide a library for writing to files discreetly with minimal logging. 

Design Philosphy exploited: For some reason, the devs over at the mm subsystem wanted to set the dirty flag on the page struct at their own discretion and essentially ignore the pte dirty flag on memory mappings

There are 2 cases for GhostFops thus far: File Memory Mappings and Inodes

## Inodes

The first case is trivial. Inodes are the kernel representation of files on the disk (This is different from a file struct. A file struct is the representation of an open file. They are typically the backend of file descriptors). 

With inodes, the pages won't flush out of the page cache unless the page is set dirty. This set is usually done by the write syscall. We can bypass the write syscall by writing directly to the page holding the data within the page cache. This is where the philosphy of the mm subsystem comes in. Even though the pte is dirty, the page struct is not dirty. The only thing that matters in the eye of the kernel is the page struct. Thus, we can put data into the page struct and it won't be written back to disk. Thus ghost file operations.

## Memory Mappings

The second case is memory mappings. As you might now, in addition to being opened, files can also be mapped into the linear address space of a program using the mmap syscall. Through these mappings, the mapped files can be modified diretly. 

Because memory mappings are files on the backened, their pages are also held in the page cache.

The way memory mappings are handled in the mm subsystem is interesting in regards to writeback. You might expect the mm subsystem to use the dirty bit of the pte for writeback as it is set by the CPU. But no. For some reason, the mm subsystem has a helper function which walks the entire page table for a memory mapping, finds each dirty bit, and sets PG_dirty in the corresponding page struct.

Thus, if we don't call that helper function, modified file-mapped pages for a file-mapped region will never be written back to disk. 

Thus we are presented with a great power. 

As you might know, when ELFs are loaded into memory they are loaded as memory mapped files (Look at /proc/[pid]/maps). Typically, writtable sections like .data are marked as copy-on-write meaning that any written pages will be copied into anonymous pages which are not written back to disk. However, static sections like .text and .rodata are not marked as copy-on-write. Thus, data written to them will be written back to disk. Thus, we can semi-permanantly change the execution of programs!

All because of the design philosphy of the Linux mm subsystem...

## Footprint

A common concern might be footprint. To my understand, there is very little footprint that the code actually creates (Obviously inserting the module will create a footprint but this can be mitigated). Because it is not actually doing any writing, no file metadata is updating - something that the write syscall would do. 

## Reverting Changes

In order to revert the changes of this module, either restart which will implicitly dump the page caches (Because the page caches are in RAM which is dumped during restart) OR run `echo 3 > /proc/sys/vm/drop_caches` as root which explicitly tells the kernel to drop the page cache.

## Installing

If you want to run this for yourself, just clone the repository and run `make` which will create `ghost.ko` in the directory. The run `sudo insmod ghost.ko` to run the program.

This module has been tested on Ubuntu 20.04 with the 5.11 kernel.
