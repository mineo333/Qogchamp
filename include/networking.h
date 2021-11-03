#include "depend.h"
#ifndef NETWORK_FUNCS
#define NETWORK_FUNCS

void enumerate_pci(void);

struct pci_dev* find_pci(const char* name, int size);

const char* get_pci_name(struct pci_dev* pd);

const char* get_dev_name(struct pci_dev* pd);

struct net_device* get_net_dev(struct pci_dev* pd);

struct e1000_adapter* get_e1000_adapter(struct net_device* net_dev);

#endif
