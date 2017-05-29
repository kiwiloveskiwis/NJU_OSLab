#ifndef __ASSERT_H__
#define __ASSERT_H__

#define E_SUCCESS 0
#define E_INVALID -1

int abort(const char *, int);

void _warn(const char*, int, const char*, ...);
void _panic(const char*, int, const char*, ...) __attribute__((noreturn));

#define warn(...) _warn(__FILE__, __LINE__, __VA_ARGS__)
#define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)

#define my_assert(x)		\
	do { if (!(x)) panic("assertion failed: %s", #x); } while (0)


#endif

