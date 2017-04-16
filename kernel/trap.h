/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_TRAP_H
#define JOS_KERN_TRAP_H

#include <inc/trap.h>
#include <inc/mmu.h>

/* The kernel's interrupt descriptor table */
extern struct Gatedesc idt[];
extern struct Pseudodesc idt_pd;

void trap_init(void);
void trap(struct Trapframe *tf);
uint32_t syscall_handler(struct Trapframe *);

extern void (*do_timer)(void), (*do_keyboard)(int);

#endif /* JOS_KERN_TRAP_H */
