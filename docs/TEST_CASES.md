# Test Cases

The compiler suite is stored in `tests/cases.json` and is executed by `tests/run_tests.py`.
Every case is sent to the single native compiler through its `--json` interface. The runner
checks the exit status, JSON contract, stage, AST presence, token positions, and expected
diagnostic code. A separate bridge suite in `tests/test_bridge.py` exercises the HTTP layer.

## Summary

| Category | Cases | Expected result |
| --- | ---: | --- |
| Valid Queries | 30 | All of these queries must finish with exit code 0 and `Result: VALID`. |
| Semantic Errors | 25 | Parsing must succeed, then semantic analysis must report the listed code. |
| Syntax Errors | 20 | The parser must reject the query. `P` means any parser code (`P001` or `P002`). |
| Lexical Errors | 4 | The scanner must report the listed lexical code before parsing. |
| **Total** | **79** | **The compiler must pass every case** |

Current result:

```text
compiler  79 passed    0 failed
bridge     8 passed    0 failed
```

## How to Run

```powershell
python tests\run_tests.py
python tests\run_tests.py --verbose
python -m unittest discover -s tests -p "test_*.py" -v
```

The runner invokes `compiler/bin/sqlc.exe --json` and parses stdout as JSON. The bridge
suite starts a temporary local server and checks health, static serving, valid and invalid
analysis, malformed requests, wrong field types, and oversized input.

## Valid Queries (30)

All of these queries must finish with exit code 0 and `Result: VALID`.

| ID | SQL | Expected | Note |
| --- | --- | --- | --- |
| V01 | `SELECT * FROM students` | `VALID` | star projection |
| V02 | `SELECT name, cgpa FROM students` | `VALID` | column list |
| V03 | `SELECT name AS student_name, age FROM students` | `VALID` | AS alias |
| V04 | `SELECT name student_name FROM students` | `VALID` | implicit alias |
| V05 | `SELECT DISTINCT * FROM students` | `VALID` | distinct star |
| V06 | `SELECT name FROM students WHERE age >= 18 ORDER BY age DESC` | `VALID` | where and order |
| V07 | `SELECT s.name, e.grade FROM students s, enrollments e WHERE s.student_id = e.student_id` | `VALID` | two-table join |
| V08 | `SELECT s.name, c.title FROM students s, enrollments e, courses c WHERE s.student_id = e.student_id AND e.course_id = c.course_id ORDER BY s.name ASC, c.title` | `VALID` | three-table join |
| V09 | `SELECT name FROM students WHERE cgpa BETWEEN 7.5 AND 10 AND city IN ('Delhi', 'Mumbai')` | `VALID` | between and in |
| V10 | `SELECT name FROM students WHERE name LIKE 'A%' AND department IS NOT NULL` | `VALID` | like and is not null |
| V11 | `SELECT name FROM students WHERE age NOT IN (18, 19) AND name NOT LIKE 'X%' AND cgpa NOT BETWEEN 0 AND 4` | `VALID` | negated predicates |
| V12 | `SELECT name FROM students WHERE NOT (age < 18 OR cgpa < 5) AND city IS NOT NULL` | `VALID` | parenthesized boolean expression |
| V13 | `SELECT credits * 2 + 1 AS weighted FROM courses ORDER BY weighted DESC` | `VALID` | arithmetic expression and alias |
| V14 | `SELECT cgpa * 2 AS double_cgpa FROM students ORDER BY double_cgpa` | `VALID` | order by select alias |
| V15 | `SELECT name, age FROM students ORDER BY 2 DESC, 1` | `VALID` | order by ordinal |
| V16 | `SELECT name FROM students ORDER BY cgpa DESC, name ASC` | `VALID` | multiple order keys |
| V17 | `sElEcT NAME FrOm STUDENTS WhErE AGE > 18 OrDeR By AGE dEsC` | `VALID` | case-insensitive keywords and identifiers |
| V18 | `SELECT name /* projection */ FROM students -- source table WHERE age > 18` | `VALID` | line and block comments |
| V19 | `SELECT name FROM students WHERE name = 'O''Brien'` | `VALID` | escaped quote in string |
| V20 | `SELECT students.name, students.cgpa FROM students` | `VALID` | qualified columns |
| V21 | `SELECT s.name FROM students s WHERE s.age > 20` | `VALID` | table alias without AS |
| V22 | `SELECT name FROM students WHERE TRUE` | `VALID` | boolean literal predicate |
| V23 | `SELECT name FROM students WHERE age > -1` | `VALID` | negative numeric literal |
| V24 | `SELECT cgpa / 2.0 AS half_cgpa FROM students WHERE cgpa + 0.5 > 5.0` | `VALID` | floating-point arithmetic |
| V25 | `SELECT name FROM students;` | `VALID` | optional semicolon |
| V26 | `SELECT title FROM courses WHERE department IS NULL` | `VALID` | IS NULL |
| V27 | `SELECT name FROM students WHERE NOT age = 18` | `VALID` | NOT before comparison |
| V28 | `SELECT age > 18 AS is_adult FROM students ORDER BY 1` | `VALID` | boolean select expression |
| V29 | `SELECT name FROM students WHERE city = 'Delhi' OR city = 'Mumbai' OR city = 'Pune'` | `VALID` | OR chain |
| V30 | `SELECT name FROM students WHERE (age >= 18 AND cgpa >= 6) OR (age < 18 AND cgpa >= 8)` | `VALID` | nested boolean groups |

