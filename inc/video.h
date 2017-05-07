#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "types.h"
#include "assert.h"

#define SCR_WIDTH  320
#define SCR_HEIGHT 200
#define SCR_SIZE ((SCR_WIDTH) * (SCR_HEIGHT))
#define VMEM_ADDR  ((uint8_t*) 0xA0000)

extern uint8_t vbuf[SCR_SIZE];

static inline void
draw_pixel(int x, int y, int color) {
	my_assert(x >= 0 && y >= 0 && x < SCR_HEIGHT && y < SCR_WIDTH);
	vbuf[(x << 8) + (x << 6) + y] = (uint8_t) color;
}

void prepare_buffer(void);
void display_buffer(void);

void draw_string(const char*, int, int, int);

void draw_character(char ch, int x, int y, int color);
#endif

