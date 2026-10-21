%{
#include "sqlmini.h"
#include "driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DiagnosticList *g_diagnostics = NULL;
Query *g_query = NULL;
int g_show_tokens = 0;

int yylex(void);
void yyerror(const char *message);

static ExprNode *expr_node_new(Expression *expression)
{
    ExprNode *node = (ExprNode *)malloc(sizeof(ExprNode));
    if (node == NULL) {
        fprintf(stderr, "fatal: out of memory\n");
        exit(3);
    }
    node->expression = expression;
    node->next = NULL;
    return node;
}

static void expr_node_append(ExprNode *head, Expression *expression)
{
    ExprNode *node = head;
    while (node->next != NULL) {
        node = node->next;
    }
    node->next = expr_node_new(expression);
}

static void expr_node_free(ExprNode *head)
{
    while (head != NULL) {
        ExprNode *next = head->next;
        free(head);
        head = next;
    }
}

void yyerror(const char *message)
{
    if (g_diagnostics != NULL) {
        diag_add(g_diagnostics, "P001", g_token_line, g_token_column, "%s", message);
    }
}
%}

%code requires {
#include "driver.h"
}

%define parse.error verbose
%locations
%start query
%expect 0

%union {
    char *text;
    long long integer;
    double number;
    Expression *expr;
    ExprNode *expr_node;
    SelectItem select_item;
    SelectList select_list;
    TableReference table_ref;
    TableList table_list;
    OrderItem order_item;
    OrderList order_list;
    int flag;
}

%token <text> IDENT STRING
%token <integer> INT_LITERAL
%token <number> FLOAT_LITERAL
%token KW_SELECT KW_DISTINCT KW_FROM KW_WHERE KW_ORDER KW_BY KW_ASC KW_DESC
%token KW_AND KW_OR KW_NOT KW_AS KW_IN KW_BETWEEN KW_LIKE KW_IS KW_NULL KW_TRUE KW_FALSE
%token STAR PLUS MINUS SLASH EQ NE LT LE GT GE COMMA DOT LPAREN RPAREN SEMI

%type <flag> opt_distinct
%type <expr> opt_where
%type <order_list> opt_order
%type <select_list> select_list
%type <select_item> select_item
%type <table_list> table_list
%type <table_ref> table_ref
%type <order_list> order_list
%type <order_item> order_item
%type <expr> expr or_expr and_expr not_expr predicate additive multiplicative unary primary literal
%type <expr_node> literal_list

%%

query:
    KW_SELECT opt_distinct select_list KW_FROM table_list opt_where opt_order
    {
        g_query = query_create($2, $3, $5, $6, $7, @1.first_line, @1.first_column);
    }
    opt_semi
    ;

opt_distinct:
    %empty
    { $$ = 0; }
  | KW_DISTINCT
    { $$ = 1; }
    ;

select_list:
    select_item
    {
        SelectList list;
        memset(&list, 0, sizeof(list));
        select_list_add(&list, $1);
        $$ = list;
    }
  | select_list COMMA select_item
    {
        select_list_add(&$1, $3);
        $$ = $1;
    }
    ;

select_item:
    STAR
    {
        SelectItem item;
        memset(&item, 0, sizeof(item));
        item.star = 1;
        item.line = @1.first_line;
        item.column = @1.first_column;
        $$ = item;
    }
  | expr
    {
        SelectItem item;
        memset(&item, 0, sizeof(item));
        item.expression = $1;
        item.line = @1.first_line;
        item.column = @1.first_column;
        $$ = item;
    }
  | expr IDENT
    {
        SelectItem item;
        memset(&item, 0, sizeof(item));
        item.expression = $1;
        item.alias = $2;
        item.line = @1.first_line;
        item.column = @1.first_column;
        $$ = item;
    }
  | expr KW_AS IDENT
    {
        SelectItem item;
        memset(&item, 0, sizeof(item));
        item.expression = $1;
        item.alias = $3;
        item.line = @1.first_line;
        item.column = @1.first_column;
        $$ = item;
    }
    ;

