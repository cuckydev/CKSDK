#include <CKSDK/CKSDK.h>

#include <string.h>

extern "C"
{
	KEEP void *memset(void *s, int c, size_t n)
	{
		unsigned char *p = (unsigned char *)s;
		while (n-- > 0)
			*p++ = (unsigned char)c;
		return s;
	}

	KEEP void *memcpy(void *dest, const void *src, size_t n)
	{
		unsigned char *d = (unsigned char *)dest;
		const unsigned char *s = (const unsigned char *)src;
		while (n-- > 0)
			*d++ = *s++;
		return dest;
	}

	KEEP void *memmove(void *dest, const void *src, size_t n)
	{
		unsigned char *d = (unsigned char *)dest;
		const unsigned char *s = (const unsigned char *)src;
		if (d < s)
		{
			while (n-- > 0)
				*d++ = *s++;
		}
		else
		{
			d += n;
			s += n;
			while (n-- > 0)
				*--d = *--s;
		}
		return dest;
	}

	KEEP int memcmp(const void *s1, const void *s2, size_t n)
	{
		const unsigned char *p1 = (const unsigned char *)s1;
		const unsigned char *p2 = (const unsigned char *)s2;

		while (n-- > 0)
		{
			if (*p1++ != *p2++)
				return p1[-1] < p2[-1] ? -1 : 1;
		}
		return 0;
	}

	KEEP void *memchr(const void *s, int c, size_t n)
	{
		const unsigned char *p = (const unsigned char *)s;
		while (n-- > 0)
		{
			if (*p == (unsigned char)c)
				return (void *)p;
			p++;
		}
		return NULL;
	}

	KEEP int strcmp(const char *s1, const char *s2)
	{
		const unsigned char *p1 = (const unsigned char *)s1;
		const unsigned char *p2 = (const unsigned char *)s2;

		while (*p1 && *p1 == *p2)
		{
			p1++;
			p2++;
		}

		return (*p1 > *p2) - (*p2 > *p1);
	}
}
