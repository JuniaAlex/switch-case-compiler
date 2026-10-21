#!/usr/bin/env python3
import json
import subprocess
import sys
import textwrap
import time
import urllib.error
import urllib.request
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont
from playwright.sync_api import sync_playwright

ROOT = Path(__file__).resolve().parent.parent
SCREENSHOTS = ROOT / "screenshots"
COMPILER = ROOT / "compiler" / "bin" / "sqlc.exe"
CONSOLA = Path(r"C:\Windows\Fonts\consola.ttf")
CONSOLA_BOLD = Path(r"C:\Windows\Fonts\consolab.ttf")
BRIDGE_PORT = 8767
BRIDGE_URL = "http://127.0.0.1:%d" % BRIDGE_PORT

VALID_SQL = (
    "SELECT s.name, e.grade\n"
    "FROM students s, enrollments e\n"
    "WHERE s.student_id = e.student_id\n"
    "ORDER BY s.name ASC;"
)

TOKEN_SQL = (
    "SELECT DISTINCT cgpa FROM students "
    "WHERE age >= 18 ORDER BY cgpa DESC;"
)

JSON_SQL = "SELECT name FROM students WHERE age > 20 ORDER BY name;"

OLD_SCREENSHOTS = [
    "01_pure_c_valid.png",
    "02_lex_yacc_valid.png",
    "03_c_semantic_error.png",
    "04_c_syntax_error.png",
    "05_web_valid_ast.png",
    "06_web_semantic_error.png",
    "07_web_syntax_error.png",
    "08_web_tokens.png",
    "09_web_schema.png",
    "10_test_results.png",
]


def run_command(command, cwd):
    result = subprocess.run(
        command,
        cwd=str(cwd),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=120,
    )
    return result.stdout


def line_color(line):
    stripped = line.strip()
    if stripped.startswith("Result: VALID"):
        return "#059669"
    if stripped.startswith("Result: INVALID"):
        return "#dc2626"
    if stripped.startswith("[S") or stripped.startswith("[P") or stripped.startswith("[L"):
        return "#dc2626"
    if stripped.startswith("[1]") or stripped.startswith("[2]") or stripped.startswith("[3]") or stripped.startswith("[4]"):
        return "#2563eb"
    if stripped.startswith("SQL Mini Compiler"):
        return "#0f172a"
    if stripped.startswith("Query:"):
        return "#475569"
    if stripped.startswith("#") or stripped.startswith("---"):
        return "#94a3b8"
    if "passed" in stripped and "failed" in stripped:
        return "#059669" if " 0 failed" in stripped else "#dc2626"
    if stripped.startswith("Ran ") or stripped == "OK":
        return "#059669"
    if stripped.startswith("{") or stripped.startswith("}") or stripped.startswith('"'):
        if '"ok": true' in stripped or '"stage": "valid"' in stripped:
            return "#059669"
        if '"stage": "semantic"' in stripped or '"stage": "syntax"' in stripped:
            return "#dc2626"
        return "#0f172a"
    return "#334155"


