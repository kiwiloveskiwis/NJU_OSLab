#include <inc/common.h>
#include <inc/video.h>
#include <inc/string.h>
#include <inc/font.h>
#include <inc/syscall.h>

uint8_t vbuf[SCR_SIZE];

void prepare_buffer() {
	memset(vbuf, 0, SCR_SIZE);
}

void display_buffer() {
	sys_display(vbuf);
}

void
draw_character(char ch, int x, int y, int color) {
	int i, j;
	my_assert((ch & 0x80) == 0);
	char *p = font8x8_basic[(int)ch];
	for (i = 0; i < 8; i ++) 
		for (j = 0; j < 8; j ++) 
			if ((p[i] >> j) & 1)
				draw_pixel(x + i, y + j, color);
}

void 
draw_string(const char *str, int x, int y, int color) {
	while (*str) {
		draw_character(*str ++, x, y, color);
		if (y + 8 >= SCR_WIDTH) {
			x += 8; y = 0;
		} else {
			y += 8;
		}
	}
}
