#include "switchc.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *allocate_memory(size_t size)
{
    void *pointer = malloc(size == 0 ? 1 : size);
    if (pointer == NULL) {
        fprintf(stderr, "fatal: out of memory\n");
        exit(3);
    }
    return pointer;
}

void *resize_memory(void *pointer, size_t size)
{
    void *result = realloc(pointer, size == 0 ? 1 : size);
    if (result == NULL) {
        fprintf(stderr, "fatal: out of memory\n");
        exit(3);
    }
    return result;
}

char *duplicate_string(const char *value)
{
    size_t length;
    char *copy;
    if (value == NULL) {
        return NULL;
    }
    length = strlen(value);
    copy = (char *)allocate_memory(length + 1);
    memcpy(copy, value, length + 1);
    return copy;
}

char *lowercase_string(const char *value)
{
    size_t index;
    size_t length;
    char *copy;
    if (value == NULL) {
        return NULL;
    }
    length = strlen(value);
    copy = (char *)allocate_memory(length + 1);
    for (index = 0; index < length; index++) {
        copy[index] = (char)tolower((unsigned char)value[index]);
    }
    copy[length] = '\0';
    return copy;
}
