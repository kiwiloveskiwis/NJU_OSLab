#include <inc/x86.h>
#define PORT 0x3f8   /* COM1 */
#define ANSI_COLOR_BLUE    "\x1b[34m"

void init_serial() {
   outb(PORT + 1, 0x00);
   outb(PORT + 3, 0x80);
   outb(PORT + 0, 0x03);
   outb(PORT + 1, 0x00);
   outb(PORT + 3, 0x03);
   outb(PORT + 2, 0xC7);
   outb(PORT + 4, 0x0B);
}

int is_serial_idle() {
   return inb(PORT + 5) & 0x20;
}
int serial_printc(char c) {
	while(!is_serial_idle())
		;
	outb(PORT, c);
    return 1; // 1 byte
}
