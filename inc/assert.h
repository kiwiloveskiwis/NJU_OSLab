#ifndef __ASSERT_H__
#define __ASSERT_H__

int abort(const char *, int);


#define my_assert(cond) \
	((cond) ? (0) : (abort(__FILE__, __LINE__)))

#endif

