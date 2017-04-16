#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"
#define PROB_HIDE 70
#define NR_KEY 26
#define COUNTDOWN 50
#define MISS_END 10

enum letter_state {UNKNOWN, IN_WORD, OUT_WORD};

char * ans, * show_str, wrong_guess[26];
int letter_known[NR_KEY];
int count_down;

void timer_event(void);
void keyboard_event(int scan_code);

void press_key(int scan_code);
void release_key(int ch);
bool query_key(int ch);
int last_key_code(void);

void main_loop(void);

void update_keypress(void);

int get_hit(void);
void reset_hit(void);
int get_miss(void);
void reset_miss();
int get_fps(void);
void set_fps(int fps);

void redraw_screen(void);
void draw_fail(char *endword);
void draw_win(void);

int rand(void);
void srand(int seed);

#endif

