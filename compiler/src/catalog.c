#include "sqlmini.h"

#include <stdio.h>

static const ColumnDefinition STUDENT_COLUMNS[] = {
    {"student_id", SQL_TYPE_INTEGER},
    {"name", SQL_TYPE_TEXT},
    {"age", SQL_TYPE_INTEGER},
    {"department", SQL_TYPE_TEXT},
    {"cgpa", SQL_TYPE_FLOAT},
    {"city", SQL_TYPE_TEXT}
};

static const ColumnDefinition COURSE_COLUMNS[] = {
    {"course_id", SQL_TYPE_INTEGER},
    {"title", SQL_TYPE_TEXT},
    {"credits", SQL_TYPE_INTEGER},
    {"department", SQL_TYPE_TEXT}
};

static const ColumnDefinition ENROLLMENT_COLUMNS[] = {
    {"enrollment_id", SQL_TYPE_INTEGER},
    {"student_id", SQL_TYPE_INTEGER},
    {"course_id", SQL_TYPE_INTEGER},
    {"grade", SQL_TYPE_TEXT},
    {"semester", SQL_TYPE_INTEGER}
};

static const ColumnDefinition DEPARTMENT_COLUMNS[] = {
    {"department", SQL_TYPE_TEXT},
    {"hod", SQL_TYPE_TEXT},
    {"building", SQL_TYPE_TEXT},
    {"budget", SQL_TYPE_FLOAT}
};

const TableDefinition SQL_CATALOG[] = {
    {"students", STUDENT_COLUMNS, (int)(sizeof(STUDENT_COLUMNS) / sizeof(STUDENT_COLUMNS[0]))},
    {"courses", COURSE_COLUMNS, (int)(sizeof(COURSE_COLUMNS) / sizeof(COURSE_COLUMNS[0]))},
    {"enrollments", ENROLLMENT_COLUMNS, (int)(sizeof(ENROLLMENT_COLUMNS) / sizeof(ENROLLMENT_COLUMNS[0]))},
    {"departments", DEPARTMENT_COLUMNS, (int)(sizeof(DEPARTMENT_COLUMNS) / sizeof(DEPARTMENT_COLUMNS[0]))}
};

const int SQL_CATALOG_COUNT = (int)(sizeof(SQL_CATALOG) / sizeof(SQL_CATALOG[0]));

const char *sql_type_name(SqlType type)
{
    switch (type) {
        case SQL_TYPE_INTEGER: return "INTEGER";
        case SQL_TYPE_FLOAT: return "FLOAT";
        case SQL_TYPE_TEXT: return "TEXT";
        case SQL_TYPE_BOOLEAN: return "BOOLEAN";
        case SQL_TYPE_NULL: return "NULL";
        default: return "UNKNOWN";
    }
}

int sql_type_is_numeric(SqlType type)
{
    return type == SQL_TYPE_INTEGER || type == SQL_TYPE_FLOAT;
}

void print_catalog(void)
{
    int table_index;
    int column_index;
    for (table_index = 0; table_index < SQL_CATALOG_COUNT; table_index++) {
        const TableDefinition *table = &SQL_CATALOG[table_index];
        printf("Table %s\n", table->name);
        for (column_index = 0; column_index < table->column_count; column_index++) {
            const ColumnDefinition *column = &table->columns[column_index];
            printf("  %-16s %s\n", column->name, sql_type_name(column->type));
        }
    }
}
