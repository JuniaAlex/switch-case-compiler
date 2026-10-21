#include "sqlmini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *effective_name;
    const char *source_name;
    const TableDefinition *definition;
    int line;
    int column;
} ResolvedTable;

typedef struct {
    ResolvedTable *tables;
    int count;
    int capacity;
} ResolvedTableList;

typedef struct {
    ResolvedTableList from_tables;
    DiagnosticList *diagnostics;
} SemanticContext;

static const TableDefinition *find_table(const char *name)
{
    int index;
    for (index = 0; index < SQL_CATALOG_COUNT; index++) {
        if (strcmp(SQL_CATALOG[index].name, name) == 0) {
            return &SQL_CATALOG[index];
        }
    }
    return NULL;
}

static const ColumnDefinition *find_column(const TableDefinition *table, const char *name)
{
    int index;
    for (index = 0; index < table->column_count; index++) {
        if (strcmp(table->columns[index].name, name) == 0) {
            return &table->columns[index];
        }
    }
    return NULL;
}

static void resolved_tables_add(ResolvedTableList *list, ResolvedTable table)
{
    if (list->count == list->capacity) {
        int new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        list->tables = (ResolvedTable *)resize_memory(
            list->tables, (size_t)new_capacity * sizeof(ResolvedTable));
        list->capacity = new_capacity;
    }
    list->tables[list->count++] = table;
}

static ResolvedTable *find_resolved_table(SemanticContext *context, const char *name)
{
    int index;
    for (index = 0; index < context->from_tables.count; index++) {
        if (strcmp(context->from_tables.tables[index].effective_name, name) == 0) {
            return &context->from_tables.tables[index];
        }
    }
    return NULL;
}

static SqlType resolve_column(SemanticContext *context, const Expression *expression)
{
    if (expression->as.column.qualifier != NULL) {
        ResolvedTable *table = find_resolved_table(context, expression->as.column.qualifier);
        const ColumnDefinition *column;
        if (table == NULL) {
            diag_add(context->diagnostics, "S001", expression->line, expression->column,
                     "unknown table or alias '%s' in column reference '%s.%s'",
                     expression->as.column.qualifier,
                     expression->as.column.qualifier,
                     expression->as.column.name);
            return SQL_TYPE_UNKNOWN;
        }
        column = find_column(table->definition, expression->as.column.name);
        if (column == NULL) {
            diag_add(context->diagnostics, "S003", expression->line, expression->column,
                     "column '%s' does not exist in table '%s'",
                     expression->as.column.name,
                     table->source_name);
            return SQL_TYPE_UNKNOWN;
        }
        return column->type;
    }

    {
        const ColumnDefinition *matched_column = NULL;
        const TableDefinition *matched_table = NULL;
        int matches = 0;
        int index;
        char locations[256];
        size_t used = 0;

        locations[0] = '\0';
        for (index = 0; index < context->from_tables.count; index++) {
            ResolvedTable *table = &context->from_tables.tables[index];
            const ColumnDefinition *column = find_column(table->definition, expression->as.column.name);
            if (column != NULL) {
                int written;
                matches++;
                matched_column = column;
                matched_table = table->definition;
                written = snprintf(locations + used, sizeof(locations) - used, "%s%s.%s",
                                   used == 0 ? "" : ", ",
                                   table->effective_name,
                                   expression->as.column.name);
                if (written > 0 && (size_t)written < sizeof(locations) - used) {
                    used += (size_t)written;
                }
            }
        }

        if (matches == 0) {
            diag_add(context->diagnostics, "S003", expression->line, expression->column,
                     "unknown column '%s'", expression->as.column.name);
            return SQL_TYPE_UNKNOWN;
        }
        if (matches > 1) {
            diag_add(context->diagnostics, "S004", expression->line, expression->column,
                     "ambiguous column '%s' (matches %s)",
                     expression->as.column.name, locations);
            return SQL_TYPE_UNKNOWN;
        }
        (void)matched_table;
        return matched_column->type;
    }
}

static int types_compatible(SqlType left, SqlType right)
{
    if (left == SQL_TYPE_UNKNOWN || right == SQL_TYPE_UNKNOWN) {
        return 1;
    }
    if (left == SQL_TYPE_NULL || right == SQL_TYPE_NULL) {
        return 1;
    }
    if (left == right) {
        return 1;
    }
    if (sql_type_is_numeric(left) && sql_type_is_numeric(right)) {
        return 1;
    }
    return 0;
}

static SqlType check_expression(SemanticContext *context, const Expression *expression);

static SqlType require_numeric(SemanticContext *context, const Expression *expression, SqlType type, const char *operator_name)
{
    if (type == SQL_TYPE_UNKNOWN || type == SQL_TYPE_NULL) {
        return type;
    }
    if (!sql_type_is_numeric(type)) {
        diag_add(context->diagnostics, "S005", expression->line, expression->column,
                 "%s requires numeric operands, found %s",
                 operator_name, sql_type_name(type));
        return SQL_TYPE_UNKNOWN;
    }
    return type;
}

