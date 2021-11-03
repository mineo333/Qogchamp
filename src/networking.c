#include "networking.h"
#include "e1000.h"
#include "e1000_hw.h"
#include "e1000_osdep.h"
/*
Eventually a good portion of these functions will likely be moved to their own pci file
*/


/*
Random notes:
* pci_name() is useless and does not give any useful information whatsoever. Use the driver names
* In the case of the e1000, the driver data is filled with the net_dev struct

*/



struct e1000_adapter* get_e1000_adapter(struct net_device* net_dev){
    return (struct e1000_adapter*) netdev_priv(net_dev);
}

struct pci_dev* find_pci(const char* d_name, int size){ 
    struct pci_dev* d; 
    printk(KERN_INFO "Starting find_pci with %s\n", d_name); //for some reason this function will die if this line is not here
    for_each_pci_dev(d){ //macro in pci.h
        if(!d -> driver || !(d -> driver -> name))
            continue;
        //printk(KERN_INFO "Found device: %s\n", d->driver->name);
        if(!strncmp(d->driver->name, d_name, size))
            return d;
        
        
    }
    return NULL;
}
struct net_device* get_net_dev(struct pci_dev* pd){
    return pci_get_drvdata(pd);
}
void enumerate_pci(void){
    struct pci_dev* d;
    for_each_pci_dev(d){ //macro in pci.h
        if(!(d -> driver) || !(d -> driver -> name))
            continue;
        printk(KERN_INFO "%s\n", d->driver->name); 
    }
}
const char* get_pci_name(struct pci_dev* pd){
    if(!pd -> driver){
        return NULL;
    }
    return pd->driver->name;
}

const char* get_dev_name(struct pci_dev* pd){
    return pci_name(pd);
}
