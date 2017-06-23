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
//	my_assert((uint32_t) str != 0);
	int len = 0;
	while (*str ++) len ++;
	return len;
}

void strcpy(char *d, const char *s) {
//	my_assert((uint32_t) d != 0);
//	my_assert((uint32_t) s != 0);
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

int strncmp(const char *s1, const char *s2, size_t n) {
	for (int x; n; --n) if ((x = *s1 - *s2) || !(*s1 && *s2)) return x;
	return 0;
}

char *strncpy(char *dest, const char *src, size_t n) {
	while (n--) if ((*dest++ = *src)) ++src;
	return dest;
}