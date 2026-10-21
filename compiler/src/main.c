#include "sqlmini.h"
#include "driver.h"
#include "json_output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define IS_TERMINAL(stream) _isatty(_fileno(stream))
#else
#include <unistd.h>
#define IS_TERMINAL(stream) isatty(fileno(stream))
#endif

int yyparse(void);
void yy_scan_string(const char *text);
void yylex_destroy(void);

static void print_usage(const char *program)
{
    printf("SQL Mini Compiler - Flex/Bison Front-End\n\n");
    printf("Usage: %s [options] [file.sql]\n\n", program);
    printf("Options:\n");
    printf("  -e, --query TEXT   compile TEXT instead of a file or stdin\n");
    printf("      --tokens       print the token stream produced by Flex\n");
    printf("      --ast          print the parse tree / AST (default)\n");
    printf("      --no-ast       suppress the AST\n");
    printf("      --schema       print the supported database schema\n");
    printf("      --json         emit machine-readable analysis JSON\n");
    printf("      --quiet        print only diagnostics and the final result\n");
    printf("  -h, --help         show this help text\n\n");
    printf("Exit codes: 0 valid, 1 lexical/syntax error, 2 semantic error, 3 usage/I/O error\n");
}

static char *read_stream(FILE *stream)
{
    size_t capacity = 1024;
    size_t length = 0;
    char *buffer = (char *)allocate_memory(capacity);

    for (;;) {
        size_t available = capacity - length - 1;
        size_t read_count;
        if (available == 0) {
            capacity *= 2;
            buffer = (char *)resize_memory(buffer, capacity);
            available = capacity - length - 1;
        }
        read_count = fread(buffer + length, 1, available, stream);
        length += read_count;
        if (read_count < available) {
            break;
        }
    }
    buffer[length] = '\0';
    return buffer;
}

static char *read_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    char *content;
    if (file == NULL) {
        fprintf(stderr, "error: cannot open file '%s'\n", path);
        return NULL;
    }
    content = read_stream(file);
    fclose(file);
    return content;
}

static void strip_utf8_bom(char *text)
{
    unsigned char *bytes = (unsigned char *)text;
    size_t length = strlen(text);
    if (length >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF) {
        memmove(text, text + 3, length - 2);
    }
}

static int count_code_prefix(const DiagnosticList *diagnostics, char prefix)
{
    int count = 0;
    int index;
    for (index = 0; index < diagnostics->count; index++) {
        if (diagnostics->items[index].code[0] == prefix) {
            count++;
        }
    }
    return count;
}

