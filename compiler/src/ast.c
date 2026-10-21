#include "sqlmini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Expression *expression_new(ExpressionKind kind, int line, int column)
{
    Expression *expression = (Expression *)allocate_memory(sizeof(Expression));
    memset(expression, 0, sizeof(Expression));
    expression->kind = kind;
    expression->line = line;
    expression->column = column;
    return expression;
}

Expression *expression_literal_integer(long long value, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_LITERAL, line, column);
    expression->as.literal.kind = LITERAL_INTEGER;
    expression->as.literal.integer_value = value;
    return expression;
}

Expression *expression_literal_float(double value, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_LITERAL, line, column);
    expression->as.literal.kind = LITERAL_FLOAT;
    expression->as.literal.float_value = value;
    return expression;
}

Expression *expression_literal_string(char *value, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_LITERAL, line, column);
    expression->as.literal.kind = LITERAL_STRING;
    expression->as.literal.text = value;
    return expression;
}

Expression *expression_literal_null(int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_LITERAL, line, column);
    expression->as.literal.kind = LITERAL_NULL;
    return expression;
}

Expression *expression_literal_boolean(int value, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_LITERAL, line, column);
    expression->as.literal.kind = LITERAL_BOOLEAN;
    expression->as.literal.boolean_value = value;
    return expression;
}

Expression *expression_column(char *qualifier, char *name, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_COLUMN, line, column);
    expression->as.column.qualifier = qualifier;
    expression->as.column.name = name;
    return expression;
}

Expression *expression_unary(UnaryOperator op, Expression *operand, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_UNARY, line, column);
    expression->as.unary.op = op;
    expression->as.unary.operand = operand;
    return expression;
}

Expression *expression_binary(BinaryOperator op, Expression *left, Expression *right, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_BINARY, line, column);
    expression->as.binary.op = op;
    expression->as.binary.left = left;
    expression->as.binary.right = right;
    return expression;
}

Expression *expression_like(Expression *value, Expression *pattern, int negated, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_LIKE, line, column);
    expression->as.like.value = value;
    expression->as.like.pattern = pattern;
    expression->as.like.negated = negated;
    return expression;
}

Expression *expression_in(Expression *value, int negated, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_IN, line, column);
    expression->as.in.value = value;
    expression->as.in.negated = negated;
    return expression;
}

Expression *expression_between(Expression *value, Expression *lower, Expression *upper, int negated, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_BETWEEN, line, column);
    expression->as.between.value = value;
    expression->as.between.lower = lower;
    expression->as.between.upper = upper;
    expression->as.between.negated = negated;
    return expression;
}

Expression *expression_is_null(Expression *value, int negated, int line, int column)
{
    Expression *expression = expression_new(EXPRESSION_IS_NULL, line, column);
    expression->as.is_null.value = value;
    expression->as.is_null.negated = negated;
    return expression;
}

void expression_list_add(Expression *in_expression, Expression *item)
{
    if (in_expression->as.in.count == in_expression->as.in.capacity) {
        int new_capacity = in_expression->as.in.capacity == 0 ? 4 : in_expression->as.in.capacity * 2;
        in_expression->as.in.items = (Expression **)resize_memory(
            in_expression->as.in.items,
            (size_t)new_capacity * sizeof(Expression *));
        in_expression->as.in.capacity = new_capacity;
    }
    in_expression->as.in.items[in_expression->as.in.count++] = item;
}

void select_list_add(SelectList *list, SelectItem item)
{
    if (list->count == list->capacity) {
        int new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        list->items = (SelectItem *)resize_memory(list->items, (size_t)new_capacity * sizeof(SelectItem));
        list->capacity = new_capacity;
    }
    list->items[list->count++] = item;
}

void table_list_add(TableList *list, TableReference item)
{
    if (list->count == list->capacity) {
        int new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        list->items = (TableReference *)resize_memory(list->items, (size_t)new_capacity * sizeof(TableReference));
        list->capacity = new_capacity;
    }
    list->items[list->count++] = item;
}

void order_list_add(OrderList *list, OrderItem item)
{
    if (list->count == list->capacity) {
        int new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        list->items = (OrderItem *)resize_memory(list->items, (size_t)new_capacity * sizeof(OrderItem));
        list->capacity = new_capacity;
    }
    list->items[list->count++] = item;
}

Query *query_create(int distinct, SelectList select_list, TableList table_list, Expression *where, OrderList order_list, int line, int column)
{
    Query *query = (Query *)allocate_memory(sizeof(Query));
    memset(query, 0, sizeof(Query));
    query->distinct = distinct;
    query->select_list = select_list;
    query->table_list = table_list;
    query->where = where;
    query->order_list = order_list;
    query->line = line;
    query->column = column;
    return query;
}

