#include <inc/common.h>
#include <inc/string.h>
#include <inc/stdarg.h>
#include <inc/syscall.h>

// extern void sys_crash() __attribute__((noreturn));

char *itoa(int a) {
	static char buf[30];
	char *p = buf + sizeof(buf) - 1;
	do {
		*--p = '0' + a % 10;
	} while (a /= 10);
	return p;
}

void memcpy(void *dest, const void *src, size_t size) {
	asm volatile ("cld; rep movsb" : : "c"(size), "S"(src), "D"(dest));
}

void memset(void *dest, int data, size_t size) {
	asm volatile ("cld; rep stosb" : : "c"(size), "a"(data), "D"(dest));
}

size_t strlen(const char *str) {
	int len = 0;
	while (*str ++) len ++;
	return len;
}

void strcpy(char *d, const char *s) {
	memcpy(d, s, strlen(s) + 1);
}

void _warn(const char* file, int line, const char* format, ...) {
	printk("Warning (%s:%d): ", file, line);
	va_list args;
	va_start(args, format);
	printk(format, args);
	va_end(args);
	printk("\n");
}

void _panic(const char* file, int line, const char* format, ...) {
	__asm __volatile("cld");
	printk("Fatal (%s:%d): ", file, line);
	va_list args;
	va_start(args, format);
	printk(format, args);
	va_end(args);
	printk("\n");
	sys_crash();
}