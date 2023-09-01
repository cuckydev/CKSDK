#pragma once
#include <cstdint>

extern "C"
{
	void *memset(void *dest, int c, size_t n);
	void *memcpy(void *dest, const void *src, size_t n);
	void *memmove(void *dest, const void *src, size_t n);
	int memcmp(const void *s1, const void *s2, size_t n);
	void *memchr(const void *s, int c, size_t n);
}