int main(int argc, char **argv)
{
    const char *query_argument = NULL;
    const char *file_path = NULL;
    int show_tokens = 0;
    int show_ast = 1;
    int show_schema = 0;
    int quiet = 0;
    int json_mode = 0;
    int parse_status;
    int syntax_ok;
    int lex_errors;
    int semantic_errors;
    char *input = NULL;
    DiagnosticList diagnostics;
    int index;

    for (index = 1; index < argc; index++) {
        const char *argument = argv[index];
        if (strcmp(argument, "-h") == 0 || strcmp(argument, "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argument, "-e") == 0 || strcmp(argument, "--query") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "error: %s requires a value\n", argument);
                return 3;
            }
            query_argument = argv[++index];
        } else if (strcmp(argument, "--tokens") == 0) {
            show_tokens = 1;
        } else if (strcmp(argument, "--ast") == 0) {
            show_ast = 1;
        } else if (strcmp(argument, "--no-ast") == 0) {
            show_ast = 0;
        } else if (strcmp(argument, "--schema") == 0) {
            show_schema = 1;
        } else if (strcmp(argument, "--json") == 0) {
            json_mode = 1;
        } else if (strcmp(argument, "--quiet") == 0) {
            quiet = 1;
        } else if (argument[0] == '-') {
            fprintf(stderr, "error: unknown option '%s'\n", argument);
            print_usage(argv[0]);
            return 3;
        } else if (file_path == NULL) {
            file_path = argument;
        } else {
            fprintf(stderr, "error: multiple input files are not supported\n");
            return 3;
        }
    }

    if (json_mode && show_schema) {
        fprintf(stderr, "error: --schema and --json cannot be combined\n");
        return 3;
    }
    if (json_mode) {
        quiet = 1;
    }
    json_tokens_init();
    g_json_mode = json_mode;

    if (show_schema) {
        print_catalog();
        if (query_argument == NULL && file_path == NULL) {
            return 0;
        }
        if (!quiet) {
            printf("\n");
        }
    }

    if (query_argument != NULL) {
        input = duplicate_string(query_argument);
    } else if (file_path != NULL) {
        input = read_file(file_path);
        if (input == NULL) {
            return 3;
        }
    } else {
        if (IS_TERMINAL(stdin)) {
            fprintf(stderr, "Enter a SQL query, then press Ctrl+Z and Enter:\n");
        }
        input = read_stream(stdin);
    }

    if (input[0] == '\0') {
        if (json_mode) {
            diag_init(&diagnostics);
            g_diagnostics = &diagnostics;
            diag_add(&diagnostics, "P001", 1, 1, "empty input, expected SELECT");
            json_print_result(NULL, &diagnostics, 0, 0);
            diag_free(&diagnostics);
            json_tokens_free();
            free(input);
            return 1;
        }
        fprintf(stderr, "[P001] line 1, column 1: empty input, expected SELECT\n");
        printf("Result: INVALID (syntax)\n");
        free(input);
        return 1;
    }

    strip_utf8_bom(input);

    diag_init(&diagnostics);
    g_diagnostics = &diagnostics;
    g_query = NULL;
    g_show_tokens = show_tokens && !quiet && !json_mode;

    if (!quiet) {
        printf("SQL Mini Compiler - Flex/Bison Front-End\n");
        printf("Query: %s\n\n", input);
        printf("[1] Lexical analysis (Flex)\n");
        if (show_tokens) {
            printf("\n");
        }
    }

    yy_scan_string(input);
    parse_status = yyparse();
    yylex_destroy();

    lex_errors = count_code_prefix(&diagnostics, 'L');

    if (!quiet) {
        if (show_tokens) {
            printf("\nTokens: %d, lexical errors: %d\n", g_token_count, lex_errors);
        } else {
            printf("\n[1] Lexical analysis: %d token%s, %d error%s\n",
                   g_token_count,
                   g_token_count == 1 ? "" : "s",
                   lex_errors,
                   lex_errors == 1 ? "" : "s");
        }
    }

    syntax_ok = parse_status == 0 && g_query != NULL && diagnostics.count == 0;

    if (!syntax_ok) {
        if (json_mode) {
            json_print_result(g_query, &diagnostics, 0, lex_errors);
            query_free(g_query);
            diag_free(&diagnostics);
            json_tokens_free();
            free(input);
            return 1;
        }
        if (!quiet) {
            printf("[2] Syntax analysis: FAIL\n\n");
        }
        fflush(stdout);
        diag_print(&diagnostics);
        if (!quiet) {
            printf("\n");
        }
        if (lex_errors > 0) {
            printf("Result: INVALID (lexical)\n");
        } else {
            printf("Result: INVALID (syntax)\n");
        }
        query_free(g_query);
        diag_free(&diagnostics);
        free(input);
        return 1;
    }

    if (!quiet) {
        printf("[2] Syntax analysis: PASS\n\n");
        if (show_ast) {
            printf("[3] Parse tree / AST:\n");
            print_ast(g_query);
            printf("\n");
        }
    }

    semantic_errors = analyze_query(g_query, &diagnostics);

    if (json_mode) {
        json_print_result(g_query, &diagnostics, 1, lex_errors);
        query_free(g_query);
        diag_free(&diagnostics);
        json_tokens_free();
        free(input);
        return semantic_errors > 0 ? 2 : 0;
    }

    if (semantic_errors > 0) {
        if (!quiet) {
            printf("[4] Semantic analysis: %d error%s\n\n",
                   semantic_errors,
                   semantic_errors == 1 ? "" : "s");
        }
        fflush(stdout);
        diag_print(&diagnostics);
        if (!quiet) {
            printf("\n");
        }
        printf("Result: INVALID (semantic)\n");
        query_free(g_query);
        diag_free(&diagnostics);
        free(input);
        return 2;
    }

    if (!quiet) {
        printf("[4] Semantic analysis: PASS\n");
        printf("    Tables: %d, select items: %d, order items: %d\n",
               g_query->table_list.count,
               g_query->select_list.count,
               g_query->order_list.count);
        printf("\n");
    }
    printf("Result: VALID\n");

    query_free(g_query);
    diag_free(&diagnostics);
    free(input);
    return 0;
}
