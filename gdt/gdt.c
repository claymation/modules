#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <asm/desc.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/seq_file.h>

static struct dentry	*file;

static int gdt_show(struct seq_file *s, void *data)
{
	struct desc_struct *gdt;
	struct desc_ptr gdtr;

	native_store_gdt(&gdtr);

	seq_printf(s, "gdtr: 0x%016lx (%d bytes)\n", gdtr.address, gdtr.size);

	gdt = (struct desc_struct *)gdtr.address;
	while ((unsigned long)gdt < gdtr.address + gdtr.size) {
		seq_printf(s, " - 0x%016lx+0x%016lx %6s priv %d %7s %5s %4s %2s %s\n",
			((unsigned long)gdt->base2 << 24) +
			((unsigned long)gdt->base1 << 16) +
			(unsigned long)gdt->base0,

			((unsigned long)gdt->limit1 << 16) +
			(unsigned long)gdt->limit0,

			gdt->s ? (gdt->type & 8 ? "code" : "data") : "system",
			gdt->dpl,
			gdt->p ? "present" : "-",
			gdt->avl ? "avail" : "-",
			gdt->l ? "long" : "-",
			gdt->d ? "32" : "16",
			gdt->g ? "4 KiB" : "1");
		++gdt;
	}

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(gdt);

static int __init skel_init(void)
{
	file = debugfs_create_file("gdt", 0444, NULL, NULL, &gdt_fops);
	if (!file)
		return -ENOMEM;

	return 0;
}
module_init(skel_init);

static void __exit skel_exit(void)
{
	debugfs_remove(file);
}
module_exit(skel_exit);

MODULE_AUTHOR("Clay McClure <clay@daemons.net>");
MODULE_LICENSE("GPL");
