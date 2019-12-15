#include <asm/ldt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void cat(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (!f) {
		perror(filename);
		exit(EXIT_FAILURE);
	}

	char line[81];
	while (fgets(line, sizeof(line), f) && line[0]) {
		line[strlen(line) - 1] = '\0';
		puts(line);
	}

	fclose(f);
}

int main(int argc, char *argv[])
{
	struct user_desc desc = {
		.entry_number	= 1,
		.base_addr	= 0xdead,
		.limit		= 0xbeef,
		.seg_32bit	= 0,
		.useable	= 0
	};

	if (syscall(SYS_modify_ldt, 1, &desc, sizeof(desc)) < 0) {
		perror("modify_ldt");
		exit(EXIT_FAILURE);
	}

	cat("/sys/kernel/debug/x86/gdt/gdtr");
	cat("/sys/kernel/debug/x86/gdt/gdt");
	cat("/sys/kernel/debug/x86/gdt/ldtr");
	cat("/sys/kernel/debug/x86/gdt/ldt");
}