static void expression_free(Expression *expression)
{
    if (expression == NULL) {
        return;
    }
    switch (expression->kind) {
        case EXPRESSION_LITERAL:
            free(expression->as.literal.text);
            break;
        case EXPRESSION_COLUMN:
            free(expression->as.column.qualifier);
            free(expression->as.column.name);
            break;
        case EXPRESSION_UNARY:
            expression_free(expression->as.unary.operand);
            break;
        case EXPRESSION_BINARY:
            expression_free(expression->as.binary.left);
            expression_free(expression->as.binary.right);
            break;
        case EXPRESSION_LIKE:
            expression_free(expression->as.like.value);
            expression_free(expression->as.like.pattern);
            break;
        case EXPRESSION_IN: {
            int index;
            expression_free(expression->as.in.value);
            for (index = 0; index < expression->as.in.count; index++) {
                expression_free(expression->as.in.items[index]);
            }
            free(expression->as.in.items);
            break;
        }
        case EXPRESSION_BETWEEN:
            expression_free(expression->as.between.value);
            expression_free(expression->as.between.lower);
            expression_free(expression->as.between.upper);
            break;
        case EXPRESSION_IS_NULL:
            expression_free(expression->as.is_null.value);
            break;
        default:
            break;
    }
    free(expression);
}

void query_free(Query *query)
{
    int index;
    if (query == NULL) {
        return;
    }
    for (index = 0; index < query->select_list.count; index++) {
        free(query->select_list.items[index].alias);
        expression_free(query->select_list.items[index].expression);
    }
    free(query->select_list.items);
    for (index = 0; index < query->table_list.count; index++) {
        free(query->table_list.items[index].name);
        free(query->table_list.items[index].alias);
    }
    free(query->table_list.items);
    expression_free(query->where);
    for (index = 0; index < query->order_list.count; index++) {
        expression_free(query->order_list.items[index].expression);
    }
    free(query->order_list.items);
    free(query);
}

static void print_indent(int depth)
{
    int index;
    for (index = 0; index < depth; index++) {
        printf("  ");
    }
}

static const char *unary_name(UnaryOperator op)
{
    switch (op) {
        case UNARY_PLUS: return "+";
        case UNARY_MINUS: return "-";
        case UNARY_NOT: return "NOT";
        default: return "?";
    }
}

static const char *binary_name(BinaryOperator op)
{
    switch (op) {
        case BINARY_ADD: return "+";
        case BINARY_SUBTRACT: return "-";
        case BINARY_MULTIPLY: return "*";
        case BINARY_DIVIDE: return "/";
        case BINARY_EQUAL: return "=";
        case BINARY_NOT_EQUAL: return "<>";
        case BINARY_LESS: return "<";
        case BINARY_LESS_EQUAL: return "<=";
        case BINARY_GREATER: return ">";
        case BINARY_GREATER_EQUAL: return ">=";
        case BINARY_AND: return "AND";
        case BINARY_OR: return "OR";
        default: return "?";
    }
}

static void print_expression(const Expression *expression, int depth);

static void print_literal(const Expression *expression, int depth)
{
    const char *kind;
    switch (expression->as.literal.kind) {
        case LITERAL_INTEGER:
            kind = "integer";
            print_indent(depth);
            printf("Literal [%d:%d] %s %lld\n", expression->line, expression->column,
                   kind, expression->as.literal.integer_value);
            return;
        case LITERAL_FLOAT:
            kind = "float";
            print_indent(depth);
            printf("Literal [%d:%d] %s %g\n", expression->line, expression->column,
                   kind, expression->as.literal.float_value);
            return;
        case LITERAL_STRING:
            kind = "string";
            print_indent(depth);
            printf("Literal [%d:%d] %s '%s'\n", expression->line, expression->column,
                   kind, expression->as.literal.text == NULL ? "" : expression->as.literal.text);
            return;
        case LITERAL_NULL:
            print_indent(depth);
            printf("Literal [%d:%d] null\n", expression->line, expression->column);
            return;
        case LITERAL_BOOLEAN:
            print_indent(depth);
            printf("Literal [%d:%d] boolean %s\n", expression->line, expression->column,
                   expression->as.literal.boolean_value ? "TRUE" : "FALSE");
            return;
        default:
            return;
    }
}

