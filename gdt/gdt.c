#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <asm/desc.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/seq_file.h>

/**
 * Show the contents of the global descriptor table (GDT) register.
 */
static int gdtr_show(struct seq_file *s, void *data)
{
	struct desc_ptr gdtr;

	native_store_gdt(&gdtr);

	seq_printf(s, "0x%lx+0x%x\n", gdtr.address, gdtr.size);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(gdtr);

/**
 * Show the contents of the local descriptor table (LDT) register.
 */
static int ldtr_show(struct seq_file *s, void *data)
{
	unsigned short ldtr;

	store_ldt(ldtr);

	seq_printf(s, "0x%x\n", ldtr);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(ldtr);

static void dump_table(struct seq_file *file,
		       struct desc_struct *desc,
		       int n_entries)
{
	for (; n_entries--; desc++)
		seq_printf(file, "0x%08lx+0x%04lx %x %x %c%c%c%c%c%c\n",
			((unsigned long)desc->base2 << 24) +
			((unsigned long)desc->base1 << 16) +
			(unsigned long)desc->base0,

			((unsigned long)desc->limit1 << 16) +
			(unsigned long)desc->limit0,

			desc->type,
			desc->dpl,

			desc->s		? 's' : '-',
			desc->p		? 'p' : '-',
			desc->avl	? 'a' : '-',
			desc->l		? 'l' : '-',
			desc->d		? 'd' : '-',
			desc->g		? 'g' : '-');
}

static int gdt_show(struct seq_file *file, void *data)
{
	struct desc_ptr gdtr;

	native_store_gdt(&gdtr);
	dump_table(file,
		   (struct desc_struct *)gdtr.address,
		   (gdtr.size + 1) / sizeof(struct desc_struct));

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(gdt);

static int ldt_show(struct seq_file *file, void *data)
{
	struct ldttss_desc *desc;
	struct desc_ptr gdtr;
	unsigned short ldtr;
	unsigned long base;
	unsigned long size;

	native_store_gdt(&gdtr);
	store_ldt(ldtr);

	desc = (struct ldttss_desc *)((char *)gdtr.address + ldtr);
	base = ((unsigned long)desc->base3 << 32) +
	       ((unsigned long)desc->base2 << 24) +
	       ((unsigned long)desc->base1 << 16) +
		(unsigned long)desc->base0;
	size = ((unsigned long)desc->limit1 << 16) +
		(unsigned long)desc->limit0;

	seq_printf(file, "base: %lx\n", base);
	seq_printf(file, "size: %lx\n", size);

	dump_table(file,
		   (struct desc_struct *)base,
		   (size + 1) / sizeof(struct desc_struct));

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(ldt);

static struct dentry	*x86_dir;
static struct dentry	*gdt_dir;

static int __init skel_init(void)
{
	struct dentry	*file;

	x86_dir = debugfs_lookup("x86", NULL);
	if (IS_ERR_OR_NULL(x86_dir))
		return -ENODEV;

	gdt_dir = debugfs_create_dir("gdt", x86_dir);
	if (IS_ERR_OR_NULL(gdt_dir))
		goto err_put;

	file = debugfs_create_file("gdtr", 0444, gdt_dir, NULL, &gdtr_fops);
	if (IS_ERR_OR_NULL(file))
		goto err_remove;

	file = debugfs_create_file("gdt", 0444, gdt_dir, NULL, &gdt_fops);
	if (IS_ERR_OR_NULL(file))
		goto err_remove;

	file = debugfs_create_file("ldtr", 0444, gdt_dir, NULL, &ldtr_fops);
	if (IS_ERR_OR_NULL(file))
		goto err_remove;

	file = debugfs_create_file("ldt", 0444, gdt_dir, NULL, &ldt_fops);
	if (IS_ERR_OR_NULL(file))
		goto err_remove;

	return 0;

err_remove:
	debugfs_remove_recursive(gdt_dir);
err_put:
	dput(x86_dir);

	return -ENOMEM;
}
module_init(skel_init);

static void __exit skel_exit(void)
{
	debugfs_remove_recursive(gdt_dir);
	dput(x86_dir);
}
module_exit(skel_exit);

MODULE_AUTHOR("Clay McClure <clay@daemons.net>");
MODULE_LICENSE("GPL");
