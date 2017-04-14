#include <inc/common.h>
#include <inc/x86.h>
#include <inc/string.h>
#include <inc/video.h>

static void
append(char **p, const char *str) {
	while (*str) {
		*((*p) ++) = *str ++;
	}
}

/* Show file name and the line of failed assertion */
#define BLUE_SCREEN_TEXT "Assertion failed: "
static void
blue_screen(const char *file, int line) {
	static char buf[256] = BLUE_SCREEN_TEXT;
	char *p = buf + sizeof(BLUE_SCREEN_TEXT) - 1;

	append(&p, file);
	append(&p, ":");
	append(&p, itoa(line));

	// abort(__FILE__, __LINE__);

	prepare_buffer();
	memset(vmem, 1, SCR_SIZE);
	draw_string(buf, 0, 0, 15);
	display_buffer();
}

int
abort(const char *fname, int line) {
	/* disable intr to avoid other errors 
		Display error info and stay awaiting */
	disable_interrupt();
	blue_screen(fname, line);
	while (TRUE) {
		wait_for_interrupt();
	}
}

