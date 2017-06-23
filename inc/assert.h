#ifndef __ASSERT_H__
#define __ASSERT_H__

#define E_SUCCESS 0
#define E_INVALID -1

#define E_FAILURE -1
#define E_ALIGN -2
#define E_OUTOFMEM -3
#define E_INVID -4
#define E_ACCESS -5
#define E_DQUOT -6
#define E_NOENT -7
#define E_BADFD -8
#define E_NOTIMPLEMENTED -9
#define E_EMPTYPATH -10
#define E_SYSCALL_NOT_FOUND 0x80000000


int abort(const char *, int);

void _warn(const char*, int, const char*, ...);
void _panic(const char*, int, const char*, ...) __attribute__((noreturn));

#define warn(...) _warn(__FILE__, __LINE__, __VA_ARGS__)
#define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)

#define my_assert(x)		\
	do { if (!(x)) panic("assertion failed: %s", #x); } while (0)


#endif

