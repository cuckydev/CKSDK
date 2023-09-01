#include <CKSDK/CKSDK.h>

#include <ctype.h>

extern "C"
{
	KEEP int toupper(int c)
	{
		if (c >= 'a' && c <= 'z')
			return c - ('a' - 'A');
		return c;
	}

	KEEP int tolower(int c)
	{
		if (c >= 'A' && c <= 'Z')
			return c + ('a' - 'A');
		return c;
	}
}
