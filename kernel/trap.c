#include <inc/memlayout.h>
#include <inc/x86.h>
#include <kernel/trap.h>
#include <inc/syscall.h>
#include <inc/pmap.h>
#include <inc/pcb.h>

//
struct Segdesc gdt[6] =
{
        // 0x0 - unused (always faults -- for trapping NULL far pointers)
        SEG_NULL,

        // 0x8 - kernel code segment
        [GD_KT >> 3] = SEG(STA_X | STA_R, 0x0, 0xffffffff, 0),

        // 0x10 - kernel data segment
        [GD_KD >> 3] = SEG(STA_W, 0x0, 0xffffffff, 0),

        // 0x18 - user code segment
        [GD_UT >> 3] = SEG(STA_X | STA_R, 0x0, 0xffffffff, 3),

        // 0x20 - user data segment
        [GD_UD >> 3] = SEG(STA_W, 0x0, 0xffffffff, 3),

        // Per-CPU TSS descriptors (starting from GD_TSS0) are initialized
        // in trap_init_percpu()
        [GD_TSS0 >> 3] = SEG_NULL
};
struct Pseudodesc gdt_pd = { sizeof(gdt) - 1, (uint32_t) gdt };

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
        sizeof(idt) - 1, (uint32_t) idt
};

extern uint32_t trap_handlers[];
struct Taskstate pts;

void (*do_timer)(void), (*do_keyboard)(int);

void trap_init(void) {
    lgdt(&gdt_pd);
    // The kernel never uses GS or FS, so we leave those set to
    // the user data segment.
    asm volatile("movw %%ax,%%gs" :: "a" (GD_UD|3));
    asm volatile("movw %%ax,%%fs" :: "a" (GD_UD|3));
    // The kernel does use ES, DS, and SS.  We'll change between
    // the kernel and user data segments as needed.
    asm volatile("movw %%ax,%%es" :: "a" (GD_KD));
    asm volatile("movw %%ax,%%ds" :: "a" (GD_KD));
    asm volatile("movw %%ax,%%ss" :: "a" (GD_KD));
    // Load the kernel text segment into CS.
    asm volatile("ljmp %0,$1f\n 1:\n" :: "i" (GD_KT));
    // For good measure, clear the local descriptor table (LDT),
    // since we don't use it.
    lldt(0);

    int i = 0;
    for ( ; i < 256 ; i++) {
        SETGATE(idt[i], 0, GD_KT, trap_handlers[i], 0);
    }

    // init break point
    SETGATE(idt[T_BRKPT], 0, GD_KT, trap_handlers[T_BRKPT], 3);
    // init syscall
    SETGATE(idt[T_SYSCALL], 0, GD_KT, trap_handlers[T_SYSCALL], 3);

    pts.ts_esp0 = KSTACKTOP;
    pts.ts_ss0 = GD_KD;

    // Initialize the TSS slot of the gdt.
    gdt[GD_TSS0 >> 3] = SEG16(STS_T32A, (uint32_t) (&pts),
                                      sizeof(struct Taskstate), 0);
    gdt[GD_TSS0 >> 3].sd_s = 0;

    // Load the TSS selector (like other segment selectors, the
    // bottom three bits are special; we leave them 0)
    ltr(GD_TSS0);

    // Load the IDT
    lidt(&idt_pd);
}


void trap(struct Trapframe *tf) {
    // The environment may have set DF and some versions
    // of GCC rely on DF being clear
    asm volatile("cld" ::: "cc");
    // Check that interrupts are disabled.  If this assertion
    // fails, DO NOT be tempted to fix it by inserting a "cli" in
    // the interrupt path.
    my_assert(!(read_eflags() & FL_IF));

    switch (tf->tf_trapno) {
        case T_ILLOP:
            panic("----> Illegal opcode at 0x%x, pid=%d.\n", tf->tf_eip, get_pid());
        case T_GPFLT:
            panic("----> General protection fault at 0x%x, pid=%d.\n", tf->tf_eip, get_pid());
        case T_PGFLT:
            printk("----> Page fault at 0x%x, va=0x%x, pid=%d.\n", tf->tf_eip, rcr2(), get_pid());
            panic("");
        case T_SYSCALL:
            tf->tf_regs.reg_eax = syscall_handler(tf);
            break;
        case IRQ_OFFSET + IRQ_TIMER:
            sys_runned_time++;
            user_pcbs[get_pid()].runned_time++;
            if (do_timer != NULL) do_timer();
            break;
        case IRQ_OFFSET + IRQ_KBD: {
            uint32_t code = inb(0x60), val = inb(0x61);
            outb(0x61, (uint8_t) (val | 0x80));
            outb(0x61, (uint8_t) val);
            if (do_keyboard != NULL) do_keyboard(code);
            break;
        }
        case IRQ_OFFSET + IRQ_IDE: break;   // ignore IDE IRQ
        default:
            panic("----> Unknown interrupt #%d (err=0x%x) at 0x%x.\n", tf->tf_trapno, tf->tf_err, tf->tf_eip);
    }
}
