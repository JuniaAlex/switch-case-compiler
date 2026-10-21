# SQL Mini Compiler

A compiler front-end for a restricted SQL subset. The project performs lexical analysis, syntax analysis, AST construction, and catalog-aware semantic validation for `SELECT` queries. It is validation-only: it never executes a query and never connects to a database.

The submission uses one implementation path:

| Layer | Technology | Entry point |
| --- | --- | --- |
| Compiler | Flex scanner, Bison grammar, C11 semantic modules | `compiler/bin/sqlc.exe` |
| Bridge | Python 3 standard-library HTTP server | `backend/server.py` |
| Frontend | HTML, CSS, and JavaScript | `frontend/index.html` |

The browser sends SQL to the local Python bridge, which runs the native compiler with `--json` and returns tokens, syntax status, AST, diagnostics, and semantic status. The Flex lexer records tokens once; the same token stream feeds parsing and JSON output.

The compiler passes all **79/79** cases in the shared suite. The HTTP bridge passes **8/8** integration tests. The earlier pure-C and in-browser prototypes are retained under `archive/` for reference only.

## Features

- `SELECT` with optional `DISTINCT`, star projection, expressions, and aliases
- `FROM` with one or more tables and optional table aliases
- `WHERE` with `AND`, `OR`, `NOT`, comparisons, arithmetic, `LIKE`, `IN`, `BETWEEN`, and `IS [NOT] NULL`
- `ORDER BY` with `ASC`, `DESC`, column names, select aliases, and ordinal positions
- Case-insensitive keywords and case-sensitive identifiers
- String literals with doubled-quote escaping (`'O''Brien'`)
- `--`, `//`, and `/* ... */` comments
- Token stream, parse tree/AST, and one-based source positions
- Catalog-aware checks for unknown tables/columns, ambiguity, duplicates, and type misuse
- Stable JSON output for tooling and the web frontend
- Local-only Python bridge with request limits and compiler timeout

## SQL Subset

```sql
SELECT [DISTINCT] select_list
FROM table_list
[WHERE expression]
[ORDER BY order_list];
```

Supported expressions include:

```text
OR, AND, NOT
=  !=  <>  <  <=  >  >=
LIKE, NOT LIKE
IN, NOT IN
BETWEEN, NOT BETWEEN
IS NULL, IS NOT NULL
+  -  *  /
unary +, unary -
```

Parentheses are supported. The trailing semicolon is optional.

### Demo catalog

```text
students(student_id INTEGER, name TEXT, age INTEGER,
         department TEXT, cgpa FLOAT, city TEXT)

courses(course_id INTEGER, title TEXT, credits INTEGER,
        department TEXT)

enrollments(enrollment_id INTEGER, student_id INTEGER,
            course_id INTEGER, grade TEXT, semester INTEGER)

departments(department TEXT, hod TEXT, building TEXT, budget FLOAT)
```

## Requirements

- Windows with MinGW GCC on `PATH`
- WSL with `bison` and `flex` for the Windows build script
- Python 3.10 or newer
- A modern browser
- Optional for maintainers: Node.js for JavaScript syntax checks, Playwright for screenshots, `python-docx`/`python-pptx` for generated documents

## Quick Start

Build the compiler:

```powershell
powershell -ExecutionPolicy Bypass -File compiler\build.ps1
```

Start the bridge and open the frontend:

```powershell
python backend\server.py --open
```

Or open `http://127.0.0.1:8000/` after starting the bridge without `--open`.

Do not open `frontend/index.html` through `file://`: the page calls `/api/analyze`, so it must be served by the bridge.

### Compiler CLI

```powershell
compiler\bin\sqlc.exe -e "SELECT name, cgpa FROM students WHERE age >= 18 ORDER BY cgpa DESC;"
compiler\bin\sqlc.exe --tokens -e "SELECT name FROM students;"
compiler\bin\sqlc.exe --json -e "SELECT name FROM students;"
compiler\bin\sqlc.exe queries\example.sql
Get-Content queries\example.sql | compiler\bin\sqlc.exe --json
```

Options:

