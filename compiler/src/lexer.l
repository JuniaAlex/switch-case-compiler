%{
#include "parser.tab.h"
#include "driver.h"
#include "json_output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int g_line = 1;
int g_column = 1;
int g_token_line = 1;
int g_token_column = 1;
int g_token_count = 0;
static int g_token_index = 0;
static int comment_start_line = 1;
static int comment_start_column = 1;

static void update_position(const char *text, int length)
{
    int index;
    for (index = 0; index < length; index++) {
        if (text[index] == '\n') {
            g_line++;
            g_column = 1;
        } else {
            g_column++;
        }
    }
}

static int keyword_code(const char *text)
{
    if (strcmp(text, "select") == 0) return KW_SELECT;
    if (strcmp(text, "distinct") == 0) return KW_DISTINCT;
    if (strcmp(text, "from") == 0) return KW_FROM;
    if (strcmp(text, "where") == 0) return KW_WHERE;
    if (strcmp(text, "order") == 0) return KW_ORDER;
    if (strcmp(text, "by") == 0) return KW_BY;
    if (strcmp(text, "asc") == 0) return KW_ASC;
    if (strcmp(text, "desc") == 0) return KW_DESC;
    if (strcmp(text, "and") == 0) return KW_AND;
    if (strcmp(text, "or") == 0) return KW_OR;
    if (strcmp(text, "not") == 0) return KW_NOT;
    if (strcmp(text, "as") == 0) return KW_AS;
    if (strcmp(text, "in") == 0) return KW_IN;
    if (strcmp(text, "between") == 0) return KW_BETWEEN;
    if (strcmp(text, "like") == 0) return KW_LIKE;
    if (strcmp(text, "is") == 0) return KW_IS;
    if (strcmp(text, "null") == 0) return KW_NULL;
    if (strcmp(text, "true") == 0) return KW_TRUE;
    if (strcmp(text, "false") == 0) return KW_FALSE;
    return 0;
}

static const char *keyword_name(int token)
{
    switch (token) {
        case KW_SELECT: return "SELECT";
        case KW_DISTINCT: return "DISTINCT";
        case KW_FROM: return "FROM";
        case KW_WHERE: return "WHERE";
        case KW_ORDER: return "ORDER";
        case KW_BY: return "BY";
        case KW_ASC: return "ASC";
        case KW_DESC: return "DESC";
        case KW_AND: return "AND";
        case KW_OR: return "OR";
        case KW_NOT: return "NOT";
        case KW_AS: return "AS";
        case KW_IN: return "IN";
        case KW_BETWEEN: return "BETWEEN";
        case KW_LIKE: return "LIKE";
        case KW_IS: return "IS";
        case KW_NULL: return "NULL";
        case KW_TRUE: return "TRUE";
        case KW_FALSE: return "FALSE";
        case IDENT: return "IDENTIFIER";
        case INT_LITERAL: return "INT_LITERAL";
        case FLOAT_LITERAL: return "FLOAT_LITERAL";
        case STRING: return "STRING";
        case STAR: return "STAR";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case SLASH: return "SLASH";
        case EQ: return "EQUAL";
        case NE: return "NOT_EQUAL";
        case LT: return "LESS";
        case LE: return "LESS_EQUAL";
        case GT: return "GREATER";
        case GE: return "GREATER_EQUAL";
        case COMMA: return "COMMA";
        case DOT: return "DOT";
        case LPAREN: return "LEFT_PAREN";
        case RPAREN: return "RIGHT_PAREN";
        case SEMI: return "SEMICOLON";
        default: return "TOKEN";
    }
}

static char *unquote_string(const char *text)
{
    int length = (int)strlen(text);
    int index = 1;
    int output = 0;
    char *result = (char *)malloc((size_t)length + 2);
    if (result == NULL) {
        fprintf(stderr, "fatal: out of memory\n");
        exit(3);
    }
    while (index < length) {
        if (text[index] == '\'') {
            if (index + 1 < length && text[index + 1] == '\'') {
                result[output++] = '\'';
                index += 2;
                continue;
            }
            break;
        }
        result[output++] = text[index++];
    }
    result[output] = '\0';
    return result;
}

void print_token_line(const char *type, const char *text, int line, int column)
{
    if (g_token_index == 0) {
        printf("%-4s %-14s %-22s %-6s %-6s\n", "#", "TOKEN", "LEXEME", "LINE", "COL");
        printf("%-4s %-14s %-22s %-6s %-6s\n", "---", "-----", "------", "----", "---");
    }
    g_token_index++;
    printf("%-4d %-14s %-22s %-6d %-6d\n", g_token_index, type, text, line, column);
}

#define YY_USER_ACTION do { \
    yylloc.first_line = g_line; \
    yylloc.first_column = g_column; \
    g_token_line = g_line; \
    g_token_column = g_column; \
    update_position(yytext, yyleng); \
    yylloc.last_line = g_line; \
    yylloc.last_column = g_column > 1 ? g_column - 1 : 1; \
} while (0);

