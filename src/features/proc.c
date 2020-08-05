// MIT License, Copyright (c) 2020 Marvin Borner

#include <interrupts.h>
#include <load.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <timer.h>

u32 pid = 0;
struct proc *root;
struct proc *current;
struct proc *last;

void scheduler(struct regs *regs)
{
	printf("%d", current->pid);
	memcpy(&current->regs, regs, sizeof(struct regs));

	timer_handler();

	if (current->next)
		current = current->next;
	else
		current = root;

	while (current->state == PROC_ASLEEP)
		if (!current->next)
			current = root;
		else
			current = current->next;

	memcpy(regs, &current->regs, sizeof(struct regs));
}

void proc_print()
{
	struct proc *proc = root;

	while (proc) {
		printf("Process %d [%s]\n", proc->pid,
		       proc->state == PROC_RUNNING ? "running" : "sleeping");
		proc = proc->next;
	}
	printf("\n");
}

void proc_attach(struct proc *proc)
{
	if (!last->next) {
		last->next = proc;
	} else {
		struct proc *save = last;
		while (save->next)
			save = save->next;
		save->next = proc;
	}
}

struct proc *proc_make()
{
	struct proc *proc = malloc(sizeof(*proc));
	proc->pid = pid++;
	proc->state = PROC_RUNNING;
	if (current)
		proc_attach(proc);
	last = proc;
	return proc;
}

void proc_init()
{
	__asm__ volatile("cli");
	current = root = proc_make();
	bin_load("/init", root);
	irq_install_handler(0, scheduler);
	proc_print();

	// JUMP!
	void (*entry)();
	*(void **)(&entry) = (u32 *)root->regs.eip;
	__asm__ volatile("movl %%eax, %%ebp" ::"a"(root->regs.ebp));
	__asm__ volatile("movl %%eax, %%esp" ::"a"(root->regs.esp));
	__asm__ volatile("sti");
	entry();
}
