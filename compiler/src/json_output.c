#include "json_output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int g_json_mode = 0;
JsonTokenList g_json_tokens = {NULL, 0, 0};

void json_tokens_init(void)
{
    g_json_tokens.items = NULL;
    g_json_tokens.count = 0;
    g_json_tokens.capacity = 0;
}

void json_tokens_add(const char *type, const char *lexeme, int line, int column)
{
    JsonToken *item;
    if (!g_json_mode) {
        return;
    }
    if (g_json_tokens.count == g_json_tokens.capacity) {
        int capacity = g_json_tokens.capacity == 0 ? 32 : g_json_tokens.capacity * 2;
        JsonToken *items = (JsonToken *)realloc(g_json_tokens.items, (size_t)capacity * sizeof(JsonToken));
        if (items == NULL) {
            fprintf(stderr, "fatal: out of memory\n");
            exit(3);
        }
        g_json_tokens.items = items;
        g_json_tokens.capacity = capacity;
    }
    item = &g_json_tokens.items[g_json_tokens.count++];
    item->type = duplicate_string(type != NULL ? type : "TOKEN");
    item->lexeme = duplicate_string(lexeme != NULL ? lexeme : "");
    item->line = line;
    item->column = column;
}

void json_tokens_free(void)
{
    int index;
    for (index = 0; index < g_json_tokens.count; index++) {
        free(g_json_tokens.items[index].type);
        free(g_json_tokens.items[index].lexeme);
    }
    free(g_json_tokens.items);
    g_json_tokens.items = NULL;
    g_json_tokens.count = 0;
    g_json_tokens.capacity = 0;
}

static void json_string(const char *value)
{
    const unsigned char *cursor = (const unsigned char *)(value != NULL ? value : "");
    putchar('"');
    while (*cursor != '\0') {
        switch (*cursor) {
            case '"': fputs("\\\"", stdout); cursor++; break;
            case '\\': fputs("\\\\", stdout); cursor++; break;
            case '\b': fputs("\\b", stdout); cursor++; break;
            case '\f': fputs("\\f", stdout); cursor++; break;
            case '\n': fputs("\\n", stdout); cursor++; break;
            case '\r': fputs("\\r", stdout); cursor++; break;
            case '\t': fputs("\\t", stdout); cursor++; break;
            default:
                if (*cursor < 0x20) {
                    printf("\\u%04x", *cursor);
                    cursor++;
                } else {
                    putchar(*cursor);
                    cursor++;
                }
        }
    }
    putchar('"');
}
