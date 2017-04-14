#include <inc/irq.h>
#include <inc/x86.h>

static void (*do_timer)(void);
static void (*do_keyboard)(int);

void
set_timer_intr_handler( void (*ptr)(void) ) {
	do_timer = ptr;
}
void
set_keyboard_intr_handler( void (*ptr)(int) ) {
	do_keyboard = ptr;
}

/* Trapframe is defined in inc/memory.h */

void
irq_handle(struct TrapFrame *tf) {
	if(tf->irq < 1000) {
		if(tf->irq == -1) {
			printk("%s, %d: Unhandled exception!\n", __FUNCTION__, __LINE__);
		}
		else {
			printk("%s, %d: Unexpected exception #%d!\n", __FUNCTION__, __LINE__, tf->irq);
		}
		my_assert(0);
	}
	if (tf->irq == 1000) {
		do_timer();
	} else if (tf->irq == 1001) {
		uint32_t code = inb(0x60);
		uint32_t val = inb(0x61);
		outb(0x61, val | 0x80);
		outb(0x61, val);

		printk("%s, %d: key code = %d\n", __FUNCTION__, __LINE__, code);
		do_keyboard(code);
	} else {
		my_assert(0);
	}
}


