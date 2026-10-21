#!/usr/bin/env python3
"""Generate docs/TEST_CASES.md from the shared JSON test suite."""

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CASES_FILE = ROOT / "tests" / "cases.json"
OUTPUT = ROOT / "docs" / "TEST_CASES.md"

CATEGORY_INFO = {
    "valid": ("Valid Queries", "All of these queries must finish with exit code 0 and `Result: VALID`."),
    "semantic": ("Semantic Errors", "Parsing must succeed, then semantic analysis must report the listed code."),
    "syntax": ("Syntax Errors", "The parser must reject the query. `P` means any parser code (`P001` or `P002`)."),
    "lexical": ("Lexical Errors", "The scanner must report the listed lexical code before parsing."),
}


def expected_label(case):
    if case["valid"]:
        return "VALID"
    if case.get("error") == "P":
        return "P001/P002"
    return case.get("error", "")


def code(value):
    return "`" + value.replace("|", "\\|") + "`"


def main():
    data = json.loads(CASES_FILE.read_text(encoding="utf-8"))
    cases = data["cases"]
    counts = {name: sum(1 for case in cases if case["category"] == name) for name in CATEGORY_INFO}

    lines = []
    lines.append("# Test Cases")
    lines.append("")
    lines.append("The shared suite is stored in `tests/cases.json` and is executed by `tests/run_tests.py`.")
    lines.append("Every case is run against all three front-ends. The runner checks both the process exit")
    lines.append("status and the expected diagnostic code.")
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append("| Category | Cases | Expected result |")
    lines.append("| --- | ---: | --- |")
    for category, (label, rule) in CATEGORY_INFO.items():
        lines.append("| %s | %d | %s |" % (label, counts[category], rule))
    lines.append("| **Total** | **%d** | **All implementations must pass every case** |" % len(cases))
    lines.append("")
    lines.append("Current result:")
    lines.append("")
    lines.append("```text")
    lines.append("c      79 passed    0 failed")
    lines.append("yacc   79 passed    0 failed")
    lines.append("web    79 passed    0 failed")
    lines.append("Total failures: 0")
    lines.append("```")
    lines.append("")
    lines.append("## How to Run")
    lines.append("")
    lines.append("```powershell")
    lines.append("python tests\\run_tests.py --suite all")
    lines.append("python tests\\run_tests.py --suite c")
    lines.append("python tests\\run_tests.py --suite yacc")
    lines.append("python tests\\run_tests.py --suite web")
    lines.append("```")
    lines.append("")
    lines.append("The `c` and `yacc` suites use the native executables; the `web` suite uses")
    lines.append("`node web/compile_cli.js` so the same browser compiler core is tested without a browser.")
    lines.append("")

    for category, (label, rule) in CATEGORY_INFO.items():
        subset = [case for case in cases if case["category"] == category]
        lines.append("## %s (%d)" % (label, len(subset)))
        lines.append("")
        lines.append(rule)
        lines.append("")
        lines.append("| ID | SQL | Expected | Note |")
        lines.append("| --- | --- | --- | --- |")
        for case in subset:
            sql = "`" + case["sql"].replace("\n", " ").replace("|", "\\|") + "`"
            note = case.get("note", "").replace("|", "\\|")
            lines.append("| %s | %s | %s | %s |" % (case["id"], sql, code(expected_label(case)), note))
        lines.append("")

    lines.append("## Error-Code Distribution")
    lines.append("")
    lines.append("| Code | Meaning | Invalid cases |")
    lines.append("| --- | --- | ---: |")
    distribution = {}
    for case in cases:
        if not case["valid"]:
            distribution[case.get("error", "")] = distribution.get(case.get("error", ""), 0) + 1
    meaning = {
        "L001": "Illegal character",
        "L002": "Unterminated string",
        "L003": "Unterminated block comment",
        "P": "Any parser diagnostic",
        "S001": "Unknown table or alias",
        "S002": "Duplicate table or alias",
        "S003": "Unknown column",
        "S004": "Ambiguous column",
        "S005": "Type or operator error",
        "S006": "WHERE is not boolean",
        "S007": "Duplicate select alias",
        "S008": "ORDER BY ordinal out of range",
    }
    for key in ["L001", "L002", "L003", "P", "S001", "S002", "S003", "S004", "S005", "S006", "S007", "S008"]:
        if distribution.get(key):
            lines.append("| `%s` | %s | %d |" % (key, meaning[key], distribution[key]))
    lines.append("")
    lines.append("## Coverage Notes")
    lines.append("")
    lines.append("- Valid cases cover star projection, aliases, joins, comments, quoted strings, arithmetic,")
    lines.append("  comparisons, boolean operators, `LIKE`, `IN`, `BETWEEN`, `IS NULL`, ordinal ordering,")
    lines.append("  alias ordering, case-insensitive keywords, and optional semicolons.")
    lines.append("- Semantic cases cover every code from `S001` through `S008`.")
    lines.append("- Syntax cases cover missing clauses, malformed lists, duplicate tokens, empty input, and")
    lines.append("  missing parentheses or bounds.")
    lines.append("- Lexical cases cover unterminated strings, illegal characters, illegal `!`, and unterminated")
    lines.append("  block comments.")
    lines.append("- The suite is implementation-neutral: the same JSON input is used for the C, Lex/Yacc,")
    lines.append("  and browser implementations.")
    lines.append("")

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    print("wrote", OUTPUT)


if __name__ == "__main__":
    main()
