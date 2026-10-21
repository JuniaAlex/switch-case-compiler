# Individual Contributions

> Replace the placeholders with the actual team names, roll numbers, and contribution percentages before submission. The allocation below is a planning baseline based on the module boundaries in this repository, not a claim about the final team split.

## Team

| Member | Roll number | Primary role | Contribution |
| --- | --- | --- | --- |
| `<Member 1>` | `<Roll No.>` | Lexical analysis and compiler core | `compiler/src/lexer.l`, `compiler/src/support.c`, token/diagnostic types, lexical tests |
| `<Member 2>` | `<Roll No.>` | Bison grammar and AST | `compiler/src/parser.y`, `compiler/src/ast.c`, JSON serializer, AST tests |
| `<Member 3>` | `<Roll No.>` | Semantic analysis and catalog | `compiler/src/semantic.c`, `compiler/src/catalog.c`, semantic tests and rule documentation |
| `<Member 4>` | `<Roll No.>` | Bridge, frontend, testing, documentation | `backend/`, `frontend/`, `tests/`, `scripts/`, report, presentation, screenshots |

Planning baseline: **25% per member**. Adjust the percentages and evidence after reviewing the actual work log.

## Detailed Ownership

### Member 1 - Lexical Analysis and Compiler Core

- Defined token categories and the scanner interface in `compiler/src/sqlmini.h`.
- Implemented whitespace, comment, identifier, keyword, number, string, and operator scanning.
- Added one-based line/column tracking and token recording for JSON output.
- Implemented diagnostics `L001`, `L002`, and `L003`.
- Implemented dynamic token storage, source reading, and UTF-8 BOM handling.
- Contributed lexical cases `L01`-`L04` and scanner documentation.

Evidence:

- `compiler/src/lexer.l`
- `compiler/src/support.c`
- `compiler/src/driver.h`
- `compiler/src/sqlmini.h`
- `screenshots/05_lexical_error.png`

### Member 2 - Parser, AST, and JSON Output

- Wrote the Bison grammar and AST-building actions.
- Implemented list parsing, optional aliases, table references, and expression precedence.
- Designed and implemented AST constructors, destructors, and printer.
- Implemented the JSON serializer for tokens, syntax status, AST, and diagnostics.
- Verified `%expect 0` and conflict-free Bison generation.
- Contributed syntax cases and grammar documentation.

Evidence:

- `compiler/src/parser.y`
- `compiler/src/ast.c`
- `compiler/src/json_output.c`
- `compiler/src/json_output.h`
- `compiler/src/main.c`
- `docs/CFG_AND_TRANSFORMATIONS.md`

### Member 3 - Semantic Analysis and Catalog

- Defined the four-table demo catalog with SQL types.
- Implemented `FROM` scope construction and alias resolution.
- Implemented qualified and unqualified column resolution.
- Implemented ambiguity detection (`S004`) and duplicate table/alias detection (`S002`).
- Implemented the type system and expression checks.
- Implemented `WHERE` boolean rule, select-alias validation, and `ORDER BY` ordinal/alias rules.
- Contributed semantic cases `E01`-`E25` and semantic documentation.

Evidence:

- `compiler/src/semantic.c`
- `compiler/src/catalog.c`
- `docs/SEMANTIC_CHECKS.md`

### Member 4 - Bridge, Frontend, Testing, and Documentation

- Implemented the Python HTTP bridge and subprocess isolation.
- Built the editor, examples, tabs, AST tree, token table, diagnostics, schema, and bridge status.
- Added the JSON compiler test runner and bridge integration tests.
- Added automated screenshot generation with Playwright and PIL.
- Prepared the report, presentation, README, and submission structure.
- Ran the final 79-case compiler suite and 8 bridge tests.

Evidence:

- `backend/server.py`
- `frontend/index.html`
- `frontend/styles.css`
- `frontend/app.js`
- `tests/cases.json`
- `tests/run_tests.py`
- `tests/test_bridge.py`
- `scripts/make_screenshots.py`
- `screenshots/`
- `docs/PROJECT_REPORT.md`

## Shared Responsibilities

All members should review:

- the grammar and semantic rules;
- the final test results;
- the report's claims and screenshots;
- the presentation;
- the repository before submission.

## Sign-Off

| Member | Reviewed grammar | Reviewed semantics | Reviewed tests | Reviewed report | Signature |
| --- | --- | --- | --- | --- | --- |
| `<Member 1>` | | | | | |
| `<Member 2>` | | | | | |
| `<Member 3>` | | | | | |
| `<Member 4>` | | | | | |
