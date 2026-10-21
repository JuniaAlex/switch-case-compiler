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

static const char *literal_kind_name(LiteralKind kind)
{
    switch (kind) {
        case LITERAL_INTEGER: return "integer";
        case LITERAL_FLOAT: return "float";
        case LITERAL_STRING: return "string";
        case LITERAL_NULL: return "null";
        case LITERAL_BOOLEAN: return "boolean";
        default: return "unknown";
    }
}

static const char *unary_operator_name(UnaryOperator op)
{
    switch (op) {
        case UNARY_PLUS: return "+";
        case UNARY_MINUS: return "-";
        case UNARY_NOT: return "NOT";
        default: return "?";
    }
}

static const char *binary_operator_name(BinaryOperator op)
{
    switch (op) {
        case BINARY_ADD: return "+";
        case BINARY_SUBTRACT: return "-";
        case BINARY_MULTIPLY: return "*";
        case BINARY_DIVIDE: return "/";
        case BINARY_EQUAL: return "=";
        case BINARY_NOT_EQUAL: return "!=";
        case BINARY_LESS: return "<";
        case BINARY_LESS_EQUAL: return "<=";
        case BINARY_GREATER: return ">";
        case BINARY_GREATER_EQUAL: return ">=";
        case BINARY_AND: return "AND";
        case BINARY_OR: return "OR";
        default: return "?";
    }
}

static void print_expression(const Expression *expression)
{
    int index;
    if (expression == NULL) {
        fputs("null", stdout);
        return;
    }
    switch (expression->kind) {
        case EXPRESSION_LITERAL:
            fputs("{\"kind\":\"literal\",\"literalKind\":", stdout);
            json_string(literal_kind_name(expression->as.literal.kind));
            fputs(",\"value\":", stdout);
            if (expression->as.literal.kind == LITERAL_NULL) {
                fputs("null", stdout);
            } else if (expression->as.literal.kind == LITERAL_BOOLEAN) {
                fputs(expression->as.literal.boolean_value ? "true" : "false", stdout);
            } else if (expression->as.literal.kind == LITERAL_STRING) {
                json_string(expression->as.literal.text);
            } else if (expression->as.literal.kind == LITERAL_FLOAT) {
                printf("%.15g", expression->as.literal.float_value);
            } else {
                printf("%lld", expression->as.literal.integer_value);
            }
            printf(",\"line\":%d,\"column\":%d}", expression->line, expression->column);
            break;
        case EXPRESSION_COLUMN:
            fputs("{\"kind\":\"column\",\"qualifier\":", stdout);
            if (expression->as.column.qualifier != NULL) {
                json_string(expression->as.column.qualifier);
            } else {
                fputs("null", stdout);
            }
            fputs(",\"name\":", stdout);
            json_string(expression->as.column.name);
            printf(",\"line\":%d,\"column\":%d}", expression->line, expression->column);
            break;
        case EXPRESSION_UNARY:
            fputs("{\"kind\":\"unary\",\"op\":", stdout);
            json_string(unary_operator_name(expression->as.unary.op));
            fputs(",\"operand\":", stdout);
            print_expression(expression->as.unary.operand);
            printf(",\"line\":%d,\"column\":%d}", expression->line, expression->column);
            break;
        case EXPRESSION_BINARY:
            fputs("{\"kind\":\"binary\",\"op\":", stdout);
            json_string(binary_operator_name(expression->as.binary.op));
            fputs(",\"left\":", stdout);
            print_expression(expression->as.binary.left);
            fputs(",\"right\":", stdout);
            print_expression(expression->as.binary.right);
            printf(",\"line\":%d,\"column\":%d}", expression->line, expression->column);
            break;
        case EXPRESSION_LIKE:
            printf("{\"kind\":\"like\",\"negated\":%s,\"value\":",
                   expression->as.like.negated ? "true" : "false");
            print_expression(expression->as.like.value);
            fputs(",\"pattern\":", stdout);
            print_expression(expression->as.like.pattern);
            printf(",\"line\":%d,\"column\":%d}", expression->line, expression->column);
            break;
        case EXPRESSION_IN:
            printf("{\"kind\":\"in\",\"negated\":%s,\"value\":",
                   expression->as.in.negated ? "true" : "false");
            print_expression(expression->as.in.value);
            fputs(",\"items\":[", stdout);
            for (index = 0; index < expression->as.in.count; index++) {
                if (index > 0) {
                    putchar(',');
                }
                print_expression(expression->as.in.items[index]);
            }
            printf("],\"line\":%d,\"column\":%d}", expression->line, expression->column);
            break;
        case EXPRESSION_BETWEEN:
            printf("{\"kind\":\"between\",\"negated\":%s,\"value\":",
                   expression->as.between.negated ? "true" : "false");
            print_expression(expression->as.between.value);
            fputs(",\"lower\":", stdout);
            print_expression(expression->as.between.lower);
            fputs(",\"upper\":", stdout);
            print_expression(expression->as.between.upper);
            printf(",\"line\":%d,\"column\":%d}", expression->line, expression->column);
            break;
        case EXPRESSION_IS_NULL:
            printf("{\"kind\":\"is_null\",\"negated\":%s,\"value\":",
                   expression->as.is_null.negated ? "true" : "false");
            print_expression(expression->as.is_null.value);
            printf(",\"line\":%d,\"column\":%d}", expression->line, expression->column);
            break;
        default:
            fputs("null", stdout);
    }
}

