#ifndef DRIVER_H
#define DRIVER_H

#include "sqlmini.h"

typedef struct ExprNode {
    Expression *expression;
    struct ExprNode *next;
} ExprNode;

extern DiagnosticList *g_diagnostics;
extern Query *g_query;
extern int g_show_tokens;
extern int g_token_line;
extern int g_token_column;
extern int g_token_count;

void print_token_line(const char *type, const char *text, int line, int column);

#endif