```text
-e, --query TEXT   compile TEXT instead of a file or stdin
    --tokens       print the token stream
    --ast          print the parse tree / AST (default)
    --no-ast       suppress the AST
    --schema       print the supported database schema
    --json         emit machine-readable analysis JSON
    --quiet        print only diagnostics and the final result
-h, --help         show help
```

Exit codes:

| Code | Meaning |
| --- | --- |
| `0` | Query is valid |
| `1` | Lexical or syntax error |
| `2` | Semantic error |
| `3` | Usage or I/O error |

`--json` implies quiet output. `--schema` and `--json` cannot be combined.

### JSON contract

```json
{
  "ok": true,
  "stage": "valid",
  "tokens": [
    {"index": 1, "type": "SELECT", "lexeme": "SELECT", "line": 1, "column": 1}
  ],
  "diagnostics": [],
  "syntax": {"ok": true, "stage": null, "error": null},
  "ast": {"kind": "query", "selectList": [], "fromList": [], "where": null, "orderList": []},
  "semantics": {"ok": true, "diagnostics": []}
}
```

For lexical or syntax failures, `ast` is `null` and `syntax.ok` is `false`. For semantic failures, `ast` is still present so the UI can show the accepted structure. Top-level `diagnostics` contains all phases; `semantics.diagnostics` contains only `S` codes.

## HTTP API

The bridge binds to `127.0.0.1` by default.

| Method | Path | Purpose |
| --- | --- | --- |
| `GET` | `/` | Serve `frontend/index.html` |
| `GET` | `/api/health` | Report bridge and compiler availability |
| `POST` | `/api/analyze` | Analyze `{"source": "SELECT ..."}` |

Safety limits:

- Request body: 64 KiB maximum
- SQL source: 60,000 characters maximum
- Compiler subprocess timeout: 5 seconds
- `shell=False`; source is passed on stdin
- Analysis failures return HTTP 200 with `ok: false`; transport and validation errors return 4xx/5xx with `{"error": {"code": "...", "message": "..."}}`

## Scenarios

```powershell
# Valid two-table query
compiler\bin\sqlc.exe -e "SELECT s.name, e.grade FROM students s, enrollments e WHERE s.student_id = e.student_id ORDER BY s.name"

# Semantic error: ambiguous unqualified column
compiler\bin\sqlc.exe -e "SELECT department FROM students, courses"

# Syntax error: missing closing parenthesis
compiler\bin\sqlc.exe -e "SELECT name FROM students WHERE (age > 18 ORDER BY age;"

# Token stream
compiler\bin\sqlc.exe --tokens -e "SELECT name, cgpa FROM students ORDER BY cgpa DESC;"
```

## Diagnostics

| Code | Phase | Meaning |
| --- | --- | --- |
| `L001` | Lexical | Illegal character |
| `L002` | Lexical | Unterminated string literal |
| `L003` | Lexical | Unterminated block comment |
| `P001` | Syntax | Unexpected token or malformed construct |
| `P002` | Syntax | Expected a specific token |
| `S001` | Semantic | Unknown table, alias, or qualifier |
| `S002` | Semantic | Duplicate table name or alias |
| `S003` | Semantic | Unknown column |
| `S004` | Semantic | Ambiguous unqualified column |
| `S005` | Semantic | Type or operator misuse |
| `S006` | Semantic | `WHERE` expression is not boolean |
| `S007` | Semantic | Duplicate select alias |
| `S008` | Semantic | `ORDER BY` ordinal out of range |

Diagnostics use one-based line and column numbers. Text mode prints them as:

```text
[S004] line 1, column 8: ambiguous column 'department' (matches students.department, courses.department)
```

## Testing

The shared suite is defined in `tests/cases.json`:

- 30 valid queries
- 25 semantic-error queries
- 20 syntax-error queries
- 4 lexical-error queries

Run the compiler suite:

```powershell
python tests\run_tests.py
python tests\run_tests.py --verbose
```

Run the bridge integration tests:

```powershell
python -m unittest discover -s tests -p "test_*.py" -v
```

Expected results:

```text
compiler suite: 79 passed, 0 failed
bridge tests:    8 passed, 0 failed
```