table_list:
    table_ref
    {
        TableList list;
        memset(&list, 0, sizeof(list));
        table_list_add(&list, $1);
        $$ = list;
    }
  | table_list COMMA table_ref
    {
        table_list_add(&$1, $3);
        $$ = $1;
    }
    ;

table_ref:
    IDENT
    {
        TableReference table;
        memset(&table, 0, sizeof(table));
        table.name = $1;
        table.line = @1.first_line;
        table.column = @1.first_column;
        $$ = table;
    }
  | IDENT IDENT
    {
        TableReference table;
        memset(&table, 0, sizeof(table));
        table.name = $1;
        table.alias = $2;
        table.line = @1.first_line;
        table.column = @1.first_column;
        $$ = table;
    }
  | IDENT KW_AS IDENT
    {
        TableReference table;
        memset(&table, 0, sizeof(table));
        table.name = $1;
        table.alias = $3;
        table.line = @1.first_line;
        table.column = @1.first_column;
        $$ = table;
    }
    ;

opt_where:
    %empty
    { $$ = NULL; }
  | KW_WHERE expr
    { $$ = $2; }
    ;

opt_order:
    %empty
    {
        OrderList list;
        memset(&list, 0, sizeof(list));
        $$ = list;
    }
  | KW_ORDER KW_BY order_list
    { $$ = $3; }
    ;

order_list:
    order_item
    {
        OrderList list;
        memset(&list, 0, sizeof(list));
        order_list_add(&list, $1);
        $$ = list;
    }
  | order_list COMMA order_item
    {
        order_list_add(&$1, $3);
        $$ = $1;
    }
    ;

order_item:
    expr
    {
        OrderItem item;
        memset(&item, 0, sizeof(item));
        item.expression = $1;
        item.ascending = 1;
        item.line = @1.first_line;
        item.column = @1.first_column;
        $$ = item;
    }
  | expr KW_ASC
    {
        OrderItem item;
        memset(&item, 0, sizeof(item));
        item.expression = $1;
        item.ascending = 1;
        item.line = @1.first_line;
        item.column = @1.first_column;
        $$ = item;
    }
  | expr KW_DESC
    {
        OrderItem item;
        memset(&item, 0, sizeof(item));
        item.expression = $1;
        item.ascending = 0;
        item.line = @1.first_line;
        item.column = @1.first_column;
        $$ = item;
    }
    ;

opt_semi:
    %empty
  | SEMI
    ;

expr:
    or_expr
    { $$ = $1; }
    ;

or_expr:
    and_expr
    { $$ = $1; }
  | or_expr KW_OR and_expr
    { $$ = expression_binary(BINARY_OR, $1, $3, @2.first_line, @2.first_column); }
    ;

and_expr:
    not_expr
    { $$ = $1; }
  | and_expr KW_AND not_expr
    { $$ = expression_binary(BINARY_AND, $1, $3, @2.first_line, @2.first_column); }
    ;

not_expr:
    predicate
    { $$ = $1; }
  | KW_NOT not_expr
    { $$ = expression_unary(UNARY_NOT, $2, @1.first_line, @1.first_column); }
    ;

