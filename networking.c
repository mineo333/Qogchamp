#include "networking.h"

void enumerate_pci(void){ //pci driver name is the way
    struct pci_dev* d;
    for_each_pci_dev(d){ //macro in pci.h
        if(!d -> driver)
            continue;
        printk(KERN_INFO "%s\n",d->driver->name);
    }
}