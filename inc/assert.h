#ifndef __ASSERT_H__
#define __ASSERT_H__

int abort(const char *, int);

void _warn(const char*, int, const char*, ...);
void _panic(const char*, int, const char*, ...) __attribute__((noreturn));

#define warn(...) _warn(__FILE__, __LINE__, __VA_ARGS__)
#define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)

#define my_assert(cond) \
	((cond) ? (0) : (abort(__FILE__, __LINE__)))


#endif

