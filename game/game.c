#include <inc/x86.h>
#include <inc/common.h>
#include <game/game.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <game/sel_words.h>
#include <inc/syscall.h>
#include <kernel/trap.h>

#define FPS 30

extern size_t strlen(const char *);
extern void strcpy(char *d, const char *s);
extern const char * sel_words[NR_WORDS];

void hangman_init();
bool check_win();

volatile int tick = 0;
char ans[30], show_str[30], wrong_guess[26];
int count_down;

void timer_event(void) {
	tick ++;
}

static int real_fps;

void set_fps(int value) {
	real_fps = value;
}

int get_fps() {
	return real_fps;
}

int now = 0, target;
int num_draw = 0;
bool redraw;

void main_loop(void) {

	while (TRUE) {
		hangman_init();
		while (!check_win() && count_down > 0 && get_miss() < MISS_END) {
            // TODO: possible race conditions here
			if (now == tick) {
                sys_wait_intr();
				continue;
			}
			my_assert(now < tick);
			target = tick; 
			// TODO: enable_interrupt();

			redraw = FALSE;

			update_keypress();

			while (now < target) { 
				if(now % (HZ / 2) == 0) count_down --;
				if (now % (HZ / FPS) == 0) {
					redraw = TRUE;
				}
				if (now % (HZ / 2) == 0) {
					int now_fps = num_draw * 2 + 1;
					if (now_fps > FPS) now_fps = FPS;
					set_fps(now_fps);
					num_draw = 0;
				}
				now ++;
			}

			if (redraw) { 
				num_draw ++;
				redraw_screen();
			}
		}  // Time is up, or user wins

		// printk("breaked the loop\n");
		if(count_down <= 0) draw_fail("TIME IS UP! ");
		else if(check_win()) draw_win();
		else if(get_miss() >= MISS_END) draw_fail("The man was hung to death. CRUEL YOU");

		// TODO: enable_interrupt();
		while(TRUE) {
			if(query_key('r' - 'a')) break;
		}
	}
}

void hangman_init() {
	reset_miss();
	reset_hit();
	now = tick = 0;
	strcpy(ans, sel_words[rand() % NR_WORDS]);
	count_down = COUNTDOWN;
	memset(letter_known, FALSE, sizeof(letter_known));
	for(int i = 0; i < NR_KEY; i++) release_key(i);

	for(int i = 0; i < strlen(ans); i++) {
		if(rand() % 100 > PROB_HIDE) { // show this letter
			int idx = ans[i] - 'a';
			letter_known[idx] = IN_WORD;
		}
	} 
	strcpy(show_str, ans);

	for(int i = 0; i < strlen(ans); i++) {
		if(letter_known[ans[i] - 'a'] == UNKNOWN) show_str[i] = '_';
	}
}

bool check_win() {
	for(int i = 0; i < strlen(ans); i++) {
		if(letter_known[ans[i] - 'a'] != IN_WORD) return FALSE;
	} 
	return TRUE;
}

int main() {
	sys_timer(timer_event);
	sys_keyboard(keyboard_event);
    // sys_sleep(0x1000);
    int new_pid = sys_fork();
    if (new_pid != 0) {
		printk("main: new_pid = %d\n", new_pid);
		sys_exit();
	} // main process sleep

// ﻿0x08048a91?
    printk("game start! SYS_pid = %d\n", sys_getpid());

    main_loop();
    panic("main_loop returns");
}
