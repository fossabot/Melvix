// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <proc.h>
#include <str.h>

void bin_load(char *path, struct proc *proc)
{
	char *data = read_file(path);
	u32 stack = (u32)malloc(0x1000) + 0x1000;

	proc->regs.ebp = (u32)stack;
	proc->regs.esp = (u32)stack;
	proc->regs.useresp = (u32)stack;
	proc->regs.eip = (u32)data;
	strcpy(proc->name, path + 1);
}

int elf_verify(struct elf_header *h)
{
	return h->ident[0] == ELF_MAG && (strncmp((char *)&h->ident[1], "ELF", 3) == 0) &&
	       h->ident[4] == ELF_32 && h->ident[5] == ELF_LITTLE && h->ident[6] == ELF_CURRENT &&
	       h->machine == ELF_386 && (h->type == ET_REL || h->type == ET_EXEC);
}

void elf_load(char *path, struct proc *proc)
{
	char *data = read_file(path);
	struct elf_header *h = (struct elf_header *)data;

	assert(elf_verify(h));

	printf("%d", h->type);
	switch (h->type) {
	case ET_EXEC:
		return;
	case ET_REL:
		return;
	}

	loop();
	u32 stack = (u32)malloc(0x1000) + 0x1000;
	proc->regs.ebp = (u32)stack;
	proc->regs.esp = (u32)stack;
	proc->regs.useresp = (u32)stack;
	proc->regs.eip = (u32)h->entry;
	strcpy(proc->name, path + 1);
}
