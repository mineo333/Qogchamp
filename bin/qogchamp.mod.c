#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x9f593a9b, "module_layout" },
	{ 0x25931311, "kmalloc_caches" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x3cd279f8, "replace_page_cache_page" },
	{ 0x3d098d97, "call_usermodehelper_exec" },
	{ 0x37110088, "remove_wait_queue" },
	{ 0xd304cfde, "boot_cpu_data" },
	{ 0xf8712322, "arp_tbl" },
	{ 0xe468df37, "__lock_page" },
	{ 0xf34e8ace, "alloc_pages" },
	{ 0xa843805a, "get_unused_fd_flags" },
	{ 0xcab5244f, "add_to_page_cache_lru" },
	{ 0x2d5f69b3, "rcu_read_unlock_strict" },
	{ 0xc512626a, "__supported_pte_mask" },
	{ 0x4629334c, "__preempt_count" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x48980845, "pv_ops" },
	{ 0x745a981, "xa_erase" },
	{ 0xac9c62e, "__neigh_create" },
	{ 0x15ba50a6, "jiffies" },
	{ 0x5cd1952d, "__dynamic_netdev_dbg" },
	{ 0xf4d1f7b1, "skb_trim" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xaad8c7d6, "default_wake_function" },
	{ 0x99a05979, "__get_task_comm" },
	{ 0x5ab533c4, "kern_path" },
	{ 0xdad13544, "ptrs_per_p4d" },
	{ 0xfb578fc5, "memset" },
	{ 0xc7bbdde8, "dma_sync_single_for_cpu" },
	{ 0x703c188c, "l3mdev_master_ifindex_rcu" },
	{ 0xd35cce70, "_raw_spin_unlock_irqrestore" },
	{ 0xf0ca6d41, "current_task" },
	{ 0xc5850110, "printk" },
	{ 0x3c3fce39, "__local_bh_enable_ip" },
	{ 0xa0ac0233, "inet_select_addr" },
	{ 0xa4f54bd8, "generic_writepages" },
	{ 0x9166fada, "strncpy" },
	{ 0x5a921311, "strncmp" },
	{ 0xa85a3e6d, "xa_load" },
	{ 0x1d19f77b, "physical_mask" },
	{ 0xb0f8d0d0, "unlock_page" },
	{ 0xab148a7a, "ipv6_stub" },
	{ 0x30bd8afd, "build_skb" },
	{ 0x800473f, "__cond_resched" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0xa13d54c6, "init_task" },
	{ 0x80d2c282, "napi_gro_receive" },
	{ 0x50237dd5, "ip_route_output_key_hash" },
	{ 0x1f775d3b, "__alloc_skb" },
	{ 0x73e37125, "make_kuid" },
	{ 0x296695f, "refcount_warn_saturate" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x1000e51, "schedule" },
	{ 0x7f48b507, "kfree_skb" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0x2e07f6b, "eth_type_trans" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xed5f6e, "kmem_cache_alloc_trace" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0x34db050b, "_raw_spin_lock_irqsave" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0x7aee5d9a, "call_usermodehelper_setup" },
	{ 0xbb04db62, "find_get_pid" },
	{ 0x4afb2238, "add_wait_queue" },
	{ 0x37a0cba, "kfree" },
	{ 0x72d79d83, "pgdir_shift" },
	{ 0x69acdf38, "memcpy" },
	{ 0x9abe7583, "fd_install" },
	{ 0x3aaaba43, "dma_unmap_page_attrs" },
	{ 0x6def5e22, "pci_get_device" },
	{ 0x714ea94b, "get_pid_task" },
	{ 0xb23ef646, "consume_skb" },
	{ 0xc7b4df8, "__napi_alloc_skb" },
	{ 0xfe571fca, "dev_queue_xmit" },
	{ 0x45d5ad0d, "skb_put" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x24e4a13c, "anon_inode_getfile" },
	{ 0x8a35b432, "sme_me_mask" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "7A058B6974D8EE0C54D4288");