## Semantic Errors (25)

Parsing must succeed, then semantic analysis must report the listed code.

| ID | SQL | Expected | Note |
| --- | --- | --- | --- |
| E01 | `SELECT name FROM teachers` | `S001` | unknown table |
| E02 | `SELECT salary FROM students` | `S003` | unknown column |
| E03 | `SELECT department FROM students, courses` | `S004` | ambiguous unqualified column |
| E04 | `SELECT s.name FROM students` | `S001` | unknown table alias |
| E05 | `SELECT * FROM students s, courses s` | `S002` | duplicate table alias |
| E06 | `SELECT name AS n, age AS n FROM students` | `S007` | duplicate select alias |
| E07 | `SELECT name FROM students WHERE age` | `S006` | WHERE is not boolean |
| E08 | `SELECT name + 1 FROM students` | `S005` | arithmetic on text |
| E09 | `SELECT name FROM students WHERE age = 'eighteen'` | `S005` | comparison type mismatch |
| E10 | `SELECT name FROM students WHERE name LIKE 10` | `S005` | LIKE pattern must be text |
| E11 | `SELECT name FROM students WHERE age IN (1, 'x')` | `S005` | IN list type mismatch |
| E12 | `SELECT name FROM students WHERE cgpa BETWEEN 'a' AND 'b'` | `S005` | BETWEEN type mismatch |
| E13 | `SELECT name FROM students ORDER BY salary` | `S003` | unknown ORDER BY column |
| E14 | `SELECT name FROM students ORDER BY 3` | `S008` | ORDER BY ordinal out of range |
| E15 | `SELECT name FROM students ORDER BY age > 18` | `S005` | ORDER BY must be scalar |
| E16 | `SELECT students.salary FROM students` | `S003` | qualified unknown column |
| E17 | `SELECT x.name FROM students` | `S001` | unknown qualified table |
| E18 | `SELECT * FROM students, students` | `S002` | duplicate table without alias |
| E19 | `SELECT name FROM students WHERE age AND TRUE` | `S005` | AND requires booleans |
| E20 | `SELECT name FROM students WHERE NOT age` | `S005` | NOT requires boolean |
| E21 | `SELECT name / 2 FROM students` | `S005` | division on text |
| E22 | `SELECT name FROM students WHERE nonexistent = 1` | `S003` | unknown column in WHERE |
| E23 | `SELECT name FROM students WHERE age IN (TRUE, FALSE)` | `S005` | IN boolean mismatch |
| E24 | `SELECT name FROM students ORDER BY TRUE` | `S005` | ORDER BY boolean literal |
| E25 | `SELECT name AS n FROM students ORDER BY m` | `S003` | unknown ORDER BY alias and column |

