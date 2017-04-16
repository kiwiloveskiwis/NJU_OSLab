#include <inc/x86.h>
#include <inc/common.h>
#include <inc/game.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <game/sel_words.h>
#include <inc/syscall.h>
#include <kernel/trap.h>

#define FPS 30

extern size_t strlen(const char *);
extern void strcpy(char *d, const char *s);
extern char * sel_words[NR_WORDS];

void hangman_init();
bool check_win();

volatile int tick = 0;
char * ans, * show_str, wrong_guess[26];
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
			wait_for_interrupt();
			disable_interrupt();

			if (now == tick) {
				enable_interrupt();
				continue;
			}
			my_assert(now < tick);
			target = tick; 
			enable_interrupt();

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

		printk("break the loop");
		if(count_down <= 0) draw_fail("TIME IS UP! ");
		else if(check_win()) draw_win();
		else if(get_miss() >= MISS_END) draw_fail("The man was hung to death. CRUEL YOU");

		enable_interrupt();
		while(TRUE) {
			if(query_key('r' - 'a')) break;
		}
	}
}

void hangman_init() {
	reset_miss();
	reset_hit();
	now = tick = 0;
	ans = sel_words[rand() % NR_WORDS];
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

void game_main() {
    do_timer = (timer_event);
    do_keyboard = (keyboard_event);

    printk("game start!\n");
    enable_interrupt();

    main_loop();
    my_assert(0);
}