static void print_query(const Query *query)
{
    int index;
    if (query == NULL) {
        fputs("null", stdout);
        return;
    }
    printf("{\"kind\":\"query\",\"distinct\":%s,\"line\":%d,\"column\":%d,\"selectList\":[",
           query->distinct ? "true" : "false", query->line, query->column);
    for (index = 0; index < query->select_list.count; index++) {
        const SelectItem *item = &query->select_list.items[index];
        if (index > 0) {
            putchar(',');
        }
        printf("{\"star\":%s,\"alias\":", item->star ? "true" : "false");
        if (item->alias != NULL) {
            json_string(item->alias);
        } else {
            fputs("null", stdout);
        }
        fputs(",\"expression\":", stdout);
        print_expression(item->expression);
        printf(",\"line\":%d,\"column\":%d}", item->line, item->column);
    }
    fputs("],\"fromList\":[", stdout);
    for (index = 0; index < query->table_list.count; index++) {
        const TableReference *table = &query->table_list.items[index];
        if (index > 0) {
            putchar(',');
        }
        fputs("{\"name\":", stdout);
        json_string(table->name);
        fputs(",\"alias\":", stdout);
        if (table->alias != NULL) {
            json_string(table->alias);
        } else {
            fputs("null", stdout);
        }
        printf(",\"line\":%d,\"column\":%d}", table->line, table->column);
    }
    fputs("],\"where\":", stdout);
    print_expression(query->where);
    fputs(",\"orderList\":[", stdout);
    for (index = 0; index < query->order_list.count; index++) {
        const OrderItem *item = &query->order_list.items[index];
        if (index > 0) {
            putchar(',');
        }
        printf("{\"ascending\":%s,\"expression\":", item->ascending ? "true" : "false");
        print_expression(item->expression);
        printf(",\"line\":%d,\"column\":%d}", item->line, item->column);
    }
    fputs("]}", stdout);
}

static const char *diagnostic_phase(const Diagnostic *diagnostic)
{
    switch (diagnostic->code[0]) {
        case 'L': return "lexical";
        case 'P': return "syntax";
        case 'S': return "semantic";
        default: return "internal";
    }
}

static void print_diagnostic(const Diagnostic *diagnostic)
{
    fputs("{\"code\":", stdout);
    json_string(diagnostic->code);
    fputs(",\"phase\":", stdout);
    json_string(diagnostic_phase(diagnostic));
    printf(",\"line\":%d,\"column\":%d,\"message\":", diagnostic->line, diagnostic->column);
    json_string(diagnostic->message);
    putchar('}');
}

static void print_diagnostics(const DiagnosticList *diagnostics, char prefix)
{
    int index;
    int first = 1;
    putchar('[');
    for (index = 0; index < diagnostics->count; index++) {
        const Diagnostic *diagnostic = &diagnostics->items[index];
        if (prefix != '\0' && diagnostic->code[0] != prefix) {
            continue;
        }
        if (!first) {
            putchar(',');
        }
        first = 0;
        print_diagnostic(diagnostic);
    }
    putchar(']');
}

static const Diagnostic *first_diagnostic(const DiagnosticList *diagnostics, char prefix)
{
    int index;
    for (index = 0; index < diagnostics->count; index++) {
        if (diagnostics->items[index].code[0] == prefix) {
            return &diagnostics->items[index];
        }
    }
    return NULL;
}

void json_print_result(const Query *query, const DiagnosticList *diagnostics,
                       int syntax_ok, int lex_errors)
{
    int index;
    int semantic_errors = 0;
    int first_token = 1;
    const Diagnostic *syntax_diagnostic;
    const char *stage;

    for (index = 0; index < diagnostics->count; index++) {
        if (diagnostics->items[index].code[0] == 'S') {
            semantic_errors++;
        }
    }

    if (!syntax_ok) {
        stage = lex_errors > 0 ? "lexical" : "syntax";
    } else if (semantic_errors > 0) {
        stage = "semantic";
    } else {
        stage = "valid";
    }

    printf("{\"ok\":%s,\"stage\":", (syntax_ok && semantic_errors == 0) ? "true" : "false");
    json_string(stage);
    fputs(",\"tokens\":[", stdout);
    for (index = 0; index < g_json_tokens.count; index++) {
        JsonToken *token = &g_json_tokens.items[index];
        if (!first_token) {
            putchar(',');
        }
        first_token = 0;
        printf("{\"index\":%d,\"type\":", index + 1);
        json_string(token->type);
        fputs(",\"lexeme\":", stdout);
        json_string(token->lexeme);
        printf(",\"line\":%d,\"column\":%d}", token->line, token->column);
    }
    fputs("],\"diagnostics\":", stdout);
    print_diagnostics(diagnostics, '\0');
    fputs(",\"syntax\":{\"ok\":", stdout);
    fputs(syntax_ok ? "true" : "false", stdout);
    fputs(",\"stage\":", stdout);
    if (syntax_ok) {
        fputs("null", stdout);
    } else {
        json_string(lex_errors > 0 ? "lexical" : "syntax");
    }
    fputs(",\"error\":", stdout);
    if (syntax_ok) {
        fputs("null", stdout);
    } else {
        syntax_diagnostic = first_diagnostic(diagnostics, lex_errors > 0 ? 'L' : 'P');
        if (syntax_diagnostic == NULL) {
            syntax_diagnostic = first_diagnostic(diagnostics, 'L');
        }
        json_string(syntax_diagnostic != NULL ? syntax_diagnostic->message : "syntax analysis failed");
    }
    fputs("},\"ast\":", stdout);
    if (syntax_ok) {
        print_query(query);
    } else {
        fputs("null", stdout);
    }
    fputs(",\"semantics\":{\"ok\":", stdout);
    fputs((syntax_ok && semantic_errors == 0) ? "true" : "false", stdout);
    fputs(",\"diagnostics\":", stdout);
    print_diagnostics(diagnostics, 'S');
    fputs("}}\n", stdout);
}