## Syntax Errors (20)

The parser must reject the query. `P` means any parser code (`P001` or `P002`).

| ID | SQL | Expected | Note |
| --- | --- | --- | --- |
| P01 | `SELECT name` | `P001/P002` | missing FROM |
| P02 | `SELECT name, FROM students` | `P001/P002` | trailing comma in select list |
| P03 | `SELECT name FROM students WHERE` | `P001/P002` | missing WHERE expression |
| P04 | `SELECT name FROM students WHERE (age > 18` | `P001/P002` | missing closing parenthesis |
| P05 | `SELECT name FROM students students2 students3` | `P001/P002` | extra token |
| P06 | `SELECT name FROM students WHERE age = = 18` | `P001/P002` | double comparison operator |
| P07 | `` | `P001/P002` | empty input |
| P08 | `SELECT name FROM students ORDER age` | `P001/P002` | missing BY |
| P09 | `SELECT name FROM students WHERE age BETWEEN 1 AND` | `P001/P002` | missing BETWEEN upper bound |
| P10 | `SELECT name FROM students WHERE age IN ()` | `P001/P002` | empty IN list |
| P11 | `SELECT name FROM students WHERE age = 18 AND` | `P001/P002` | dangling AND |
| P12 | `SELECT name FROM students;;` | `P001/P002` | extra semicolon |
| P13 | `SELECT * AS x FROM students` | `P001/P002` | star cannot have alias |
| P14 | `FROM students` | `P001/P002` | missing SELECT |
| P15 | `SELECT , name FROM students` | `P001/P002` | leading comma |
| P16 | `SELECT name FROM` | `P001/P002` | missing table |
| P17 | `SELECT name FROM students ORDER BY` | `P001/P002` | missing ORDER BY expression |
| P18 | `SELECT name FROM students WHERE age >` | `P001/P002` | missing comparison operand |
| P19 | `SELECT name FROM students WHERE age IS 18` | `P001/P002` | IS must be followed by NULL |
| P20 | `SELECT name FROM students WHERE age IN (1, 2` | `P001/P002` | missing IN closing parenthesis |

## Lexical Errors (4)

The scanner must report the listed lexical code before parsing.

| ID | SQL | Expected | Note |
| --- | --- | --- | --- |
| L01 | `SELECT name FROM students WHERE name = 'Bob` | `L002` | unterminated string |
| L02 | `SELECT name FROM students @` | `L001` | illegal character |
| L03 | `SELECT name FROM students WHERE age ! 18` | `L001` | illegal exclamation mark |
| L04 | `SELECT name FROM students /* unfinished comment` | `L003` | unterminated block comment |

## Error-Code Distribution

| Code | Meaning | Invalid cases |
| --- | --- | ---: |
| `L001` | Illegal character | 2 |
| `L002` | Unterminated string | 1 |
| `L003` | Unterminated block comment | 1 |
| `P` | Any parser diagnostic | 20 |
| `S001` | Unknown table or alias | 3 |
| `S002` | Duplicate table or alias | 2 |
| `S003` | Unknown column | 5 |
| `S004` | Ambiguous column | 1 |
| `S005` | Type or operator error | 11 |
| `S006` | WHERE is not boolean | 1 |
| `S007` | Duplicate select alias | 1 |
| `S008` | ORDER BY ordinal out of range | 1 |

## Coverage Notes

- Valid cases cover star projection, aliases, joins, comments, quoted strings, arithmetic,
  comparisons, boolean operators, `LIKE`, `IN`, `BETWEEN`, `IS NULL`, ordinal ordering,
  alias ordering, case-insensitive keywords, and optional semicolons.
- Semantic cases cover every code from `S001` through `S008`.
- Syntax cases cover missing clauses, malformed lists, duplicate tokens, empty input, and
  missing parentheses or bounds.
- Lexical cases cover unterminated strings, illegal characters, illegal `!`, and unterminated
  block comments.
- The suite is interface-stable: every case is checked through the same `--json` contract
  consumed by the Python bridge and frontend.
