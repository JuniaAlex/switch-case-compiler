# Grammar and Transformations

This document defines the concrete grammar used by the SQL mini-compiler, explains operator precedence, and records the grammar shape used by Bison. The implementation is `compiler/src/parser.y`, with tokens produced by `compiler/src/lexer.l`.

## 1. Notation

- Terminals are written in uppercase: `SELECT`, `IDENT`, `NUMBER`.
- Nonterminals are lowercase: `query`, `expr`, `select_list`.
- `ε` denotes the empty production.
- `{ x }` denotes zero or more repetitions of `x`.
- `[ x ]` denotes an optional `x`.
- `|` separates alternatives.
- `EOF` is the end-of-input token.

## 2. Lexical Tokens

| Category | Tokens |
| --- | --- |
| Keywords | `SELECT`, `DISTINCT`, `FROM`, `WHERE`, `ORDER`, `BY`, `ASC`, `DESC`, `AND`, `OR`, `NOT`, `AS`, `IN`, `BETWEEN`, `LIKE`, `IS`, `NULL`, `TRUE`, `FALSE` |
| Identifiers | `IDENT` |
| Literals | `INT_LITERAL`, `FLOAT_LITERAL`, `STRING` |
| Operators | `STAR`, `PLUS`, `MINUS`, `SLASH`, `EQ`, `NE`, `LT`, `LE`, `GT`, `GE` |
| Punctuation | `COMMA`, `DOT`, `LPAREN`, `RPAREN`, `SEMI` |
| End of input | `EOF` |

Keywords are case-insensitive. Identifiers are case-sensitive after lexing. A string literal uses single quotes and escapes an embedded quote by doubling it: `'O''Brien'`.

## 3. Concrete Grammar

The grammar below is the Bison grammar. It is stratified into precedence levels, so no `%left` or `%right` declarations are required and Bison reports no shift/reduce or reduce/reduce conflicts.

```text
query
    : SELECT opt_distinct select_list FROM table_list opt_where opt_order opt_semi

opt_distinct
    : ε
    | DISTINCT

select_list
    : select_item
    | select_list COMMA select_item

select_item
    : STAR
    | expr
    | expr IDENT
    | expr AS IDENT

table_list
    : table_ref
    | table_list COMMA table_ref

table_ref
    : IDENT
    | IDENT IDENT
    | IDENT AS IDENT

opt_where
    : ε
    | WHERE expr

opt_order
    : ε
    | ORDER BY order_list

order_list
    : order_item
    | order_list COMMA order_item

order_item
    : expr
    | expr ASC
    | expr DESC

opt_semi
    : ε
    | SEMI

expr
    : or_expr

or_expr
    : and_expr
    | or_expr OR and_expr

and_expr
    : not_expr
    | and_expr AND not_expr

not_expr
    : predicate
    | NOT not_expr

predicate
    : additive
    | additive EQ additive
    | additive NE additive
    | additive LT additive
    | additive LE additive
    | additive GT additive
    | additive GE additive
    | additive LIKE additive
    | additive NOT LIKE additive
    | additive IN LPAREN literal_list RPAREN
    | additive NOT IN LPAREN literal_list RPAREN
    | additive BETWEEN additive AND additive
    | additive NOT BETWEEN additive AND additive
    | additive IS NULL
    | additive IS NOT NULL

additive
    : multiplicative
    | additive PLUS multiplicative
    | additive MINUS multiplicative

multiplicative
    : unary
    | multiplicative STAR unary
    | multiplicative SLASH unary

unary
    : primary
    | PLUS unary
    | MINUS unary

primary
    : literal
    | IDENT
    | IDENT DOT IDENT
    | LPAREN expr RPAREN

literal
    : INT_LITERAL
    | FLOAT_LITERAL
    | STRING
    | NULL
    | TRUE
    | FALSE

literal_list
    : literal
    | literal_list COMMA literal
```

## 4. Operator Precedence

From lowest to highest:

| Level | Operators | Associativity |
| --- | --- | --- |
| 1 | `OR` | Left |
| 2 | `AND` | Left |
| 3 | `NOT` | Right (prefix) |
| 4 | `=`, `!=`, `<>`, `<`, `<=`, `>`, `>=`, `LIKE`, `IN`, `BETWEEN`, `IS NULL` | Non-associative in practice |
| 5 | `+`, `-` | Left |
| 6 | `*`, `/` | Left |
| 7 | unary `+`, unary `-` | Right (prefix) |
| 8 | `(` ... `)`, column, literal | Highest |

Consequences:

- `a OR b AND c` parses as `a OR (b AND c)`.
- `NOT age = 18` parses as `NOT (age = 18)`.
- `age BETWEEN 18 AND 25 AND cgpa > 7` parses as `(age BETWEEN 18 AND 25) AND (cgpa > 7)` because the `AND` inside `BETWEEN` belongs to the `BETWEEN` production.
- `a + b * c` parses as `a + (b * c)`.
- Chained comparisons such as `a = b = c` are rejected; the grammar has no comparison-level recursion.

## 5. Left Recursion and Lists

The grammar keeps left recursion for list and binary levels. Bison handles left-recursive LALR productions directly, and left recursion gives the intended left associativity for subtraction, division, `AND`, and `OR`.

```text
select_list -> select_list COMMA select_item
             | select_item

table_list  -> table_list COMMA table_ref
             | table_ref

or_expr     -> or_expr OR and_expr
             | and_expr

additive    -> additive PLUS multiplicative
             | additive MINUS multiplicative
             | multiplicative
```

The repeated list pattern preserves source order and lets each action append to the existing list. The grammar reports `%expect 0`; precedence is encoded through production levels rather than conflict declarations.

The unary level remains right-recursive:

```text
unary -> PLUS unary | MINUS unary | primary
```

Bison builds the AST in the same actions, so the parser output and the JSON AST use one representation.

## 6. FIRST Sets

| Nonterminal | FIRST set |
| --- | --- |
| `literal` | `{ INT_LITERAL, FLOAT_LITERAL, STRING, NULL, TRUE, FALSE }` |
| `primary` | `FIRST(literal) ∪ { IDENT, LPAREN }` |
| `unary` | `FIRST(primary) ∪ { PLUS, MINUS }` |
| `multiplicative` | `FIRST(unary)` |
| `additive` | `FIRST(multiplicative)` |
| `predicate` | `FIRST(additive)` |
| `not_expr` | `FIRST(predicate) ∪ { NOT }` |
| `and_expr` | `FIRST(not_expr)` |
| `or_expr` | `FIRST(and_expr)` |
| `expr` | `FIRST(or_expr)` |
| `select_item` | `{ STAR } ∪ FIRST(expr)` |
| `select_list` | `FIRST(select_item)` |
| `table_ref` | `{ IDENT }` |
| `table_list` | `{ IDENT }` |
| `order_item` | `FIRST(expr)` |
| `order_list` | `FIRST(order_item)` |
| `literal_list` | `FIRST(literal)` |
| `query` | `{ SELECT }` |

Expanding the first sets:

```text
FIRST(expr) = { IDENT, INT_LITERAL, FLOAT_LITERAL, STRING,
                NULL, TRUE, FALSE, LPAREN, PLUS, MINUS, NOT }
```

`NOT` is included because `expr` can begin with `not_expr`, and `not_expr` can begin with `NOT`.

## 7. FOLLOW Sets for Optional Clauses

The most useful FOLLOW sets for parse decisions are:

| Nonterminal | FOLLOW set |
| --- | --- |
| `opt_distinct` | `{ FROM }` |
| `select_list` | `{ FROM, COMMA }` |
| `select_item` | `{ COMMA, FROM }` |
| `table_list` | `{ WHERE, ORDER, SEMI, EOF }` |
| `table_ref` | `{ COMMA, WHERE, ORDER, SEMI, EOF }` |
| `opt_where` | `{ ORDER, SEMI, EOF }` |
| `opt_order` | `{ SEMI, EOF }` |
| `order_item` | `{ COMMA, SEMI, EOF }` |
| `order_list` | `{ SEMI, EOF }` |
| `opt_semi` | `{ EOF }` |