#define EMIT(token, name) do { \
    g_token_count++; \
    json_tokens_add((name), yytext, yylloc.first_line, yylloc.first_column); \
    if (g_show_tokens) { \
        print_token_line((name), yytext, yylloc.first_line, yylloc.first_column); \
    } \
    return (token); \
} while (0)
%}

%option noyywrap nounput noinput

%x COMMENT

%%

"/*" {
    comment_start_line = yylloc.first_line;
    comment_start_column = yylloc.first_column;
    BEGIN(COMMENT);
}

"//"[^\n]* { }

"--"[^\n]* { }

[ \t\r\n]+ { }

\'([^'\n]|'')*\' {
    yylval.text = unquote_string(yytext);
    EMIT(STRING, "STRING");
}

\'([^'\n]|'')* {
    diag_add(g_diagnostics, "L002", yylloc.first_line, yylloc.first_column,
             "unterminated string literal");
    yylval.text = unquote_string(yytext);
    EMIT(STRING, "STRING");
}

[0-9]+\.[0-9]+ {
    yylval.number = strtod(yytext, NULL);
    EMIT(FLOAT_LITERAL, "FLOAT_LITERAL");
}

[0-9]+ {
    yylval.integer = strtoll(yytext, NULL, 10);
    EMIT(INT_LITERAL, "INT_LITERAL");
}

[A-Za-z_][A-Za-z0-9_]* {
    char *lower = lowercase_string(yytext);
    int keyword = keyword_code(lower);
    if (keyword != 0) {
        free(lower);
        EMIT(keyword, keyword_name(keyword));
    }
    yylval.text = lower;
    EMIT(IDENT, "IDENTIFIER");
}

"<=" { EMIT(LE, "LESS_EQUAL"); }
"<>" { EMIT(NE, "NOT_EQUAL"); }
"!=" { EMIT(NE, "NOT_EQUAL"); }
">=" { EMIT(GE, "GREATER_EQUAL"); }
"=" { EMIT(EQ, "EQUAL"); }
"<" { EMIT(LT, "LESS"); }
">" { EMIT(GT, "GREATER"); }
"+" { EMIT(PLUS, "PLUS"); }
"-" { EMIT(MINUS, "MINUS"); }
"*" { EMIT(STAR, "STAR"); }
"/" { EMIT(SLASH, "SLASH"); }
"," { EMIT(COMMA, "COMMA"); }
"." { EMIT(DOT, "DOT"); }
"(" { EMIT(LPAREN, "LEFT_PAREN"); }
")" { EMIT(RPAREN, "RIGHT_PAREN"); }
";" { EMIT(SEMI, "SEMICOLON"); }

<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>\n { }
<COMMENT>. { }
<COMMENT><<EOF>> {
    diag_add(g_diagnostics, "L003", comment_start_line, comment_start_column,
             "unterminated block comment");
    BEGIN(INITIAL);
    return 0;
}

<<EOF>> {
    json_tokens_add("EOF", "EOF", g_line, g_column);
    if (g_show_tokens) {
        print_token_line("EOF", "EOF", g_line, g_column);
    }
    return 0;
}

. {
    diag_add(g_diagnostics, "L001", yylloc.first_line, yylloc.first_column,
             "illegal character '%c' (0x%02X)", yytext[0], (unsigned char)yytext[0]);
}