static SqlType require_boolean(SemanticContext *context, const Expression *expression, SqlType type, const char *operator_name)
{
    if (type == SQL_TYPE_UNKNOWN || type == SQL_TYPE_NULL) {
        return type;
    }
    if (type != SQL_TYPE_BOOLEAN) {
        diag_add(context->diagnostics, "S005", expression->line, expression->column,
                 "%s requires boolean operands, found %s",
                 operator_name, sql_type_name(type));
        return SQL_TYPE_UNKNOWN;
    }
    return type;
}

static SqlType require_text(SemanticContext *context, const Expression *expression, SqlType type, const char *operator_name)
{
    if (type == SQL_TYPE_UNKNOWN || type == SQL_TYPE_NULL) {
        return type;
    }
    if (type != SQL_TYPE_TEXT) {
        diag_add(context->diagnostics, "S005", expression->line, expression->column,
                 "%s requires text operands, found %s",
                 operator_name, sql_type_name(type));
        return SQL_TYPE_UNKNOWN;
    }
    return type;
}

static SqlType check_expression(SemanticContext *context, const Expression *expression)
{
    if (expression == NULL) {
        return SQL_TYPE_UNKNOWN;
    }

    switch (expression->kind) {
        case EXPRESSION_LITERAL:
            switch (expression->as.literal.kind) {
                case LITERAL_INTEGER: return SQL_TYPE_INTEGER;
                case LITERAL_FLOAT: return SQL_TYPE_FLOAT;
                case LITERAL_STRING: return SQL_TYPE_TEXT;
                case LITERAL_NULL: return SQL_TYPE_NULL;
                case LITERAL_BOOLEAN: return SQL_TYPE_BOOLEAN;
                default: return SQL_TYPE_UNKNOWN;
            }

        case EXPRESSION_COLUMN:
            return resolve_column(context, expression);

        case EXPRESSION_UNARY: {
            SqlType operand = check_expression(context, expression->as.unary.operand);
            if (expression->as.unary.op == UNARY_NOT) {
                require_boolean(context, expression, operand, "NOT");
                return SQL_TYPE_BOOLEAN;
            }
            require_numeric(context, expression, operand,
                            expression->as.unary.op == UNARY_MINUS ? "unary minus" : "unary plus");
            return operand;
        }

        case EXPRESSION_BINARY: {
            SqlType left = check_expression(context, expression->as.binary.left);
            SqlType right = check_expression(context, expression->as.binary.right);
            BinaryOperator op = expression->as.binary.op;

            if (op == BINARY_AND || op == BINARY_OR) {
                require_boolean(context, expression, left, op == BINARY_AND ? "AND" : "OR");
                require_boolean(context, expression, right, op == BINARY_AND ? "AND" : "OR");
                return SQL_TYPE_BOOLEAN;
            }

            if (op == BINARY_ADD || op == BINARY_SUBTRACT || op == BINARY_MULTIPLY || op == BINARY_DIVIDE) {
                const char *name = op == BINARY_ADD ? "+" :
                                   op == BINARY_SUBTRACT ? "-" :
                                   op == BINARY_MULTIPLY ? "*" : "/";
                require_numeric(context, expression, left, name);
                require_numeric(context, expression, right, name);
                if (left == SQL_TYPE_UNKNOWN || right == SQL_TYPE_UNKNOWN) {
                    return SQL_TYPE_UNKNOWN;
                }
                if (left == SQL_TYPE_NULL || right == SQL_TYPE_NULL) {
                    return SQL_TYPE_NULL;
                }
                if (left == SQL_TYPE_FLOAT || right == SQL_TYPE_FLOAT) {
                    return SQL_TYPE_FLOAT;
                }
                return SQL_TYPE_INTEGER;
            }

            if (!types_compatible(left, right)) {
                diag_add(context->diagnostics, "S005", expression->line, expression->column,
                         "cannot compare %s with %s",
                         sql_type_name(left), sql_type_name(right));
            }
            return SQL_TYPE_BOOLEAN;
        }

        case EXPRESSION_LIKE: {
            SqlType value = check_expression(context, expression->as.like.value);
            SqlType pattern = check_expression(context, expression->as.like.pattern);
            require_text(context, expression, value, "LIKE");
            require_text(context, expression, pattern, "LIKE");
            return SQL_TYPE_BOOLEAN;
        }

        case EXPRESSION_IN: {
            SqlType value = check_expression(context, expression->as.in.value);
            int index;
            for (index = 0; index < expression->as.in.count; index++) {
                SqlType item = check_expression(context, expression->as.in.items[index]);
                if (!types_compatible(value, item)) {
                    diag_add(context->diagnostics, "S005", expression->as.in.items[index]->line,
                             expression->as.in.items[index]->column,
                             "IN list value of type %s is not compatible with %s",
                             sql_type_name(item), sql_type_name(value));
                }
            }
            return SQL_TYPE_BOOLEAN;
        }

        case EXPRESSION_BETWEEN: {
            SqlType value = check_expression(context, expression->as.between.value);
            SqlType lower = check_expression(context, expression->as.between.lower);
            SqlType upper = check_expression(context, expression->as.between.upper);
            if (!types_compatible(value, lower)) {
                diag_add(context->diagnostics, "S005", expression->line, expression->column,
                         "BETWEEN lower bound of type %s is not compatible with %s",
                         sql_type_name(lower), sql_type_name(value));
            }
            if (!types_compatible(value, upper)) {
                diag_add(context->diagnostics, "S005", expression->line, expression->column,
                         "BETWEEN upper bound of type %s is not compatible with %s",
                         sql_type_name(upper), sql_type_name(value));
            }
            return SQL_TYPE_BOOLEAN;
        }

        case EXPRESSION_IS_NULL:
            check_expression(context, expression->as.is_null.value);
            return SQL_TYPE_BOOLEAN;

        default:
            return SQL_TYPE_UNKNOWN;
    }
}