These sets determine when to stop parsing a list and when to accept an optional clause.

## 8. Recursive-Descent Parse Decisions

The hand-written parser uses one token of lookahead and the following decisions:

| Construct | Decision |
| --- | --- |
| Query | Require `SELECT`; parse optional `DISTINCT`, select list, `FROM`, table list, optional `WHERE`, optional `ORDER BY`, optional `;`. |
| Select item | If `*`, emit star item. Otherwise parse `expr`, then accept an optional alias introduced by `AS` or an implicit `IDENT`. |
| Table reference | Require `IDENT`; accept an optional alias introduced by `AS` or an implicit `IDENT`. |
| Select list | Parse one item, then repeat while the next token is `COMMA`. |
| Table list | Parse one reference, then repeat while the next token is `COMMA`. |
| WHERE | If the next token is `WHERE`, parse `expr`; otherwise stop. |
| ORDER BY | If the next tokens are `ORDER BY`, parse one order item and repeat while commas remain. |
| Order item | Parse `expr`; accept optional `ASC` or `DESC`; default is ascending. |
| Expression | Parse `or_expr`; binary levels loop while their operator appears. |
| Predicate | Parse an additive expression, then inspect the current token to decide whether to build a comparison, `LIKE`, `IN`, `BETWEEN`, or `IS NULL`. |
| Primary | Literal, column, qualified column, or parenthesized expression. |

The only place where the parser needs more than a simple FIRST-set decision is the optional select alias: after parsing an expression, an `IDENT` or `AS` token means an alias follows. This is handled by consuming the expression first and then checking the next token, which keeps the parser deterministic.

## 9. Example Derivation

Query:

```sql
SELECT name, cgpa
FROM students
WHERE age >= 18 AND city = 'Delhi'
ORDER BY cgpa DESC;
```

Outline of the derivation:

```text
query
  -> SELECT select_list FROM table_list WHERE expr ORDER BY order_list ;
  -> SELECT expr COMMA expr FROM IDENT
     WHERE expr AND expr
     ORDER BY expr DESC ;
  -> SELECT IDENT COMMA IDENT FROM IDENT
     WHERE IDENT GE INT_LITERAL AND IDENT EQ STRING
     ORDER BY IDENT DESC ;
```

The corresponding AST has:

```text
Query
  SelectList (2 items)
    SelectItem: Column(name)
    SelectItem: Column(cgpa)
  FromList (1 table)
    TableRef: students
  Where
    Binary(AND)
      Binary(>=)
        Column(age)
        Literal(integer 18)
      Binary(=)
        Column(city)
        Literal(string 'Delhi')
  OrderList (1 item)
    OrderItem(DESC)
      Column(cgpa)
```

## 10. Grammar Coverage and Restrictions

Included:

- Optional `DISTINCT`
- Star projection
- Multiple select items
- Multiple tables (comma join)
- Table aliases with or without `AS`
- Select aliases with or without `AS`
- Boolean expressions with `AND`, `OR`, `NOT`
- Comparison operators
- Arithmetic expressions
- `LIKE` and `NOT LIKE`
- `IN` and `NOT IN`
- `BETWEEN` and `NOT BETWEEN`
- `IS NULL` and `IS NOT NULL`
- Ordinal and alias references in `ORDER BY`
- Comments, optional semicolon, quoted strings

Deliberately excluded:

- `JOIN ... ON`
- `GROUP BY`, `HAVING`
- Subqueries
- `UNION`
- `INSERT`, `UPDATE`, `DELETE`, `CREATE`
- SQL functions and aggregate functions
- Query execution and database connectivity
