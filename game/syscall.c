#include <inc/syscall.h>
#include <inc/types.h>
#include <inc/trap.h>
#include <inc/assert.h>

static inline int32_t do_syscall0(int syscallno) {
    int32_t ret;
    asm volatile("int %1\n"
    : "=a" (ret)
    : "i" (T_SYSCALL),
    "a" (syscallno)
    : "cc", "memory");
    return ret;
}
static inline int32_t do_syscall1(int syscallno, uint32_t a1) {
    int32_t ret;
    asm volatile("int %1\n"
    : "=a" (ret)
    : "i" (T_SYSCALL),
    "a" (syscallno),
    "d" (a1)
    : "cc", "memory");
    return ret;
}
static inline int32_t do_syscall2(int syscallno, uint32_t a1, uint32_t a2) {
    int32_t ret;
    asm volatile("int %1\n"
    : "=a" (ret)
    : "i" (T_SYSCALL),
    "a" (syscallno),
    "d" (a1),
    "c" (a2)
    : "cc", "memory");
    return ret;
}
static inline int32_t do_syscall3(int syscallno, uint32_t a1, uint32_t a2, uint32_t a3) {
    int32_t ret;
    asm volatile("int %1\n"
    : "=a" (ret)
    : "i" (T_SYSCALL),
    "a" (syscallno),
    "d" (a1),
    "c" (a2),
    "b" (a3)
    : "cc", "memory");
    return ret;
}
static inline int32_t do_syscall4(int syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4) {
    int32_t ret;
    asm volatile("int %1\n"
    : "=a" (ret)
    : "i" (T_SYSCALL),
    "a" (syscallno),
    "d" (a1),
    "c" (a2),
    "b" (a3),
    "D" (a4)
    : "cc", "memory");
    return ret;
}
static inline int32_t do_syscall5(int syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5) {
    int32_t ret;
    asm volatile("int %1\n"
    : "=a" (ret)
    : "i" (T_SYSCALL),
    "a" (syscallno),
    "d" (a1),
    "c" (a2),
    "b" (a3),
    "D" (a4),
    "S" (a5)
    : "cc", "memory");
    return ret;
}


void sys_vprintk(const char *ctl, va_list arg) {
    my_assert(do_syscall2(SYS_vprintk, (uint32_t) ctl, (uint32_t) arg) == 0);
}

void sys_timer(void (*handler)(void)) {
    my_assert(do_syscall1(SYS_timer, (uint32_t) handler) == 0);
}

int sys_getpid() {
    return do_syscall0(SYS_getpid);
}

void sys_keyboard(void (*handler)(int)) {
    my_assert(do_syscall1(SYS_keyboard, (uint32_t) handler) == 0);
}

void sys_display(uint8_t *buffer) {
    my_assert(do_syscall1(SYS_display, (uint32_t) buffer) == 0);
}

__attribute__((noreturn)) void sys_crash() {
    for (;;) do_syscall0(SYS_crash);
}

__attribute__((noreturn)) void sys_exit() {
    for (;;) do_syscall0(SYS_exit);
}


void sys_wait_intr(){
    do_syscall0(SYS_wait_intr);
}
void sys_sleep(uint32_t time) {
    my_assert(do_syscall1(SYS_sleep, time) == 0);
}