predicate:
    additive
    { $$ = $1; }
  | additive EQ additive
    { $$ = expression_binary(BINARY_EQUAL, $1, $3, @2.first_line, @2.first_column); }
  | additive NE additive
    { $$ = expression_binary(BINARY_NOT_EQUAL, $1, $3, @2.first_line, @2.first_column); }
  | additive LT additive
    { $$ = expression_binary(BINARY_LESS, $1, $3, @2.first_line, @2.first_column); }
  | additive LE additive
    { $$ = expression_binary(BINARY_LESS_EQUAL, $1, $3, @2.first_line, @2.first_column); }
  | additive GT additive
    { $$ = expression_binary(BINARY_GREATER, $1, $3, @2.first_line, @2.first_column); }
  | additive GE additive
    { $$ = expression_binary(BINARY_GREATER_EQUAL, $1, $3, @2.first_line, @2.first_column); }
  | additive KW_LIKE additive
    { $$ = expression_like($1, $3, 0, @2.first_line, @2.first_column); }
  | additive KW_NOT KW_LIKE additive
    { $$ = expression_like($1, $4, 1, @2.first_line, @2.first_column); }
  | additive KW_IN LPAREN literal_list RPAREN
    {
        Expression *in_expression = expression_in($1, 0, @2.first_line, @2.first_column);
        ExprNode *node;
        for (node = $4; node != NULL; node = node->next) {
            expression_list_add(in_expression, node->expression);
        }
        expr_node_free($4);
        $$ = in_expression;
    }
  | additive KW_NOT KW_IN LPAREN literal_list RPAREN
    {
        Expression *in_expression = expression_in($1, 1, @2.first_line, @2.first_column);
        ExprNode *node;
        for (node = $5; node != NULL; node = node->next) {
            expression_list_add(in_expression, node->expression);
        }
        expr_node_free($5);
        $$ = in_expression;
    }
  | additive KW_BETWEEN additive KW_AND additive
    { $$ = expression_between($1, $3, $5, 0, @2.first_line, @2.first_column); }
  | additive KW_NOT KW_BETWEEN additive KW_AND additive
    { $$ = expression_between($1, $4, $6, 1, @2.first_line, @2.first_column); }
  | additive KW_IS KW_NULL
    { $$ = expression_is_null($1, 0, @2.first_line, @2.first_column); }
  | additive KW_IS KW_NOT KW_NULL
    { $$ = expression_is_null($1, 1, @2.first_line, @2.first_column); }
    ;

additive:
    multiplicative
    { $$ = $1; }
  | additive PLUS multiplicative
    { $$ = expression_binary(BINARY_ADD, $1, $3, @2.first_line, @2.first_column); }
  | additive MINUS multiplicative
    { $$ = expression_binary(BINARY_SUBTRACT, $1, $3, @2.first_line, @2.first_column); }
    ;

multiplicative:
    unary
    { $$ = $1; }
  | multiplicative STAR unary
    { $$ = expression_binary(BINARY_MULTIPLY, $1, $3, @2.first_line, @2.first_column); }
  | multiplicative SLASH unary
    { $$ = expression_binary(BINARY_DIVIDE, $1, $3, @2.first_line, @2.first_column); }
    ;

unary:
    primary
    { $$ = $1; }
  | PLUS unary
    { $$ = expression_unary(UNARY_PLUS, $2, @1.first_line, @1.first_column); }
  | MINUS unary
    { $$ = expression_unary(UNARY_MINUS, $2, @1.first_line, @1.first_column); }
    ;

primary:
    literal
    { $$ = $1; }
  | IDENT
    { $$ = expression_column(NULL, $1, @1.first_line, @1.first_column); }
  | IDENT DOT IDENT
    { $$ = expression_column($1, $3, @1.first_line, @1.first_column); }
  | LPAREN expr RPAREN
    { $$ = $2; }
    ;

literal:
    INT_LITERAL
    { $$ = expression_literal_integer($1, @1.first_line, @1.first_column); }
  | FLOAT_LITERAL
    { $$ = expression_literal_float($1, @1.first_line, @1.first_column); }
  | STRING
    { $$ = expression_literal_string($1, @1.first_line, @1.first_column); }
  | KW_NULL
    { $$ = expression_literal_null(@1.first_line, @1.first_column); }
  | KW_TRUE
    { $$ = expression_literal_boolean(1, @1.first_line, @1.first_column); }
  | KW_FALSE
    { $$ = expression_literal_boolean(0, @1.first_line, @1.first_column); }
    ;

literal_list:
    literal
    { $$ = expr_node_new($1); }
  | literal_list COMMA literal
    {
        expr_node_append($1, $3);
        $$ = $1;
    }
    ;

%%
