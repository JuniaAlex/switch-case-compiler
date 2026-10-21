#!/usr/bin/env python3
import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CASES_FILE = Path(__file__).resolve().parent / "cases.json"
COMPILER = ROOT / "compiler" / "bin" / ("sqlc.exe" if sys.platform.startswith("win") else "sqlc")


def run_case(sql):
    completed = subprocess.run(
        [str(COMPILER), "--json"],
        cwd=str(ROOT),
        input=sql,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=15,
    )
    payload = json.loads(completed.stdout)
    return completed.returncode, payload, completed.stderr


def validate_token_list(tokens):
    if not isinstance(tokens, list):
        return "tokens is not a list"
    for index, token in enumerate(tokens, start=1):
        if token.get("index") != index:
            return "token index mismatch at position %d" % index
        if not isinstance(token.get("type"), str) or not token["type"]:
            return "token %d has no type" % index
        if not isinstance(token.get("lexeme"), str):
            return "token %d has no lexeme" % index
        if not isinstance(token.get("line"), int) or token["line"] < 1:
            return "token %d has invalid line" % index
        if not isinstance(token.get("column"), int) or token["column"] < 1:
            return "token %d has invalid column" % index
    return None


def check_case(case, returncode, payload):
    token_error = validate_token_list(payload.get("tokens"))
    if token_error:
        return False, token_error
    if payload.get("stage") not in ("valid", "lexical", "syntax", "semantic"):
        return False, "unexpected stage %r" % payload.get("stage")
    if not isinstance(payload.get("diagnostics"), list):
        return False, "diagnostics is not a list"

    diagnostics = payload["diagnostics"]
    codes = [item.get("code", "") for item in diagnostics]

    if case["valid"]:
        if returncode != 0:
            return False, "expected valid, exit=%d" % returncode
        if payload.get("ok") is not True:
            return False, "expected ok=true, got %r" % payload.get("ok")
        if payload.get("stage") != "valid":
            return False, "expected stage=valid, got %r" % payload.get("stage")
        if not payload.get("syntax", {}).get("ok"):
            return False, "syntax.ok is not true"
        if not payload.get("semantics", {}).get("ok"):
            return False, "semantics.ok is not true"
        if not payload.get("ast"):
            return False, "valid query has no AST"
        if not payload.get("tokens"):
            return False, "valid query has no tokens"
        return True, ""

    if returncode not in (1, 2):
        return False, "expected failure exit 1/2, got %d" % returncode
    if payload.get("ok") is not False:
        return False, "expected ok=false, got %r" % payload.get("ok")

    expected = case["error"]
    if expected == "P":
        matched = any(code.startswith("P") for code in codes)
    elif expected in ("L", "S"):
        matched = any(code.startswith(expected) for code in codes)
    else:
        matched = expected in codes
    if not matched:
        return False, "expected %s, got %s" % (expected, ", ".join(codes) or "no diagnostics")

    if expected.startswith("L") and payload.get("stage") != "lexical":
        return False, "expected stage=lexical, got %r" % payload.get("stage")
    if expected == "P" and payload.get("stage") != "syntax":
        return False, "expected stage=syntax, got %r" % payload.get("stage")
    if expected.startswith("S") and payload.get("stage") != "semantic":
        return False, "expected stage=semantic, got %r" % payload.get("stage")
    if expected == "P" and payload.get("ast") is not None:
        return False, "syntax error should not produce an AST"
    return True, ""


def main():
    parser = argparse.ArgumentParser(description="SQL Mini Compiler JSON test suite")
    parser.add_argument("--cases", default=str(CASES_FILE))
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    if not COMPILER.is_file():
        print("Missing compiler binary: %s" % COMPILER)
        print("Build it with: powershell -ExecutionPolicy Bypass -File compiler/build.ps1")
        return 1

    with open(args.cases, "r", encoding="utf-8") as handle:
        cases = json.load(handle)["cases"]

    passed = 0
    failed = 0
    print("=== compiler suite (%d cases) ===" % len(cases))

    for case in cases:
        try:
            returncode, payload, stderr = run_case(case["sql"])
            ok, reason = check_case(case, returncode, payload)
        except Exception as exc:
            ok = False
            reason = "runner error: %s" % exc
            stderr = ""

        if ok:
            passed += 1
            if args.verbose:
                print("PASS %-4s %s" % (case["id"], case["note"]))
        else:
            failed += 1
            print("FAIL %-4s %s: %s" % (case["id"], case["note"], reason))
            if stderr:
                print("     stderr: %s" % stderr.strip()[:300])

    print("compiler: %d passed, %d failed" % (passed, failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
