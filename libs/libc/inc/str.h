// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef STR_H
#define STR_H

#include <def.h>

PURE u32 strlen(const char *s) NONNULL;
PURE u32 strnlen(const char *s, u32 max) NONNULL;
u32 strlcpy(char *dst, const char *src, u32 size) NONNULL;
PURE char *strchr(char *s, char c) NONNULL;
PURE char *strrchr(char *s, char c) NONNULL;
u32 strlcat(char *dst, const char *src, u32 size) NONNULL;
s32 strcmp(const char *s1, const char *s2) NONNULL;
s32 strncmp(const char *s1, const char *s2, u32 n) NONNULL;
char *strinv(char *s) NONNULL;
ATTR((malloc)) char *strdup(const char *s) NONNULL;

#ifdef KERNEL

INLINE PURE u32 strlen_user(const char *s) NONNULL;
INLINE PURE u32 strnlen_user(const char *s, u32 max) NONNULL;
INLINE u32 strlcpy_user(char *dst, const char *src, u32 size) NONNULL;
INLINE PURE char *strchr_user(char *s, char c) NONNULL;
INLINE PURE char *strrchr_user(char *s, char c) NONNULL;
INLINE u32 strlcat_user(char *dst, const char *src, u32 size) NONNULL;
INLINE s32 strcmp_user(const char *s1, const char *s2) NONNULL;
INLINE s32 strncmp_user(const char *s1, const char *s2, u32 n) NONNULL;
INLINE char *strinv_user(char *s) NONNULL;
INLINE ATTR((malloc)) char *strdup_user(const char *s) NONNULL;

#endif

const char *strerror(u32 err);

#endif
