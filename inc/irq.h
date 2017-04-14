#ifndef __IRQ_H__
#define __IRQ_H__

#include <inc/common.h>

#define HZ 1000

#define DPL_KERNEL              0
#define DPL_USER                3

#define NR_SEGMENTS             3
#define SEG_KERNEL_CODE         1 
#define SEG_KERNEL_DATA         2

void init_idt(void);
void init_intr(void);

void init_timer(void);

void set_timer_intr_handler( void (*ptr)(void) );
void set_keyboard_intr_handler( void (*ptr)(int) );

struct GateDescriptor {
	uint32_t offset_15_0      : 16;
	uint32_t segment          : 16;
	uint32_t pad0             : 8;
	uint32_t type             : 4;
	uint32_t system           : 1;
	uint32_t privilege_level  : 2;
	uint32_t present          : 1;
	uint32_t offset_31_16     : 16;
};


typedef struct TrapFrame {
	uint32_t edi, esi, ebp, old_esp, ebx, edx, ecx, eax;
	int32_t irq;
	uint32_t error_code, eip, cs, eflags;
} TrapFrame;


#endif

