#include "depend.h"
#ifndef NETWORK_FUNCS
#define NETWORK_FUNCS


struct eth_frame{
	//802.3
	unsigned char dest[6];
	unsigned char src[6];
	unsigned char type[2];
	//ipv4
	unsigned char version;
	unsigned char dscp;
	unsigned char length[2];
	unsigned char ident;
	unsigned char flags:3;
    unsigned short frag_off:13;
    unsigned char ttl;
    unsigned char proto;
    unsigned char checksum[2];
    //unfinished
};

void enumerate_pci(void);

struct pci_dev* find_pci(const char* name, int size);

const char* get_pci_name(struct pci_dev* pd);

const char* get_dev_name(struct pci_dev* pd);

struct net_device* get_net_dev(struct pci_dev* pd);


#endif
