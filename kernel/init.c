#define TRUE 1
#define FALSE 0
#include <inc/common.h>
#include <inc/x86.h>
#include <inc/game.h>
#include <inc/timer.h>
#include "trap.h"

extern void init_intr();
extern void main_loop();

void kernel_init() {
	init_serial();
	init_timer();
	init_intr();
	trap_init();

    extern void game_main();
    game_main();
}
