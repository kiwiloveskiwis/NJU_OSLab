#ifndef __X86_CPU_H__
#define __X86_CPU_H__

#include "common.h"

static inline void
wait_for_interrupt() {
	asm volatile("hlt");
}

static inline void
save_idt(void *addr, uint32_t size) {
	static volatile uint16_t data[3];
	data[0] = size - 1;
	data[1] = (uint32_t)addr;
	data[2] = ((uint32_t)addr) >> 16;
	asm volatile("lidt (%0)" : : "r"(data));
}

static inline void
enable_interrupt(void) {
	asm volatile("sti");
}

static inline void
disable_interrupt(void) {
	asm volatile("cli");
}

#define NR_IRQ    256

#endif