static void collect_from_tables(SemanticContext *context, const Query *query)
{
    int index;
    for (index = 0; index < query->table_list.count; index++) {
        const TableReference *reference = &query->table_list.items[index];
        const TableDefinition *definition = find_table(reference->name);
        const char *effective = reference->alias != NULL ? reference->alias : reference->name;
        int duplicate = 0;
        int previous;

        if (definition == NULL) {
            diag_add(context->diagnostics, "S001", reference->line, reference->column,
                     "unknown table '%s'", reference->name);
            continue;
        }

        for (previous = 0; previous < context->from_tables.count; previous++) {
            if (strcmp(context->from_tables.tables[previous].effective_name, effective) == 0) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) {
            diag_add(context->diagnostics, "S002", reference->line, reference->column,
                     "duplicate table name or alias '%s'", effective);
            continue;
        }

        {
            ResolvedTable table;
            table.effective_name = effective;
            table.source_name = reference->name;
            table.definition = definition;
            table.line = reference->line;
            table.column = reference->column;
            resolved_tables_add(&context->from_tables, table);
        }
    }
}

int analyze_query(const Query *query, DiagnosticList *diagnostics)
{
    SemanticContext context;
    SqlType *select_types;
    const char **select_aliases;
    int start_count = diagnostics->count;
    int index;

    context.from_tables.tables = NULL;
    context.from_tables.count = 0;
    context.from_tables.capacity = 0;
    context.diagnostics = diagnostics;

    collect_from_tables(&context, query);

    select_types = (SqlType *)allocate_memory((size_t)query->select_list.count * sizeof(SqlType));
    select_aliases = (const char **)allocate_memory((size_t)query->select_list.count * sizeof(char *));

    for (index = 0; index < query->select_list.count; index++) {
        const SelectItem *item = &query->select_list.items[index];
        select_aliases[index] = item->alias;
        if (item->star) {
            select_types[index] = SQL_TYPE_UNKNOWN;
        } else {
            select_types[index] = check_expression(&context, item->expression);
        }
        if (item->alias != NULL) {
            int previous;
            for (previous = 0; previous < index; previous++) {
                if (select_aliases[previous] != NULL &&
                    strcmp(select_aliases[previous], item->alias) == 0) {
                    diag_add(diagnostics, "S007", item->line, item->column,
                             "duplicate select alias '%s'", item->alias);
                    break;
                }
            }
        }
    }

    if (query->where != NULL) {
        SqlType where_type = check_expression(&context, query->where);
        if (where_type != SQL_TYPE_BOOLEAN && where_type != SQL_TYPE_UNKNOWN) {
            diag_add(diagnostics, "S006", query->where->line, query->where->column,
                     "WHERE clause must evaluate to BOOLEAN, found %s",
                     sql_type_name(where_type));
        }
    }

    for (index = 0; index < query->order_list.count; index++) {
        const OrderItem *item = &query->order_list.items[index];
        const Expression *expression = item->expression;

        if (expression->kind == EXPRESSION_LITERAL &&
            expression->as.literal.kind == LITERAL_INTEGER) {
            long long position = expression->as.literal.integer_value;
            if (position < 1 || position > query->select_list.count) {
                diag_add(diagnostics, "S008", expression->line, expression->column,
                         "ORDER BY position %lld is out of range (1..%d)",
                         position, query->select_list.count);
            }
            continue;
        }

        if (expression->kind == EXPRESSION_COLUMN &&
            expression->as.column.qualifier == NULL) {
            int alias_index;
            int found = 0;
            for (alias_index = 0; alias_index < query->select_list.count; alias_index++) {
                if (select_aliases[alias_index] != NULL &&
                    strcmp(select_aliases[alias_index], expression->as.column.name) == 0) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                continue;
            }
        }

        {
            SqlType order_type = check_expression(&context, expression);
            if (order_type == SQL_TYPE_BOOLEAN) {
                diag_add(diagnostics, "S005", expression->line, expression->column,
                         "ORDER BY expression must be scalar, found BOOLEAN");
            }
        }
    }

    free(select_types);
    free(select_aliases);
    free(context.from_tables.tables);

    return diagnostics->count - start_count;
}
