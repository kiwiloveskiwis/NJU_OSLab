#include <inc/game.h>
#include <inc/common.h>
#include <inc/string.h>
#include <inc/video.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <inc/game.h>

static int hit = 0, miss = 0;

char wrong_guess[26];

void reset_miss() {
	miss = 0;
}
void reset_hit() {
	hit = 0;
}

int get_hit(void) {
	return hit;
}

int get_miss(void) {
	return miss;
}


void
update_keypress(void) {
	disable_interrupt();

	for(int i = 0; i < NR_KEY; i++) {
		if(!query_key(i)) continue;
		release_key(i);
		if(letter_known[i] != UNKNOWN) {
			// printk("Letter already trid.\n"); 
			break;
		}
		bool found; // unknown key already pressed

		for(int j = 0; j < strlen(ans); j++) {
			if(ans[j] - 'a' != i) continue; 
			found = TRUE; // found matching key
			letter_known[i] = IN_WORD;
		} 
		if(!found) {
			wrong_guess[miss++] = 'a' + i;
			letter_known[i] = OUT_WORD;
			// printk("Wrong letter!\n");
		}
	}
	
	for(int i = 0; i < strlen(ans); i++) {
		if(letter_known[ans[i] - 'a'] == IN_WORD) show_str[i] = ans[i];
	}

	enable_interrupt();
}


