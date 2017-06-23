#ifndef __STRING_H__
#define __STRING_H__

#include "common.h"

char *itoa(int);
void memcpy(void *, const void *, size_t);
void memset(void *, int, size_t);
size_t strlen(const char *);
void strcpy(char *d, const char *s);
char *strncpy(char *dest, const char *src, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);

#endif

