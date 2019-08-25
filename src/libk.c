#include "kernel.h"

size_t
strlen(const char *s)
{
	size_t n;

	for (n = 0; *s++ != '\0'; n++)
		;

	return n;
}

/* From musl libc */
int
strcmp(const char *l, const char *r)
{
	for (; *l==*r && *l; l++, r++);
	return *(unsigned char *)l - *(unsigned char *)r;
}

/* From musl libc */
int
strncmp(const char *_l, const char *_r, size_t n)
{
    const unsigned char *l=(void *)_l, *r=(void *)_r;
    if (!n--) return 0;
    for (; *l && *r && n && *l == *r ; l++, r++, n--);
    return *l - *r;
}

void *
memcpy(void *dst, const void *src, size_t len)
{
	unsigned char *p1, *p2, *end;

	p1 = (unsigned char *)dst;
	p2 = (unsigned char *)src;
	end = p1 + len;

	while (p1 != end)
		*p1++ = *p2++;

	return dst;
}

void *
memset(void *s, int c, size_t n)
{
	unsigned char *p, *end;

	if (n == 0)
		return s;

	p = s;
	end = p + n;

	while (p != end)
		*p++ = c;

	return s;
}
