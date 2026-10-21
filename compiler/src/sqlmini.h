#ifndef SQLMINI_H
#define SQLMINI_H

#include <stddef.h>

#define SQLMINI_MAX_MESSAGE 256
#define SQLMINI_MAX_CODE 8

typedef struct {
    char code[SQLMINI_MAX_CODE];
    int line;
    int column;
    char message[SQLMINI_MAX_MESSAGE];
} Diagnostic;

typedef struct {
    Diagnostic *items;
    int count;
    int capacity;
} DiagnosticList;

void diag_init(DiagnosticList *list);
void diag_free(DiagnosticList *list);
void diag_add(DiagnosticList *list, const char *code, int line, int column, const char *format, ...);
void diag_print(const DiagnosticList *list);

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_SELECT,
    TOKEN_DISTINCT,
    TOKEN_FROM,
    TOKEN_WHERE,
    TOKEN_ORDER,
    TOKEN_BY,
    TOKEN_ASC,
    TOKEN_DESC,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_AS,
    TOKEN_IN,
    TOKEN_BETWEEN,
    TOKEN_LIKE,
    TOKEN_IS,
    TOKEN_NULL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_STAR,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_SLASH,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_SEMICOLON
} TokenType;

typedef struct {
    TokenType type;
    char *text;
    double number;
    int is_float;
    int line;
    int column;
} Token;

typedef struct {
    Token *items;
    int count;
    int capacity;
} TokenList;

void token_list_init(TokenList *list);
void token_list_free(TokenList *list);
int lex_source(const char *source, TokenList *tokens, DiagnosticList *diagnostics);
const char *token_type_name(TokenType type);
const char *token_display(const Token *token);

typedef enum {
    LITERAL_INTEGER,
    LITERAL_FLOAT,
    LITERAL_STRING,
    LITERAL_NULL,
    LITERAL_BOOLEAN
} LiteralKind;

typedef enum {
    UNARY_PLUS,
    UNARY_MINUS,
    UNARY_NOT
} UnaryOperator;

typedef enum {
    BINARY_ADD,
    BINARY_SUBTRACT,
    BINARY_MULTIPLY,
    BINARY_DIVIDE,
    BINARY_EQUAL,
    BINARY_NOT_EQUAL,
    BINARY_LESS,
    BINARY_LESS_EQUAL,
    BINARY_GREATER,
    BINARY_GREATER_EQUAL,
    BINARY_AND,
    BINARY_OR
} BinaryOperator;

typedef enum {
    EXPRESSION_LITERAL,
    EXPRESSION_COLUMN,
    EXPRESSION_UNARY,
    EXPRESSION_BINARY,
    EXPRESSION_LIKE,
    EXPRESSION_IN,
    EXPRESSION_BETWEEN,
    EXPRESSION_IS_NULL
} ExpressionKind;

typedef struct Expression Expression;

struct Expression {
    ExpressionKind kind;
    int line;
    int column;
    union {
        struct {
            LiteralKind kind;
            long long integer_value;
            double float_value;
            char *text;
            int boolean_value;
        } literal;
        struct {
            char *qualifier;
            char *name;
        } column;
        struct {
            UnaryOperator op;
            Expression *operand;
        } unary;
        struct {
            BinaryOperator op;
            Expression *left;
            Expression *right;
        } binary;
        struct {
            Expression *value;
            Expression *pattern;
            int negated;
        } like;
        struct {
            Expression *value;
            Expression **items;
            int count;
            int capacity;
            int negated;
        } in;
        struct {
            Expression *value;
            Expression *lower;
            Expression *upper;
            int negated;
        } between;
        struct {
            Expression *value;
            int negated;
        } is_null;
    } as;
};

typedef struct {
    int star;
    Expression *expression;
    char *alias;
    int line;
    int column;
} SelectItem;

typedef struct {
    SelectItem *items;
    int count;
    int capacity;
} SelectList;

typedef struct {
    char *name;
    char *alias;
    int line;
    int column;
} TableReference;

typedef struct {
    TableReference *items;
    int count;
    int capacity;
} TableList;

typedef struct {
    Expression *expression;
    int ascending;
    int line;
    int column;
} OrderItem;

typedef struct {
    OrderItem *items;
    int count;
    int capacity;
} OrderList;

typedef struct {
    int distinct;
    SelectList select_list;
    TableList table_list;
    Expression *where;
    OrderList order_list;
    int line;
    int column;
} Query;

Query *parse_tokens(TokenList *tokens, DiagnosticList *diagnostics);
void query_free(Query *query);
void print_ast(const Query *query);
void print_tokens(const TokenList *tokens);

typedef enum {
    SQL_TYPE_UNKNOWN,
    SQL_TYPE_INTEGER,
    SQL_TYPE_FLOAT,
    SQL_TYPE_TEXT,
    SQL_TYPE_BOOLEAN,
    SQL_TYPE_NULL
} SqlType;

typedef struct {
    const char *name;
    SqlType type;
} ColumnDefinition;

typedef struct {
    const char *name;
    const ColumnDefinition *columns;
    int column_count;
} TableDefinition;

extern const TableDefinition SQL_CATALOG[];
extern const int SQL_CATALOG_COUNT;

void print_catalog(void);
int analyze_query(const Query *query, DiagnosticList *diagnostics);
const char *sql_type_name(SqlType type);
int sql_type_is_numeric(SqlType type);

char *duplicate_string(const char *value);
char *lowercase_string(const char *value);
void *allocate_memory(size_t size);
void *resize_memory(void *pointer, size_t size);

Expression *expression_literal_integer(long long value, int line, int column);
Expression *expression_literal_float(double value, int line, int column);
Expression *expression_literal_string(char *value, int line, int column);
Expression *expression_literal_null(int line, int column);
Expression *expression_literal_boolean(int value, int line, int column);
Expression *expression_column(char *qualifier, char *name, int line, int column);
Expression *expression_unary(UnaryOperator op, Expression *operand, int line, int column);
Expression *expression_binary(BinaryOperator op, Expression *left, Expression *right, int line, int column);
Expression *expression_like(Expression *value, Expression *pattern, int negated, int line, int column);
Expression *expression_in(Expression *value, int negated, int line, int column);
Expression *expression_between(Expression *value, Expression *lower, Expression *upper, int negated, int line, int column);
Expression *expression_is_null(Expression *value, int negated, int line, int column);
void expression_list_add(Expression *in_expression, Expression *item);

Query *query_create(int distinct, SelectList select_list, TableList table_list, Expression *where, OrderList order_list, int line, int column);
void select_list_add(SelectList *list, SelectItem item);
void table_list_add(TableList *list, TableReference item);
void order_list_add(OrderList *list, OrderItem item);

#endif
