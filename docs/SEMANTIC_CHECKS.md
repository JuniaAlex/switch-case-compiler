# Semantic Analysis

The semantic phase runs after a query has been lexed and parsed successfully. It resolves names against a fixed catalog, checks operator and expression types, validates aliases, and enforces the rules for `WHERE` and `ORDER BY`.

The rules are implemented once in the native compiler:

- `compiler/src/semantic.c`
- `compiler/src/catalog.c`
- `compiler/src/sqlmini.h`

The Python bridge and frontend consume the resulting diagnostics; they do not reimplement these rules.

## 1. Demo Catalog

| Table | Columns |
| --- | --- |
| `students` | `student_id INTEGER`, `name TEXT`, `age INTEGER`, `department TEXT`, `cgpa FLOAT`, `city TEXT` |
| `courses` | `course_id INTEGER`, `title TEXT`, `credits INTEGER`, `department TEXT` |
| `enrollments` | `enrollment_id INTEGER`, `student_id INTEGER`, `course_id INTEGER`, `grade TEXT`, `semester INTEGER` |
| `departments` | `department TEXT`, `hod TEXT`, `building TEXT`, `budget FLOAT` |

The catalog is static. The compiler does not inspect a live database schema.

## 2. Type System

| Type | Meaning |
| --- | --- |
| `INTEGER` | Integer literal or integer column |
| `FLOAT` | Floating-point literal or float column |
| `TEXT` | String literal or text column |
| `BOOLEAN` | Result of a predicate or boolean operator |
| `NULL` | The `NULL` literal |
| `UNKNOWN` | Error recovery value; suppresses cascading diagnostics |

Compatibility rules:

- `INTEGER` and `FLOAT` are compatible with each other for comparisons and arithmetic.
- `NULL` is compatible with every type for comparison purposes.
- `UNKNOWN` is compatible with everything so that one error does not produce a chain of follow-on errors.
- `TEXT` is not comparable with numeric types.
- `BOOLEAN` is not a numeric type.

## 3. Table and Alias Resolution

For each entry in `FROM`:

1. Look up the table name in the catalog.
2. If the table is unknown, emit `S001`.
3. Compute the effective name: the alias if present, otherwise the table name.
4. If the effective name is already in the query scope, emit `S002` and do not add the duplicate.
5. Otherwise add the table to the query scope.

Examples:

```sql
-- S001: unknown table
SELECT name FROM teachers;

-- S001: unknown alias in qualified column
SELECT s.name FROM students;

-- S002: duplicate effective name
SELECT * FROM students s, courses s;

-- S002: duplicate table without aliases
SELECT * FROM students, students;
```

A table alias may be written with or without `AS`:

```sql
FROM students s
FROM students AS s
```

## 4. Column Resolution

### 4.1 Qualified columns

For `qualifier.column`:

1. Resolve `qualifier` against the effective table names in the current `FROM` scope.
2. If the qualifier is not found, emit `S001`.
3. If the qualifier is found, look up the column in that table.
4. If the column does not exist, emit `S003`.

Examples:

```sql
-- Valid: s is an effective name
SELECT s.name FROM students s;

-- S001: x is not a table or alias in scope
SELECT x.name FROM students;

-- S003: salary is not a column of students
SELECT students.salary FROM students;
```

### 4.2 Unqualified columns

For a bare `column`:

1. Search every table in the `FROM` scope.
2. If no table contains the column, emit `S003`.
3. If exactly one table contains it, bind to that column and return its type.
4. If two or more tables contain it, emit `S004` and list the matching qualified names.

Examples:

```sql
-- Valid: only students has name
SELECT name FROM students;

-- S004: both students and courses have department
SELECT department FROM students, courses;

-- S003: salary is not present in any table in scope
SELECT name FROM students WHERE salary > 1000;
```

Ambiguity is resolved by the user, not by the compiler. A qualified reference such as `students.department` removes the ambiguity.

## 5. Expression Type Rules

### 5.1 Literals

| Expression | Type |
| --- | --- |
| `42` | `INTEGER` |
| `3.14` | `FLOAT` |
| `'Delhi'` | `TEXT` |
| `NULL` | `NULL` |
| `TRUE`, `FALSE` | `BOOLEAN` |

### 5.2 Arithmetic

Operators: `+`, `-`, `*`, `/`.

- Both operands must be numeric (`INTEGER` or `FLOAT`).
- A non-numeric operand produces `S005`.
- If either operand is `FLOAT`, the result is `FLOAT`.
- If both operands are `INTEGER`, the result is `INTEGER`.
- If either operand is `NULL`, the result is `NULL`.
- If an operand is `UNKNOWN`, the result is `UNKNOWN` and no extra diagnostic is produced.

Examples:

```sql
SELECT credits * 2 + 1 AS weighted FROM courses;       -- valid
SELECT name + 1 FROM students;                          -- S005
SELECT name / 2 FROM students;                          -- S005
```

### 5.3 Unary operators

- Unary `+` and `-` require a numeric operand.
- Unary `NOT` requires a boolean operand.
- `NOT` returns `BOOLEAN`.
- Unary numeric operators preserve the operand type.

Examples:

```sql
SELECT name FROM students WHERE age > -1;               -- valid
SELECT name FROM students WHERE NOT age = 18;           -- valid
SELECT name FROM students WHERE NOT age;                -- S005
```

### 5.4 Comparisons

Operators: `=`, `!=`, `<>`, `<`, `<=`, `>`, `>=`.

- Both operands must be type-compatible.
- Numeric-to-numeric comparisons are allowed, including mixed `INTEGER`/`FLOAT`.
- `NULL` is compatible with any type.
- A mismatch produces `S005`.
- The result is always `BOOLEAN`.

Examples:

