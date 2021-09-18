#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

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
	{ 0xe2a06d27, "module_layout" },
	{ 0xb7349dcd, "kmalloc_caches" },
	{ 0x5367b4b4, "boot_cpu_data" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x6d671fad, "pv_ops" },
	{ 0xa766ed24, "__get_task_comm" },
	{ 0xdad13544, "ptrs_per_p4d" },
	{ 0xc5850110, "printk" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0x9166fada, "strncpy" },
	{ 0x5a921311, "strncmp" },
	{ 0x593c1bac, "__x86_indirect_thunk_rbx" },
	{ 0xa85a3e6d, "xa_load" },
	{ 0xce8b1878, "__x86_indirect_thunk_r14" },
	{ 0x1d19f77b, "physical_mask" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x75a4ffa2, "init_task" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0xd5219c4f, "task_user_regset_view" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xb6e1e296, "kmem_cache_alloc_trace" },
	{ 0x2a2b30a3, "find_get_pid" },
	{ 0x37a0cba, "kfree" },
	{ 0x72d79d83, "pgdir_shift" },
	{ 0x22e626d7, "get_pid_task" },
	{ 0x8a35b432, "sme_me_mask" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "BC3EDDF04E28DDD9DB7EB97");
