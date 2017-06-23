#include <inc/x86.h>
#include <inc/common.h>
#include <game/game.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <game/sel_words.h>
#include <inc/syscall.h>
#include <kernel/trap.h>
#include <inc/pmap.h>

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

//static inline int test_printk();
//static void test_father();
//static void test_child();

__attribute__((__aligned__(PGSIZE)))
static uint8_t shared_mem[PGSIZE];


int main() { // ﻿80488d9
	printk("Welcome to our game");
	sys_timer(timer_event);
	sys_keyboard(keyboard_event);

    int sem_id = sys_sem_open(0, 2); 	// at most 2 processes are allowed
    int child_num = 6, pid;
	int *shared_p = (int *)shared_mem; // ATTACH P TO A SHARED MEM
	*shared_p = 0;

    for(int i = 0; i < child_num; i ++) {
        if ((pid = sys_fork()) == 0) { // child, pid = 1
            break;
        }
    }
    if(pid != 0) {  // Father
        for(;;);
//        printk("\nParent: All children have exited.\n");
    } else {        // Child
		uint32_t curr_pid = sys_getpid();
		sys_mem_share(curr_pid, 0, (uint32_t)shared_mem);
        sys_sem_wait(sem_id);
        printk("Child(%d) is in critical section.\n", curr_pid);
        sys_sleep(1);
		*shared_p += curr_pid % 3;
		printk("Child(%d) new value of *p=%d.\n", curr_pid, *shared_p);
        sys_sem_post(sem_id);

    }

	while(true) {
		/* do nothing */
	}

    printk("game start! SYS_pid = %d\n", sys_getpid());

    main_loop();
    panic("main_loop returns");
}

//// Unrelated to game itself

//static inline int test_printk() {
//	int count = 0;
//	printk("Printk test begin...\n");
//	printk("the answer should be:\n");
//	printk("#######################################################\n");
//	printk("Hello, welcome to OSlab! I'm the body of the game. ");
//	printk("Bootblock loads me to the memory position of 0x100000, and Makefile also tells me that I'm at the location of 0x100000. ");
//	printk("~!@#$^&*()_+`1234567890-=...... ");
//	printk("Now I will test your printk: ");
//	printk("1 + 1 = 2, 123 * 456 = 56088\n0, -1, -2147483648, -1412505855, -32768, 102030\n0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
//	printk("#######################################################\n");
//	printk("your answer:\n");
//	printk("=======================================================\n");
//	printk("%s %s%scome %co%s", "Hello,", "", "wel", 't', " ");
//	printk("%c%c%c%c%c! ", 'O', 'S', 'l', 'a', 'b');
//	printk("I'm the %s of %s. %s 0x%x, %s 0x%x. ", "body", "the game", "Bootblock loads me to the memory position of",
//		   0x100000, "and Makefile also tells me that I'm at the location of", 0x100000);
//	printk("~!@#$^&*()_+`1234567890-=...... ");
//	printk("Now I will test your printk: ");
//	printk("%d + %d = %d, %d * %d = %d\n", 1, 1, 1 + 1, 123, 456, 123 * 456);
//	printk("%d, %d, %d, %d, %d, %d\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
//	printk("%x, %x, %x, %x, %x, %x\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
//	printk("=======================================================\n");
//	count += printk("Test end!!! Good luck!!!\n");
//	return count;
//}