static void print_expression(const Expression *expression, int depth)
{
    if (expression == NULL) {
        return;
    }
    switch (expression->kind) {
        case EXPRESSION_LITERAL:
            print_literal(expression, depth);
            break;
        case EXPRESSION_COLUMN:
            print_indent(depth);
            if (expression->as.column.qualifier != NULL) {
                printf("Column [%d:%d] %s.%s\n", expression->line, expression->column,
                       expression->as.column.qualifier, expression->as.column.name);
            } else {
                printf("Column [%d:%d] %s\n", expression->line, expression->column,
                       expression->as.column.name);
            }
            break;
        case EXPRESSION_UNARY:
            print_indent(depth);
            printf("Unary [%d:%d] %s\n", expression->line, expression->column,
                   unary_name(expression->as.unary.op));
            print_expression(expression->as.unary.operand, depth + 1);
            break;
        case EXPRESSION_BINARY:
            print_indent(depth);
            printf("Binary [%d:%d] %s\n", expression->line, expression->column,
                   binary_name(expression->as.binary.op));
            print_expression(expression->as.binary.left, depth + 1);
            print_expression(expression->as.binary.right, depth + 1);
            break;
        case EXPRESSION_LIKE:
            print_indent(depth);
            printf("Like [%d:%d]%s\n", expression->line, expression->column,
                   expression->as.like.negated ? " NOT" : "");
            print_indent(depth + 1);
            printf("Value\n");
            print_expression(expression->as.like.value, depth + 2);
            print_indent(depth + 1);
            printf("Pattern\n");
            print_expression(expression->as.like.pattern, depth + 2);
            break;
        case EXPRESSION_IN: {
            int index;
            print_indent(depth);
            printf("In [%d:%d]%s\n", expression->line, expression->column,
                   expression->as.in.negated ? " NOT" : "");
            print_indent(depth + 1);
            printf("Value\n");
            print_expression(expression->as.in.value, depth + 2);
            print_indent(depth + 1);
            printf("Items (%d)\n", expression->as.in.count);
            for (index = 0; index < expression->as.in.count; index++) {
                print_expression(expression->as.in.items[index], depth + 2);
            }
            break;
        }
        case EXPRESSION_BETWEEN:
            print_indent(depth);
            printf("Between [%d:%d]%s\n", expression->line, expression->column,
                   expression->as.between.negated ? " NOT" : "");
            print_indent(depth + 1);
            printf("Value\n");
            print_expression(expression->as.between.value, depth + 2);
            print_indent(depth + 1);
            printf("Lower\n");
            print_expression(expression->as.between.lower, depth + 2);
            print_indent(depth + 1);
            printf("Upper\n");
            print_expression(expression->as.between.upper, depth + 2);
            break;
        case EXPRESSION_IS_NULL:
            print_indent(depth);
            printf("IsNull [%d:%d]%s\n", expression->line, expression->column,
                   expression->as.is_null.negated ? " NOT" : "");
            print_expression(expression->as.is_null.value, depth + 1);
            break;
        default:
            break;
    }
}

void print_ast(const Query *query)
{
    int index;
    if (query == NULL) {
        return;
    }
    printf("Query [%d:%d]%s\n", query->line, query->column,
           query->distinct ? " DISTINCT" : "");
    print_indent(1);
    printf("SelectList (%d item%s)\n", query->select_list.count,
           query->select_list.count == 1 ? "" : "s");
    for (index = 0; index < query->select_list.count; index++) {
        const SelectItem *item = &query->select_list.items[index];
        print_indent(2);
        if (item->star) {
            printf("SelectItem [%d:%d] *\n", item->line, item->column);
        } else if (item->alias != NULL) {
            printf("SelectItem [%d:%d] alias=%s\n", item->line, item->column, item->alias);
            print_expression(item->expression, 3);
        } else {
            printf("SelectItem [%d:%d]\n", item->line, item->column);
            print_expression(item->expression, 3);
        }
    }
    print_indent(1);
    printf("FromList (%d table%s)\n", query->table_list.count,
           query->table_list.count == 1 ? "" : "s");
    for (index = 0; index < query->table_list.count; index++) {
        const TableReference *table = &query->table_list.items[index];
        print_indent(2);
        if (table->alias != NULL) {
            printf("TableRef [%d:%d] %s AS %s\n", table->line, table->column,
                   table->name, table->alias);
        } else {
            printf("TableRef [%d:%d] %s\n", table->line, table->column, table->name);
        }
    }
    if (query->where != NULL) {
        print_indent(1);
        printf("Where\n");
        print_expression(query->where, 2);
    }
    if (query->order_list.count > 0) {
        print_indent(1);
        printf("OrderList (%d item%s)\n", query->order_list.count,
               query->order_list.count == 1 ? "" : "s");
        for (index = 0; index < query->order_list.count; index++) {
            const OrderItem *item = &query->order_list.items[index];
            print_indent(2);
            printf("OrderItem [%d:%d] %s\n", item->line, item->column,
                   item->ascending ? "ASC" : "DESC");
            print_expression(item->expression, 3);
        }
    }
}