The compiler runner checks the JSON contract, exit code, stage, AST presence/absence, diagnostic code, and token position validity. The bridge tests cover health, static serving, valid/error analysis, invalid JSON, wrong field types, and oversized input.

## Project Structure

```text
.
|-- compiler/               Flex + Bison + C compiler
|   |-- src/                lexer.l, parser.y, AST, semantic, catalog, JSON output
|   |-- bin/sqlc.exe        Built native compiler
|   |-- Makefile            Linux/macOS/WSL build
|   `-- build.ps1           Windows build through WSL Flex/Bison + MinGW GCC
|-- backend/
|   `-- server.py           Local HTTP bridge
|-- frontend/
|   |-- index.html
|   |-- styles.css
|   `-- app.js
|-- tests/
|   |-- cases.json          Shared 79-case compiler suite
|   |-- run_tests.py        JSON compiler runner
|   `-- test_bridge.py      HTTP bridge integration tests
|-- scripts/
|   |-- build_presentation.py
|   |-- build_report_docx.py
|   |-- export_presentation_pdf.py
|   |-- export_report_pdf.py
|   |-- generate_test_cases_doc.py
|   |-- make_architecture_diagram.py
|   `-- make_screenshots.py
|-- screenshots/            Evidence images used in docs and slides
|-- docs/                   Report, grammar, semantic rules, tests, diagrams
|-- presentation/           PowerPoint deck and PDF export
|-- queries/                Sample SQL files
|-- archive/                Earlier prototypes, retained for reference
`-- README.md
```

## Screenshots

| File | Content |
| --- | --- |
| `screenshots/01_valid_ast.png` | Valid join with AST in the new frontend |
| `screenshots/02_tokens.png` | Token table from the native Flex scanner |
| `screenshots/03_semantic_error.png` | Ambiguous-column semantic diagnostic |
| `screenshots/04_syntax_error.png` | Missing-parenthesis syntax diagnostic |
| `screenshots/05_lexical_error.png` | Unterminated-string lexical diagnostic |
| `screenshots/06_schema.png` | Demo catalog view |
| `screenshots/07_test_results.png` | Compiler and bridge test summaries |
| `screenshots/08_json_output.png` | Native `--json` output for tooling |

Regenerate all screenshots with:

```powershell
python scripts\make_screenshots.py
```

## Documentation

- `docs/PROJECT_REPORT.md` - full project report
- `docs/CFG_AND_TRANSFORMATIONS.md` - grammar, precedence, left-recursion removal
- `docs/SEMANTIC_CHECKS.md` - catalog, name resolution, type rules, diagnostics
- `docs/TEST_CASES.md` - test strategy and case catalog
- `docs/INDIVIDUAL_CONTRIBUTIONS.md` - contribution matrix template
- `docs/SCREENSHOTS.md` - screenshot index and captions

Generated deliverables:

- `docs/PROJECT_REPORT.docx`
- `docs/PROJECT_REPORT.pdf`
- `presentation/SQL_Mini_Compiler_Presentation.pptx`
- `presentation/SQL_Mini_Compiler_Presentation.pdf`

## Design Notes

- The compiler is the single source of truth. The browser does not reimplement lexing, parsing, or semantic rules.
- Flex records tokens as they are produced; JSON output does not run a second scanner. This keeps token positions identical to the parser's input.
- Bison builds the AST directly. C modules handle catalog lookup, name resolution, and type rules.
- `--json` serializes tokens, syntax status, AST, and diagnostics as one JSON object on stdout.
- The Python bridge serves static files and invokes the compiler as a subprocess. It never evaluates SQL itself.
- The frontend is presentation-only and renders the bridge response.
- There is no SQL execution, no storage engine, and no database driver.

## Team

Replace the placeholders below before submission:

| Name | Roll number | Primary responsibility |
| --- | --- | --- |
| `<Member 1>` | `<Roll No.>` | Lexical analysis and compiler core |
| `<Member 2>` | `<Roll No.>` | Bison grammar and AST |
| `<Member 3>` | `<Roll No.>` | Semantic analysis and catalog |
| `<Member 4>` | `<Roll No.>` | Python bridge, frontend, testing, documentation |

Course, institution, and guide details are also placeholders in the report title page.
