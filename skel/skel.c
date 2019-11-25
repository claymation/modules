#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>

static int __init skel_init(void)
{
	return 0;
}
module_init(skel_init);

static void __exit skel_exit(void)
{
}
module_exit(skel_exit);

MODULE_AUTHOR("Clay McClure <clay@daemons.net>");
MODULE_LICENSE("GPL");
