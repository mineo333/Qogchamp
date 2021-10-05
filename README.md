# GhostFops

GhostFops is a kernel post expoitation module that exploits an overlooked portion of the Linux Kernel's page cache design philosphy.


## Purpose
GhostFops is just that - Ghost File Operations. The goal is to provide a library for writing to files discreetly with minimal logging.

There are 2 cases for GhostFops thus far: File Memory Mappings and Inodes

## Design Philsophy Exploited

The Linux Kernel devs within the MM subsystem have a seemingly random and quite redundant system for dirty page writeback. For some reason, the MM designers decided that they wanted to set the dirty bit at their own discretion.

In the Linux kernel, each page is described by 2 descriptors: the `struct page` and `pte_t`. The `struct page` is the main thing that is used and contains lots of information about page caches, LRU, zones, slab allocation, and more. `pte_t` on the other hand is used by the MMU for page management and fits the page descriptor specification as described in Intel Manual Volume 3. For page management, the Linux kernel mainly uses `struct page` and almost always ignore `pte_t`.

For some reason, both of these structures have their own dirty bits. `struct page` has the PG_dirty flag and `pte_t` has its own dirty bit. To add insult to injury, these flags are not linked. Meaning, although the `pte_t` is set dirty, the Linux Kernel has no clue because it is only paying attention to `struct page`. This is the design philosphy that is exploited.

Now, the initial reasoning for this might have been in regards to deferred write, but that doesn't even make any sense because writeback only happens when the flusher thread is awakened.

## Inodes

The first case is trivial. Inodes are the kernel representation of files on the disk (This is different from a file struct. A file struct is the representation of an open file. They are typically the backend of file descriptors).

With inodes, the pages won't flush out of the page cache unless the page is set dirty. This set is usually done by the write syscall. We can bypass the write syscall by writing directly to the page holding the data within the page cache. This is where the philosphy of the mm subsystem comes in. Even though the pte is dirty, the page struct is not dirty. The only thing that matters in the eye of the kernel is the page struct. Thus, we can put data into the page struct and it won't be written back to disk. Thus ghost file operations.

## Memory Mappings

The second case is memory mappings. As you might now, in addition to being opened, files can also be mapped into the linear address space of a program using the mmap syscall. Through these mappings, the mapped files can be modified discreetly.

Because memory mappings are files on the backend, their pages are also held in the page cache.

The way memory mappings are handled in the mm subsystem is interesting in regards to writeback. You might expect the mm subsystem to use the dirty bit of the pte for writeback as it is set by the CPU. But no. For some reason, the mm subsystem has a helper function which walks the entire page table for a memory mapping, finds each dirty bit, and sets PG_dirty in the corresponding page struct.

Thus, if we don't call that helper function, modified file-mapped pages for a file-mapped region will never be written back to disk.

Thus we are presented with a great power.

As you might know, when ELFs are loaded into memory they are loaded as memory mapped files (Look at /proc/[pid]/maps). Typically, writtable sections like .data are marked as copy-on-write meaning that any written pages will be copied into anonymous pages which are not written back to disk. However, static sections like .text and .rodata are not marked as copy-on-write. Thus, data written to them will be written back to disk. Thus, we can semi-permanantly change the execution of programs!

All because of the design philosphy of the Linux mm subsystem...

## Footprint

A common concern might be footprint. To my understand, there is very little footprint that the code actually creates (Obviously inserting the module will create a footprint but this can be mitigated). Because it is not actually doing any writing, no file metadata is updating - something that the write syscall would do.

## Reverting Changes

In order to revert the changes of this module, either restart which will implicitly dump the page caches (Because the page caches are in RAM which is dumped during restart) OR run `revert.sh` as root which will clean the page cache. I would recommend the second option unless you basically dead locked yourself by changing the first letter in /etc/shadow which locks you out of root.
## Usage

If you want to run this for yourself, just clone the repository and run `make VICTIM_FILE=[absolute file path]` where `[absolute file path]` is replaced with the absolute file path of the file you want to target. This will create `ghost.ko` in the directory root directory of the project. Run `sudo insmod ghost.ko` to run the program.

This module has been tested on Ubuntu 20.04 with the 5.11 kernel as well as the 5.14.0 mainline Linux Kernel.