```sql
SELECT name FROM students WHERE age >= 18;              -- valid
SELECT name FROM students WHERE age = 'eighteen';       -- S005
SELECT name FROM students WHERE cgpa > 7.5;             -- valid
```

### 5.5 Boolean operators

Operators: `AND`, `OR`.

- Both operands must be `BOOLEAN`.
- A non-boolean operand produces `S005`.
- The result is `BOOLEAN`.

Examples:

```sql
SELECT name FROM students
WHERE age >= 18 AND cgpa >= 7;                          -- valid

SELECT name FROM students
WHERE age AND TRUE;                                     -- S005
```

### 5.6 `LIKE` and `NOT LIKE`

- The value expression must be `TEXT`.
- The pattern expression must be `TEXT`.
- A mismatch produces `S005`.
- The result is `BOOLEAN`.

Examples:

```sql
SELECT name FROM students WHERE name LIKE 'A%';         -- valid
SELECT name FROM students WHERE name LIKE 10;            -- S005
```

### 5.7 `IN` and `NOT IN`

- The left value is checked normally.
- Each list item is checked normally.
- Every list item must be type-compatible with the left value.
- A mismatch produces `S005` for the offending item.
- The result is `BOOLEAN`.

Examples:

```sql
SELECT name FROM students WHERE age IN (18, 19);         -- valid
SELECT name FROM students WHERE age IN (1, 'x');         -- S005
SELECT name FROM students WHERE age IN (TRUE, FALSE);    -- S005
```

The grammar requires at least one literal in the list, so `IN ()` is a syntax error.

### 5.8 `BETWEEN` and `NOT BETWEEN`

- The value, lower bound, and upper bound are checked normally.
- The bounds must be type-compatible with the value.
- A mismatch produces `S005`.
- The result is `BOOLEAN`.

Examples:

```sql
SELECT name FROM students WHERE cgpa BETWEEN 7.5 AND 10; -- valid
SELECT name FROM students WHERE cgpa BETWEEN 'a' AND 'b'; -- S005
```

### 5.9 `IS NULL` and `IS NOT NULL`

- The operand may have any type.
- The result is `BOOLEAN`.

Examples:

```sql
SELECT name FROM students WHERE department IS NOT NULL;  -- valid
SELECT title FROM courses WHERE department IS NULL;      -- valid
```

## 6. `WHERE` Rule

A `WHERE` clause must evaluate to `BOOLEAN` or `UNKNOWN`.

- A boolean expression is accepted.
- A bare scalar expression such as `age` produces `S006`.
- `UNKNOWN` suppresses `S006` because an earlier error has already been reported.

Examples:

```sql
SELECT name FROM students WHERE age > 18;                -- valid
SELECT name FROM students WHERE TRUE;                   -- valid
SELECT name FROM students WHERE age;                    -- S006
```

## 7. `SELECT` Alias Rules

- A select item may have an alias with or without `AS`.
- Aliases are stored in select-list order.
- Two select items may not use the same alias; the second occurrence produces `S007`.
- Aliases are available to `ORDER BY` as a convenience, matching common SQL behavior.
- Aliases are not a general substitute for columns in `WHERE` or in arbitrary expressions.

Examples:

```sql
SELECT name AS student_name, age FROM students;          -- valid
SELECT name AS n, age AS n FROM students;                -- S007
SELECT cgpa * 2 AS double_cgpa FROM students
ORDER BY double_cgpa;                                    -- valid
```

## 8. `ORDER BY` Rules

Each order item is checked in this order:

1. If the expression is an integer literal, treat it as a one-based select-list position.
   - Values below `1` or above the number of select items produce `S008`.
   - The ordinal itself is not treated as a scalar value after validation.
2. If the expression is an unqualified column whose name matches a select alias, accept it as an alias reference.
3. Otherwise, check the expression normally against the catalog.
4. If the resulting type is `BOOLEAN`, produce `S005` because a boolean is not an orderable scalar.

Examples:

```sql
SELECT name, age FROM students ORDER BY 2 DESC, 1;       -- valid
SELECT name FROM students ORDER BY 3;                    -- S008
SELECT name FROM students ORDER BY age > 18;             -- S005
SELECT name FROM students ORDER BY salary;               -- S003
SELECT name AS n FROM students ORDER BY m;               -- S003
```

Default direction is ascending. `ASC` is explicit ascending; `DESC` is descending.

## 9. Diagnostic Reference

| Code | Trigger | Example |
| --- | --- | --- |
| `S001` | Unknown table, table alias, or qualifier | `SELECT x.name FROM students` |
| `S002` | Duplicate table name or effective alias | `FROM students s, courses s` |
| `S003` | Unknown column | `SELECT salary FROM students` |
| `S004` | Ambiguous unqualified column | `SELECT department FROM students, courses` |
| `S005` | Type or operator misuse | `WHERE name LIKE 10` |
| `S006` | `WHERE` is not boolean | `WHERE age` |
| `S007` | Duplicate select alias | `SELECT name AS n, age AS n` |
| `S008` | `ORDER BY` ordinal out of range | `ORDER BY 3` with two select items |

## 10. Design Notes and Limitations

- The semantic phase is intentionally separate from parsing. The parser accepts syntactically well-formed queries even when table or column names are wrong.
- Name resolution uses the `FROM` scope only. There are no nested scopes or subqueries.
- The demo catalog is fixed, so semantic results are reproducible across machines.
- The compiler validates but does not execute. It does not evaluate `LIKE` patterns, `IN` lists, or arithmetic at compile time.
- `NULL` propagation is checked only for type compatibility. Three-valued logic evaluation is outside the scope of a front-end validator.
- The compiler does not check that `ORDER BY` expressions appear in the select list, except for ordinal bounds and alias resolution. This matches the chosen subset rather than full SQL semantics.
