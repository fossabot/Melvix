// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <mem.h>

u32 strlen(const char *s)
{
	const char *ss = s;
	while (*ss)
		ss++;
	return ss - s;
}

char *strcpy(char *dst, const char *src)
{
	char *q = dst;
	const char *p = src;
	char ch;

	do {
		*q++ = ch = *p++;
	} while (ch);

	return dst;
}

char *strncpy(char *dst, const char *src, u32 n)
{
	char *q = dst;

	while (n-- && (*dst++ = *src++))
		;

	return q;
}

int strcmp(const char *s1, const char *s2)
{
	const u8 *c1 = (const u8 *)s1;
	const u8 *c2 = (const u8 *)s2;
	u8 ch;
	int d = 0;

	while (1) {
		d = (int)(ch = *c1++) - (int)*c2++;
		if (d || !ch)
			break;
	}

	return d;
}

int strncmp(const char *s1, const char *s2, u32 n)
{
	const u8 *c1 = (const u8 *)s1;
	const u8 *c2 = (const u8 *)s2;
	u8 ch;
	int d = 0;

	while (n--) {
		d = (int)(ch = *c1++) - (int)*c2++;
		if (d || !ch)
			break;
	}

	return d;
}

char *strchr(const char *s, int c)
{
	while (*s != (char)c) {
		if (!*s)
			return NULL;
		s++;
	}

	return (char *)s;
}

char *strrchr(const char *s, int c)
{
	char *ret = 0;

	do {
		if (*s == c)
			ret = (char *)s;
	} while (*s++);

	return ret;
}

char *strcat(char *dst, const char *src)
{
	strcpy(strchr(dst, '\0'), src);
	return dst;
}

char *strncat(char *dst, const char *src, u32 n)
{
	strncpy(strchr(dst, '\0'), src, n);
	return dst;
}

char *strinv(char *s)
{
	u32 s_str = strlen(s);

	int iterations = (int)s_str / 2;
	for (int i = 0; i < iterations; i++) {
		char aux = s[i];
		s[i] = s[(s_str - i) - 1];
		s[(s_str - i) - 1] = aux;
	}
	return s;
}

char *strdup(const char *s)
{
	int l = strlen(s) + 1;
	char *d = malloc(l);

	memcpy(d, s, l);

	return d;
}
