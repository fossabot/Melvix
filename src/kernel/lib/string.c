#include <stddef.h>

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

size_t strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *) s1 - *(const unsigned char *) s2;
}

char *strcat(char *dest, const char *src) {
    unsigned int i = 0;
    unsigned int j = 0;
    for (i = 0; dest[i] != 0; i++) {}
    for (j = 0; src[j] != 0; j++) {
        dest[i + j] = src[j];
    }
    dest[i + j] = 0;
    return dest;
}

char *strcpy(char *dest, const char *src) {
    unsigned int i = 0;
    for (i = 0; src[i] != 0; i++) {
        dest[i] = src[i];
    }
    dest[i] = 0;
    return dest;
}

void *itoa(int i, char *b, int base) {
    int temp_i;
    temp_i = i;
    int stringLen = 1;

    while ((int) temp_i / base != 0) {
        temp_i = (int) temp_i / base;
        stringLen++;
    }

    temp_i = i;
    do {
        *(b + stringLen - 1) = (temp_i % base) + '0';
        temp_i = (int) temp_i / base;
    } while (stringLen--);
}