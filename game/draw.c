#include <game/game.h>
#include <inc/string.h>
#include <inc/video.h>

int count_down;
extern char wrong_guess[26];

const char end_tip[] = "PRESS_R_TO_RESTART";
const char endword[] = "Congratulations! You win.";
const char tip[] = "Guess_the_word:";

void draw_miss() {
	const char *miss = itoa(get_miss());
	draw_string("MISS:", SCR_HEIGHT - 18, 0, 12);
	draw_string(miss, SCR_HEIGHT - 18, strlen("MISS:") * 8, 12);
	int skip = 0;
	for(int i = 0; i < get_miss(); i++) {
		draw_character(wrong_guess[i] + 'A' - 'a', SCR_HEIGHT - 9, skip, 12);
		skip += 12;
	}
}

void redraw_screen() {
	prepare_buffer(); 
	char hit[30];
#define c_width(i) (strlen(i) / 2 * 8)
	draw_string(tip, SCR_HEIGHT / 4 - 8, SCR_WIDTH / 2 - c_width(tip), 48);
	draw_string((char *)show_str, SCR_HEIGHT / 4 + 8, SCR_WIDTH / 2 - c_width(show_str), 48);
	/*
	draw_string(itoa(last_key_code()), SCR_HEIGHT - 8, 0, 48);
	*/

	strcpy(hit, itoa(get_hit()));

	my_assert((uint32_t) hit != 0);
	draw_string(hit, 0, SCR_WIDTH - strlen(hit) * 8, 10);
	draw_miss();

	draw_string("TIME:", 0, 0, 14);
	draw_string(itoa(count_down), 0, strlen("TIME:") * 8, 14);

	display_buffer();  // 0x80488d9
}


void draw_fail(char *endword) {

	prepare_buffer(); 
	draw_string(endword, SCR_HEIGHT / 2 - 12, SCR_WIDTH / 2 - c_width(endword), 14);
	draw_string(end_tip, SCR_HEIGHT / 2, SCR_WIDTH / 2 - c_width(end_tip), 14);
	display_buffer(); 
}

void draw_win() {
	prepare_buffer();
	draw_string(endword, SCR_HEIGHT / 2 - 12, SCR_WIDTH / 2 - c_width(endword), 14);
	draw_string(end_tip, SCR_HEIGHT / 2, SCR_WIDTH / 2 - c_width(end_tip), 14);
	display_buffer(); 
}
#undef c_width