def render_terminal(title, prompt, command_line, output_lines, destination):
    font = ImageFont.truetype(str(CONSOLA), 15)
    bold = ImageFont.truetype(str(CONSOLA_BOLD if CONSOLA_BOLD.exists() else CONSOLA), 15)
    char_width = font.getlength("M")
    lines = [("prompt", prompt)]
    command_chunks = textwrap.wrap(
        command_line, width=116, break_long_words=False, break_on_hyphens=False
    ) or [""]
    for index, chunk in enumerate(command_chunks):
        lines.append(("command", ("  " if index else "") + chunk))
    for line in output_lines:
        raw = line.rstrip("\r\n")
        chunks = [raw[index:index + 122] for index in range(0, max(1, len(raw)), 122)]
        for chunk in chunks:
            lines.append(("output", chunk))

    max_length = max(len(line[1]) for line in lines)
    max_length = min(max_length, 122)
    width = int(max(940, max_length * char_width + 52))
    title_height = 38
    line_height = 23
    height = title_height + 24 + len(lines) * line_height + 22

    # Clean Light Theme Window (Zero Drop Shadows)
    card_pad = 16
    total_w = width + card_pad * 2
    total_h = height + card_pad * 2

    image = Image.new("RGB", (total_w, total_h), "#f8fafc")
    draw = ImageDraw.Draw(image)

    # Window Card (White with 1px border)
    x0, y0 = card_pad, card_pad
    x1, y1 = total_w - card_pad - 1, total_h - card_pad - 1
    draw.rounded_rectangle((x0, y0, x1, y1), radius=10, fill="#ffffff", outline="#e2e8f0", width=1)

    # Window Titlebar
    title_y1 = y0 + title_height
    draw.rounded_rectangle((x0, y0, x1, title_y1), radius=10, fill="#f8fafc")
    draw.rectangle((x0, title_y1 - 10, x1, title_y1), fill="#f8fafc")
    draw.line([(x0, title_y1), (x1, title_y1)], fill="#e2e8f0", width=1)

    # Clean subtle window dots
    for index, (dot_fill, dot_border) in enumerate([("#fca5a5", "#f87171"), ("#fde68a", "#fbbf24"), ("#a7f3d0", "#34d399")]):
        center_x = x0 + 20 + index * 18
        draw.ellipse((center_x - 5, y0 + 14, center_x + 5, y0 + 24), fill=dot_fill, outline=dot_border, width=1)

    draw.text((x0 + 82, y0 + 11), title, font=bold, fill="#0f172a")

    y = title_y1 + 18
    for kind, line in lines:
        if kind == "prompt":
            draw.text((x0 + 24, y), line, font=bold, fill="#0f172a")
        elif kind == "command":
            draw.text((x0 + 24, y), line, font=font, fill="#475569")
        else:
            draw.text((x0 + 24, y), line, font=font, fill=line_color(line))
        y += line_height
    image.save(destination, optimize=True)
    print("wrote", destination)


def terminal_prompt(cwd):
    return "PS " + str(cwd)


def wait_for_bridge(process):
    for _ in range(60):
        if process.poll() is not None:
            raise RuntimeError("bridge exited before becoming ready")
        try:
            with urllib.request.urlopen(BRIDGE_URL + "/api/health", timeout=1):
                return
        except (urllib.error.URLError, TimeoutError):
            time.sleep(0.1)
    raise RuntimeError("bridge did not become ready")


def make_terminal_screenshots():
    compiler_output = run_command([sys.executable, str(ROOT / "tests" / "run_tests.py")], ROOT)
    bridge_output = run_command(
        [sys.executable, "-m", "unittest", "discover", "-s", "tests", "-p", "test_*.py", "-v"],
        ROOT,
    )
    combined = compiler_output.splitlines() + [""] + bridge_output.splitlines()
    cleaned = []
    for line in combined:
        marker = " [bridge] "
        cleaned.append(line.split(marker)[0] if marker in line else line)
    render_terminal(
        "SQL Mini Compiler - validation",
        terminal_prompt(ROOT),
        "python tests\\run_tests.py  ;  python -m unittest discover -s tests -p \"test_*.py\"",
        cleaned,
        SCREENSHOTS / "07_test_results.png",
    )

    raw = run_command([str(COMPILER), "--json", "-e", JSON_SQL], ROOT)
    payload = json.loads(raw)
    tokens = payload["tokens"]
    sample = tokens[0]
    sample_two = tokens[1]
    abridged = [
        "{",
        '  "ok": %s,' % str(payload["ok"]).lower(),
        '  "stage": "%s",' % payload["stage"],
        '  "tokens": [',
        '    {"index": 1, "type": "%s", "lexeme": "%s", "line": %d, "column": %d},' % (
            sample["type"], sample["lexeme"], sample["line"], sample["column"]),
        '    {"index": 2, "type": "%s", "lexeme": "%s", "line": %d, "column": %d},' % (
            sample_two["type"], sample_two["lexeme"], sample_two["line"], sample_two["column"]),
        "    ... %d more tokens" % max(0, len(tokens) - 2),
        "  ],",
        '  "diagnostics": [],',
        '  "syntax": {"ok": true, "stage": null, "error": null},',
        '  "ast": {',
        '    "kind": "query",',
        '    "selectList": [ ... ],',
        '    "fromList": [ ... ],',
        '    "where": { ... },',
        '    "orderList": [ ... ]',
        "  },",
        '  "semantics": {"ok": true, "diagnostics": []}',
        "}",
    ]
    render_terminal(
        "sqlc.exe --json (abridged)",
        terminal_prompt(ROOT / "compiler"),
        '.\\bin\\sqlc.exe --json -e "' + JSON_SQL + '"',
        abridged,
        SCREENSHOTS / "08_json_output.png",
    )


def make_frontend_screenshots():
    process = subprocess.Popen(
        [sys.executable, str(ROOT / "backend" / "server.py"), "--port", str(BRIDGE_PORT)],
        cwd=str(ROOT),
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    try:
        wait_for_bridge(process)
        with sync_playwright() as playwright:
            browser = playwright.chromium.launch(headless=True)
            page = browser.new_page(viewport={"width": 1500, "height": 1100}, device_scale_factor=1)
            page.goto(BRIDGE_URL + "/", wait_until="networkidle")
            page.wait_for_function("document.querySelector('#status-badge').textContent === 'VALID'")

            page.fill("#sql-input", VALID_SQL)
            page.click("#validate-button")
            page.wait_for_function("document.querySelector('#status-badge').textContent === 'VALID'")
            page.wait_for_timeout(500)
            page.screenshot(path=str(SCREENSHOTS / "01_valid_ast.png"), full_page=True)
            print("wrote", SCREENSHOTS / "01_valid_ast.png")

            page.click('[data-tab="tokens"]')
            page.wait_for_timeout(400)
            page.screenshot(path=str(SCREENSHOTS / "02_tokens.png"), full_page=True)
            print("wrote", SCREENSHOTS / "02_tokens.png")

            page.click('[data-example="ambiguous"]')
            page.wait_for_function("document.querySelector('#status-badge').textContent === 'SEMANTIC ERROR'")
            page.wait_for_timeout(500)
            page.screenshot(path=str(SCREENSHOTS / "03_semantic_error.png"), full_page=True)
            print("wrote", SCREENSHOTS / "03_semantic_error.png")

            page.click('[data-example="syntax"]')
            page.wait_for_function("document.querySelector('#status-badge').textContent === 'SYNTAX ERROR'")
            page.wait_for_timeout(500)
            page.screenshot(path=str(SCREENSHOTS / "04_syntax_error.png"), full_page=True)
            print("wrote", SCREENSHOTS / "04_syntax_error.png")

            page.click('[data-example="lexical"]')
            page.wait_for_function("document.querySelector('#status-badge').textContent === 'LEXICAL ERROR'")
            page.wait_for_timeout(500)
            page.screenshot(path=str(SCREENSHOTS / "05_lexical_error.png"), full_page=True)
            print("wrote", SCREENSHOTS / "05_lexical_error.png")

            page.fill("#sql-input", VALID_SQL)
            page.click("#validate-button")
            page.wait_for_function("document.querySelector('#status-badge').textContent === 'VALID'")
            page.wait_for_timeout(400)
            page.click('[data-tab="schema"]')
            page.wait_for_timeout(300)
            page.screenshot(path=str(SCREENSHOTS / "06_schema.png"), full_page=True)
            print("wrote", SCREENSHOTS / "06_schema.png")

            browser.close()
    finally:
        process.terminate()
        process.wait(timeout=10)


def main():
    SCREENSHOTS.mkdir(parents=True, exist_ok=True)
    for name in OLD_SCREENSHOTS:
        path = SCREENSHOTS / name
        if path.exists():
            path.unlink()
    make_terminal_screenshots()
    make_frontend_screenshots()
    for old in SCREENSHOTS.glob("_web_*.png"):
        old.unlink()
    print("Screenshots complete")


if __name__ == "__main__":
    main()
