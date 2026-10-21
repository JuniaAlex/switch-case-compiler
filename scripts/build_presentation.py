#!/usr/bin/env python3
"""Build the 16-slide project presentation with python-pptx."""

from pathlib import Path

from PIL import Image
from pptx import Presentation
from pptx.dml.color import RGBColor
from pptx.enum.shapes import MSO_SHAPE
from pptx.enum.text import MSO_ANCHOR, PP_ALIGN
from pptx.util import Inches, Pt

ROOT = Path(__file__).resolve().parent.parent
ASSETS = ROOT / "screenshots"
OUTPUT = ROOT / "presentation" / "SQL_Mini_Compiler_Presentation.pptx"

W, H = 13.333, 7.5
M = 0.6
BG = RGBColor(0x07, 0x0B, 0x16)
PANEL = RGBColor(0x0F, 0x17, 0x2A)
PANEL_2 = RGBColor(0x11, 0x1C, 0x33)
LINE = RGBColor(0x24, 0x32, 0x4A)
WHITE = RGBColor(0xE2, 0xE8, 0xF0)
BRIGHT = RGBColor(0xF8, 0xFA, 0xFC)
MUTED = RGBColor(0x94, 0xA3, 0xB8)
SLATE = RGBColor(0xCB, 0xD5, 0xE1)
TEAL = RGBColor(0x2D, 0xD4, 0xBF)
BLUE = RGBColor(0x60, 0xA5, 0xFA)
PURPLE = RGBColor(0xA7, 0x8B, 0xFA)
AMBER = RGBColor(0xFB, 0xBF, 0x24)
ROSE = RGBColor(0xFB, 0x71, 0x85)
GREEN = RGBColor(0x34, 0xD3, 0x99)

HEAD_FONT = "Segoe UI Semibold"
BODY_FONT = "Segoe UI"
MONO_FONT = "Consolas"


def rgb(hex_value):
    return RGBColor.from_string(hex_value)


def add_notes(slide, text):
    slide.notes_slide.notes_text_frame.text = text


def background(slide):
    fill = slide.background.fill
    fill.solid()
    fill.fore_color.rgb = BG


def rect(slide, x, y, w, h, fill=PANEL, line=LINE, radius=0.08, width=1.0):
    shape = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(x), Inches(y), Inches(w), Inches(h))
    shape.fill.solid()
    shape.fill.fore_color.rgb = fill
    if line is None:
        shape.line.fill.background()
    else:
        shape.line.color.rgb = line
        shape.line.width = Pt(width)
    try:
        shape.adjustments[0] = radius
    except Exception:
        pass
    shape.shadow.inherit = False
    return shape


def line(slide, x1, y1, x2, y2, color=LINE, width=1.0):
    connector = slide.shapes.add_connector(1, Inches(x1), Inches(y1), Inches(x2), Inches(y2))
    connector.line.color.rgb = color
    connector.line.width = Pt(width)


def textbox(slide, x, y, w, h, text="", font=BODY_FONT, size=14, color=WHITE,
            bold=False, italic=False, align=PP_ALIGN.LEFT, anchor=MSO_ANCHOR.TOP, spacing=1.0):
    box = slide.shapes.add_textbox(Inches(x), Inches(y), Inches(w), Inches(h))
    tf = box.text_frame
    tf.word_wrap = True
    tf.margin_left = 0
    tf.margin_right = 0
    tf.margin_top = 0
    tf.margin_bottom = 0
    tf.vertical_anchor = anchor
    p = tf.paragraphs[0]
    p.alignment = align
    p.line_spacing = spacing
    run = p.add_run()
    run.text = text
    run.font.name = font
    run.font.size = Pt(size)
    run.font.bold = bold
    run.font.italic = italic
    run.font.color.rgb = color
    return box


def paragraph(tf, text, font=BODY_FONT, size=14, color=WHITE, bold=False,
              align=PP_ALIGN.LEFT, space_after=2, first=False, italic=False):
    p = tf.paragraphs[0] if first else tf.add_paragraph()
    p.alignment = align
    p.space_after = Pt(space_after)
    p.line_spacing = 1.08
    run = p.add_run()
    run.text = text
    run.font.name = font
    run.font.size = Pt(size)
    run.font.bold = bold
    run.font.italic = italic
    run.font.color.rgb = color
    return p


def header(slide, eyebrow, title, subtitle=None, number=None):
    textbox(slide, M, 0.32, 8.0, 0.24, eyebrow.upper(), size=9.5, color=TEAL, bold=True)
    textbox(slide, M, 0.58, 11.8, 0.55, title, font=HEAD_FONT, size=28, color=BRIGHT, bold=True)
    if subtitle:
        textbox(slide, M, 1.12, 11.9, 0.3, subtitle, size=12.5, color=MUTED)
    line(slide, M, 1.55, W - M, 1.55, color=LINE, width=0.8)
    if number is not None:
        footer(slide, number)


def footer(slide, number):
    line(slide, M, 7.12, W - M, 7.12, color=LINE, width=0.8)
    textbox(slide, M, 7.18, 6.5, 0.2, "SQL Mini Compiler  |  Compiler Design  |  2026-27",
            size=8, color=MUTED)
    textbox(slide, W - M - 1.4, 7.18, 1.4, 0.2, "%02d / 16" % number,
            size=8, color=MUTED, align=PP_ALIGN.RIGHT)


def chip(slide, x, y, w, h, text, fill=PANEL_2, border=LINE, color=WHITE,
         size=10, bold=False, font=BODY_FONT):
    shape = rect(slide, x, y, w, h, fill=fill, line=border, radius=0.25)
    tf = shape.text_frame
    tf.word_wrap = False
    tf.margin_left = Inches(0.04)
    tf.margin_right = Inches(0.04)
    tf.margin_top = 0
    tf.margin_bottom = 0
    tf.vertical_anchor = MSO_ANCHOR.MIDDLE
    p = tf.paragraphs[0]
    p.alignment = PP_ALIGN.CENTER
    run = p.add_run()
    run.text = text
    run.font.name = font
    run.font.size = Pt(size)
    run.font.bold = bold
    run.font.color.rgb = color
    return shape


def flow_chips(slide, chips, x, y, max_width, height=0.4, gap=0.1, colors=None):
    cursor = x
    row_y = y
    for index, text in enumerate(chips):
        width = max(0.7, min(2.3, 0.12 * len(text) + 0.36))
        if cursor + width > x + max_width:
            cursor = x
            row_y += height + gap
        fill, border, color = (colors[index] if colors and index < len(colors) else (PANEL_2, LINE, WHITE))
        chip(slide, cursor, row_y, width, height, text, fill=fill, border=border, color=color, size=9.5)
        cursor += width + gap
    return row_y + height


def bullet_list(slide, x, y, w, h, items, size=12, color=SLATE, bullet_color=TEAL,
                space_after=6, numbered=False):
    box = slide.shapes.add_textbox(Inches(x), Inches(y), Inches(w), Inches(h))
    tf = box.text_frame
    tf.word_wrap = True
    tf.margin_left = 0
    tf.margin_right = 0
    tf.margin_top = 0
    tf.margin_bottom = 0
    for index, item in enumerate(items):
        p = tf.paragraphs[0] if index == 0 else tf.add_paragraph()
        p.space_after = Pt(space_after)
        p.line_spacing = 1.08
        prefix = ("%d.  " % (index + 1)) if numbered else "\u2022  "
        run = p.add_run()
        run.text = prefix
        run.font.name = BODY_FONT
        run.font.size = Pt(size)
        run.font.bold = True
        run.font.color.rgb = bullet_color
        if isinstance(item, tuple):
            lead, rest = item
            run = p.add_run()
            run.text = lead
            run.font.name = BODY_FONT
            run.font.size = Pt(size)
            run.font.bold = True
            run.font.color.rgb = color
            run = p.add_run()
            run.text = rest
            run.font.name = BODY_FONT
            run.font.size = Pt(size)
            run.font.color.rgb = color
        else:
            run = p.add_run()
            run.text = item
            run.font.name = BODY_FONT
            run.font.size = Pt(size)
            run.font.color.rgb = color
    return box


def code_panel(slide, x, y, w, h, text, size=11, title=None, title_color=TEAL):
    rect(slide, x, y, w, h, fill=RGBColor(0x08, 0x10, 0x20), line=LINE, radius=0.04)
    if title:
        textbox(slide, x + 0.18, y + 0.12, w - 0.36, 0.2, title, size=8.5, color=title_color, bold=True)
        top = y + 0.38
    else:
        top = y + 0.16
    box = slide.shapes.add_textbox(Inches(x + 0.18), Inches(top), Inches(w - 0.36), Inches(h - (top - y) - 0.1))
    tf = box.text_frame
    tf.word_wrap = False
    tf.margin_left = 0
    tf.margin_right = 0
    tf.margin_top = 0
    tf.margin_bottom = 0
    for index, line_text in enumerate(text.splitlines()):
        p = tf.paragraphs[0] if index == 0 else tf.add_paragraph()
        p.space_after = Pt(0)
        p.line_spacing = 1.02
        run = p.add_run()
        run.text = line_text if line_text else " "
        run.font.name = MONO_FONT
        run.font.size = Pt(size)
        run.font.color.rgb = RGBColor(0xD7, 0xE3, 0xF4)
    return box


def picture_fit(slide, path, x, y, max_w, max_h, caption=None):
    with Image.open(path) as image:
        iw, ih = image.size
    ratio = iw / ih
    width = max_w
    height = width / ratio
    if height > max_h:
        height = max_h
        width = height * ratio
    left = x + (max_w - width) / 2
    top = y + (max_h - height) / 2
    slide.shapes.add_picture(str(path), Inches(left), Inches(top), width=Inches(width), height=Inches(height))
    if caption:
        textbox(slide, x, y + max_h + 0.06, max_w, 0.2, caption, size=8.5, color=MUTED,
                align=PP_ALIGN.CENTER)


def build_cover(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    rect(slide, 0.0, 0.0, 0.18, 7.5, fill=TEAL, line=None, radius=0.0)

    textbox(slide, M, 0.55, 7.0, 0.22, "COMPILER DESIGN  |  PROJECT REPORT", size=10, color=TEAL, bold=True)
    textbox(slide, M, 0.92, 6.6, 1.55, "SQL Mini\nCompiler", font=HEAD_FONT, size=43, color=BRIGHT, bold=True, spacing=0.9)
    textbox(slide, M, 2.62, 6.35, 0.75, "A single-engine front-end for a restricted SQL subset.",
            font=HEAD_FONT, size=19, color=WHITE)
    textbox(slide, M, 3.42, 6.25, 0.55,
            "Flex scanner, Bison LALR(1) parser, C semantic checks, and a local Python bridge.",
            size=11.5, color=SLATE)

    chip(slide, M, 4.32, 1.25, 0.38, "Flex + Bison", size=9.5)
    chip(slide, M + 1.35, 4.32, 0.92, 0.38, "C11", size=9.5)
    chip(slide, M + 2.37, 4.32, 1.42, 0.38, "Python bridge", size=9.5)
    chip(slide, M + 3.89, 4.32, 1.55, 0.38, "HTML / CSS / JS", size=9.5)

    rect(slide, 7.55, 1.02, 5.05, 5.55, fill=PANEL, line=LINE, radius=0.05)
    textbox(slide, 7.85, 1.32, 4.45, 0.22, "ONE QUERY, ONE PIPELINE", size=9, color=TEAL, bold=True)
    code_panel(slide, 7.82, 1.68, 4.5, 2.15,
               "SELECT s.name, c.title\n"
               "FROM students s, enrollments e,\n"
               "     courses c\n"
               "WHERE s.student_id = e.student_id\n"
               "  AND e.course_id = c.course_id\n"
               "ORDER BY s.name ASC;",
               size=9.5)

    textbox(slide, 7.85, 4.12, 4.35, 0.24, "SOURCE -> TOKENS -> AST -> CHECKS", size=8.5, color=MUTED, bold=True)
    flow_chips(slide, ["Source", "Frontend", "Bridge", "Compiler", "JSON"],
               7.85, 4.45, 4.3, height=0.38, gap=0.08)
    textbox(slide, 7.85, 5.45, 4.35, 0.65,
            "One native compiler is the only implementation of the language.",
            size=11, color=SLATE, italic=True)

    textbox(slide, M, 6.35, 8.0, 0.3, "79 / 79 compiler cases   |   8 / 8 bridge tests",
            size=11, color=TEAL, bold=True)
    footer(slide, 1)


def build_motivation(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "01  |  Motivation", "Why build a SQL front-end?",
           "The query is declarative; the validation problem is classical compiler work.", 2)

    bullet_list(slide, M, 1.88, 5.7, 3.8, [
        ("Correctness: ", "a query can be syntactically perfect and still reference a missing column."),
        ("Structure: ", "lexing, parsing, AST construction, and name resolution are visible compiler phases."),
        ("Schema awareness: ", "aliases and unqualified columns force real scope and ambiguity handling."),
        ("A usable demonstration: ", "the browser should show the native compiler, not a lookalike."),
    ], size=12.5, space_after=12)

    rect(slide, 6.95, 1.85, 5.1, 4.35, fill=PANEL, line=LINE, radius=0.05)
    textbox(slide, 7.25, 2.1, 4.5, 0.24, "THE DESIGN QUESTION", size=9, color=TEAL, bold=True)
    textbox(slide, 7.25, 2.48, 4.55, 1.25,
            "How can a browser demo stay honest when the compiler is written in C?",
            font=HEAD_FONT, size=20, color=BRIGHT, bold=True)
    textbox(slide, 7.25, 3.95, 4.5, 0.85,
            "Answer: keep one native compiler and put a small, bounded bridge in front of it.",
            size=12, color=SLATE)
    chip(slide, 7.25, 5.08, 1.35, 0.4, "No reimplementation", size=8.5, fill=PANEL_2, border=TEAL)
    chip(slide, 8.72, 5.08, 1.2, 0.4, "No SQL execution", size=8.5)
    chip(slide, 10.04, 5.08, 1.55, 0.4, "One JSON contract", size=8.5)

    textbox(slide, M, 6.35, 11.2, 0.3,
            "Goal: make the demonstrated behavior identical to the tested behavior.",
            size=11, color=TEAL, bold=True)


def build_scope(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "02  |  Scope", "A deliberately small SQL subset",
           "Small enough to specify and test; rich enough to exercise a complete front-end.", 3)

    rect(slide, M, 1.85, 5.85, 4.45, fill=PANEL, line=LINE, radius=0.05)
    textbox(slide, M + 0.28, 2.08, 5.2, 0.24, "IN SCOPE", size=9, color=TEAL, bold=True)
    bullet_list(slide, M + 0.28, 2.48, 5.15, 3.55, [
        "SELECT with DISTINCT and star projection",
        "FROM with table aliases",
        "WHERE with arithmetic, comparisons, AND / OR / NOT",
        "LIKE, IN, BETWEEN, IS NULL",
        "ORDER BY with aliases, ordinals, ASC / DESC",
        "Four-table demo catalog",
        "JSON output, local bridge, browser UI",
        "79 compiler cases + 8 bridge tests",
    ], size=11.5, space_after=8)

    rect(slide, 7.05, 1.85, 5.0, 4.45, fill=PANEL, line=LINE, radius=0.05)
    textbox(slide, 7.33, 2.08, 4.4, 0.24, "OUT OF SCOPE", size=9, color=AMBER, bold=True)
    bullet_list(slide, 7.33, 2.48, 4.35, 3.55, [
        "JOIN ... ON and subqueries",
        "GROUP BY, HAVING, UNION",
        "Aggregates and user-defined functions",
        "INSERT / UPDATE / DELETE / DDL",
        "Query execution and storage",
        "Remote deployment or authentication",
        "Cost-based optimization",
    ], size=11.5, space_after=9, bullet_color=AMBER)

    textbox(slide, M, 6.4, 11.2, 0.3,
            "The boundary is explicit: this is a compiler front-end, not a database engine.",
            size=11, color=TEAL, bold=True)


def build_architecture(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "03  |  Architecture", "One engine behind a small boundary",
           "The browser presents results; it does not parse SQL.", 4)

    stages = [
        ("SOURCE", "SQL text"),
        ("FRONTEND", "HTML / CSS / JS"),
        ("HTTP", "POST /api/analyze"),
        ("BRIDGE", "server.py"),
        ("COMPILER", "sqlc.exe"),
        ("OUTPUT", "JSON response"),
    ]
    x0, y0, width, gap = M, 1.9, 1.72, 0.18
    for index, (label, detail) in enumerate(stages):
        x = x0 + index * (width + gap)
        fill = PANEL_2 if index in (3, 4) else PANEL_2
        border = TEAL if index in (3, 4) else LINE
        rect(slide, x, y0, width, 0.86, fill=fill, line=border, radius=0.12)
        textbox(slide, x + 0.1, y0 + 0.11, width - 0.2, 0.22, label, size=8, color=TEAL, bold=True, align=PP_ALIGN.CENTER)
        textbox(slide, x + 0.08, y0 + 0.38, width - 0.16, 0.34, detail, size=9, color=WHITE, bold=True, align=PP_ALIGN.CENTER)
        if index < len(stages) - 1:
            line(slide, x + width + 0.02, y0 + 0.43, x + width + gap - 0.02, y0 + 0.43, color=TEAL, width=1.5)

    rect(slide, M, 3.25, 5.9, 1.28, fill=PANEL, line=LINE, radius=0.06)
    textbox(slide, M + 0.22, 3.45, 5.4, 0.23, "SINGLE SOURCE OF TRUTH", size=8.5, color=TEAL, bold=True)
    textbox(slide, M + 0.22, 3.76, 5.45, 0.6,
            "The Flex token stream feeds Bison and the JSON token table. No second scanner.",
            size=10.5, color=SLATE)

    rect(slide, 6.95, 3.25, 5.1, 1.28, fill=PANEL, line=LINE, radius=0.06)
    textbox(slide, 7.17, 3.45, 4.65, 0.23, "BOUNDED BRIDGE", size=8.5, color=TEAL, bold=True)
    textbox(slide, 7.17, 3.76, 4.65, 0.6,
            "64 KiB body, 60k source characters, 5 s timeout, shell=False, 127.0.0.1.",
            size=10.5, color=SLATE)

    rect(slide, M, 4.85, 11.18, 1.18, fill=RGBColor(0x0B, 0x16, 0x2B), line=LINE, radius=0.05)
    textbox(slide, M + 0.24, 5.05, 10.7, 0.23, "REQUEST / RESPONSE", size=8.5, color=TEAL, bold=True)
    textbox(slide, M + 0.24, 5.36, 10.65, 0.42,
            'request: {"source": "..."}    ->    response: {ok, stage, tokens, diagnostics, syntax, ast, semantics}',
            font=MONO_FONT, size=9.5, color=WHITE)

    textbox(slide, M, 6.38, 11.2, 0.3,
            "Analysis failures return HTTP 200 with ok=false; transport failures use 4xx/5xx.",
            size=10.5, color=TEAL, bold=True)


def build_language(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "04  |  Language", "The accepted query shape",
           "A compact grammar with explicit precedence and aliases.", 5)

    code_panel(slide, M, 1.85, 6.15, 2.9,
               "SELECT [DISTINCT] select_list\n"
               "FROM table_list\n"
               "[WHERE boolean_expression]\n"
               "[ORDER BY order_list];\n"
               "\n"
               "select_item : expr [AS alias]\n"
               "table_ref   : table [AS alias]\n"
               "order_item  : expr [ASC | DESC]",
               size=10.5, title="CONCRETE SYNTAX")

    rect(slide, 7.2, 1.85, 4.85, 1.35, fill=PANEL, line=LINE, radius=0.05)
    textbox(slide, 7.45, 2.03, 4.3, 0.24, "PROJECTION", size=8.5, color=TEAL, bold=True)
    textbox(slide, 7.45, 2.34, 4.35, 0.72,
            "*  |  expression  |  expression AS alias\n"
            "DISTINCT applies to the select list.",
            size=10.5, color=SLATE)

    rect(slide, 7.2, 3.35, 4.85, 1.4, fill=PANEL, line=LINE, radius=0.05)
    textbox(slide, 7.45, 3.53, 4.3, 0.24, "PREDICATES", size=8.5, color=TEAL, bold=True)
    textbox(slide, 7.45, 3.84, 4.35, 0.78,
            "=  <>  <  <=  >  >=\nLIKE  IN  BETWEEN  IS NULL  AND  OR  NOT",
            font=MONO_FONT, size=9.5, color=WHITE)

    textbox(slide, M, 5.02, 11.2, 0.23, "PRECEDENCE, LOW TO HIGH", size=8.5, color=MUTED, bold=True)
    flow_chips(slide, ["OR", "AND", "NOT", "comparisons", "+ -", "* /", "unary", "() / column / literal"],
               M, 5.33, 11.2, height=0.42, gap=0.09)

    textbox(slide, M, 6.0, 11.2, 0.45,
            "Precedence is encoded in the grammar, not patched with conflict declarations.",
            size=10.5, color=TEAL, bold=True)


def build_lexer(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "05  |  Lexical analysis", "Flex records the only token stream",
           "Keywords, identifiers, literals, comments, operators, and positions.", 6)

    bullet_list(slide, M, 1.88, 5.55, 3.75, [
        ("Keywords: ", "SELECT, FROM, WHERE, ORDER, DISTINCT, ASC, DESC, ..."),
        ("Values: ", "integers, floats, quoted strings, NULL, TRUE, FALSE."),
        ("Comments: ", "--, //, and /* ... */ forms."),
        ("Positions: ", "one-based line and column on every token."),
        ("Recording: ", "EMIT appends the token to the JSON list before returning it to Bison."),
    ], size=11.3, space_after=10)

    code_panel(slide, 6.72, 1.85, 5.35, 3.85,
               "SELECT   KEYWORD    1:1\n"
               "name     IDENT      1:8\n"
               ",        COMMA      1:12\n"
               "age      IDENT      1:14\n"
               "FROM     KEYWORD    1:18\n"
               "students IDENT      1:23\n"
               "\n"
               "-- one token stream, recorded in EMIT",
               size=9.2, title="TOKEN TABLE  (ABRIDGED)")

    rect(slide, M, 5.75, 11.18, 0.62, fill=RGBColor(0x0B, 0x16, 0x2B), line=LINE, radius=0.06)
    textbox(slide, M + 0.22, 5.92, 10.7, 0.3,
            "The JSON token table and the parser input are the same tokens. No second scanner.",
            size=10.5, color=WHITE, bold=True)


def build_parser(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "06  |  Syntax analysis", "Bison builds the tree",
           "One LALR(1) grammar with stratified precedence and zero conflicts.", 7)

    code_panel(slide, M, 1.84, 5.85, 3.9,
               "query\n"
               "  : SELECT opt_distinct select_list\n"
               "    FROM table_list opt_where opt_order\n"
               "  ;\n"
               "\n"
               "select_list\n"
               "  : select_list ',' select_item\n"
               "  | select_item\n"
               "  ;\n"
               "\n"
               "expr\n"
               "  : expr OR expr\n"
               "  | expr AND expr\n"
               "  | expr '+' expr\n"
               "  | '(' expr ')'\n"
               "  | IDENT\n"
               "  ;",
               size=8.9, title="PARSER.Y  (ABRIDGED)")

    rect(slide, 6.95, 1.84, 5.1, 1.62, fill=PANEL, line=LINE, radius=0.05)
    textbox(slide, 7.18, 2.02, 4.6, 0.23, "LALR(1)  |  BISON", size=8.5, color=TEAL, bold=True)
    textbox(slide, 7.18, 2.34, 4.62, 1.0,
            "%expect 0  |  %locations  |  AST actions\n"
            "No conflict declarations and no hand-written parser in the active path.",
            size=10.5, color=SLATE)

    rect(slide, 6.95, 3.6, 5.1, 2.02, fill=PANEL, line=LINE, radius=0.05)
    textbox(slide, 7.18, 3.78, 4.6, 0.23, "GRAMMAR LEVELS", size=8.5, color=TEAL, bold=True)
    textbox(slide, 7.18, 4.1, 4.62, 1.35,
            "OR  ->  AND  ->  NOT  ->  predicate\n"
            "    ->  additive  ->  multiplicative\n"
            "    ->  unary  ->  primary",
            font=MONO_FONT, size=9.5, color=WHITE)

    textbox(slide, M, 5.95, 11.2, 0.3,
            "Left-recursive list rules keep the grammar compact and the AST ordered.",
            size=10.5, color=TEAL, bold=True)


def build_ast(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "07  |  Intermediate representation", "The AST is the contract",
           "Every node keeps its source position; the JSON tree mirrors the native tree.", 8)

    code_panel(slide, M, 1.85, 6.2, 4.0,
               'Query\n'
               '  distinct: false\n'
               '  SelectList\n'
               '    SelectItem\n'
               '      expr: Column("s", "name")\n'
               '      alias: null\n'
               '  FromList\n'
               '    TableRef("students", "s")\n'
               '  Where: BinaryOp("=", ...)\n'
               '  OrderList\n'
               '    OrderItem(Column("s", "name"), ASC)',
               size=9.4, title="PARSE TREE  (SCHEMATIC)")

    bullet_list(slide, 7.0, 1.88, 5.05, 3.85, [
        ("Query node: ", "distinct flag, select list, from list, WHERE, order list."),
        ("Select item: ", "star, optional expression, optional alias."),
        ("Table reference: ", "table name and optional alias."),
        ("Expressions: ", "literals, columns, unary, binary, LIKE, IN, BETWEEN, IS NULL."),
        ("Locations: ", "line and column survive all the way to diagnostics."),
    ], size=11.2, space_after=10)

    rect(slide, M, 5.98, 11.18, 0.48, fill=RGBColor(0x0B, 0x16, 0x2B), line=LINE, radius=0.06)
    textbox(slide, M + 0.22, 6.1, 10.7, 0.25,
            "Bison actions build one tree; the CLI text printer and the JSON serializer are two views of it.",
            size=10.5, color=WHITE, bold=True)


def build_semantic(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "08  |  Semantic analysis", "Catalog-aware checks after parsing",
           "Names, aliases, types, and ordering rules are validated against a fixed schema.", 9)

    code_panel(slide, M, 1.85, 5.7, 3.95,
               "students\n"
               "  student_id INTEGER  name TEXT\n"
               "  age INTEGER  department TEXT\n"
               "  cgpa FLOAT  city TEXT\n"
               "\n"
               "courses\n"
               "  course_id INTEGER  title TEXT\n"
               "  credits INTEGER  department TEXT\n"
               "\n"
               "enrollments\n"
               "  enrollment_id  student_id  course_id\n"
               "  grade  semester\n"
               "\n"
               "departments\n"
               "  department  hod  building  budget",
               size=8.8, title="DEMO CATALOG")

    bullet_list(slide, 6.85, 1.88, 5.2, 3.95, [
        ("Resolution: ", "alias if present, otherwise table name; unqualified columns search the FROM scope."),
        ("Ambiguity: ", "two tables exposing the same column produce S004."),
        ("Types: ", "arithmetic needs numerics; AND / OR / NOT need booleans; LIKE needs text."),
        ("WHERE: ", "must be boolean; a bare column is rejected."),
        ("ORDER BY: ", "accepts columns, select aliases, or a one-based ordinal."),
    ], size=10.8, space_after=9)

    chip(slide, M, 5.98, 2.3, 0.42, "S001 unknown name", size=8.5, fill=PANEL_2, border=TEAL)
    chip(slide, M + 2.42, 5.98, 2.35, 0.42, "S004 ambiguous column", size=8.5, fill=PANEL_2, border=TEAL)
    chip(slide, M + 4.9, 5.98, 2.15, 0.42, "S005 type mismatch", size=8.5, fill=PANEL_2, border=TEAL)
    chip(slide, M + 7.18, 5.98, 2.35, 0.42, "S008 ordinal range", size=8.5, fill=PANEL_2, border=TEAL)


def build_compiler(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "09  |  Native implementation", "Flex + Bison + C: one compiler",
           "The active path is a single C11 executable with a stable JSON mode.", 10)

    bullet_list(slide, M, 1.86, 5.55, 3.85, [
        ("lexer.l: ", "scanner, token positions, JSON recording."),
        ("parser.y: ", "LALR grammar and AST-building actions."),
        ("ast.c / ast.h: ", "constructors, cleanup, text printing."),
        ("semantic.c: ", "scope, name resolution, type checks."),
        ("catalog.c: ", "four-table demo schema."),
        ("json_output.c: ", "the only JSON writer."),
        ("main.c: ", "CLI, --json, phase orchestration."),
    ], size=10.7, space_after=6)

    picture_fit(slide, ASSETS / "02_tokens.png", 6.62, 1.78, 5.52, 3.65,
                caption="Native token table rendered by the frontend")

    rect(slide, M, 5.95, 11.2, 0.62, fill=RGBColor(0x0B, 0x16, 0x2B), line=LINE, radius=0.06)
    textbox(slide, M + 0.22, 6.1, 10.75, 0.27,
            "Build: flex + bison in WSL  ->  MinGW GCC  ->  compiler/bin/sqlc.exe  ->  --json",
            font=MONO_FONT, size=9.8, color=WHITE, bold=True)


def build_bridge(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "10  |  Python bridge", "A narrow, bounded HTTP boundary",
           "The bridge validates requests and invokes the compiler as a subprocess.", 11)

    bullet_list(slide, M, 1.88, 5.45, 4.0, [
        ("GET /api/health: ", "compiler availability for the UI badge."),
        ("POST /api/analyze: ", "accepts JSON with a string source."),
        ("Limits: ", "64 KiB body, 60,000 source characters, 5-second timeout."),
        ("Isolation: ", "subprocess with shell=False; source over stdin."),
        ("Mapping: ", "analysis failures return HTTP 200; transport errors use 4xx/5xx."),
        ("Locality: ", "binds to 127.0.0.1 by default."),
    ], size=10.9, space_after=9)

    code_panel(slide, 6.7, 1.84, 5.38, 4.35,
               "POST /api/analyze\n"
               '{"source": "SELECT name FROM students"}\n'
               "\n"
               "200 OK\n"
               "{\n"
               '  "ok": true,\n'
               '  "stage": "valid",\n'
               '  "tokens": [ ... ],\n'
               '  "diagnostics": [],\n'
               '  "syntax": {"ok": true},\n'
               '  "ast": { ... },\n'
               '  "semantics": {"ok": true}\n'
               "}",
               size=9.4, title="REQUEST / RESPONSE")

    rect(slide, M, 6.15, 11.18, 0.42, fill=RGBColor(0x0B, 0x16, 0x2B), line=LINE, radius=0.06)
    textbox(slide, M + 0.22, 6.25, 10.7, 0.24,
            "No SQL parsing in Python: the bridge is transport and validation only.",
            size=10.5, color=WHITE, bold=True)


def build_frontend(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "11  |  Frontend", "A presentation layer, not a second compiler",
           "Tabs expose the native JSON: parse tree, tokens, diagnostics, and schema.", 12)

    bullet_list(slide, M, 1.88, 5.35, 4.05, [
        ("Editor: ", "examples, keyboard shortcut, and clear run action."),
        ("Tabs: ", "AST, tokens, diagnostics, and schema."),
        ("Diagnostics: ", "code, phase, message, line, and column."),
        ("Bridge badge: ", "health status from /api/health."),
        ("Serving: ", "the bridge serves the page over HTTP from the same origin."),
        ("No fallback: ", "the UI does not contain a lexer, parser, or semantic checker."),
    ], size=10.9, space_after=8)

    picture_fit(slide, ASSETS / "01_valid_ast.png", 6.45, 1.78, 5.68, 4.25,
                caption="Valid query, parse tree, and status")

    chip(slide, M, 6.2, 1.45, 0.38, "Ctrl+Enter", size=8.5)
    chip(slide, M + 1.58, 6.2, 1.2, 0.38, "Examples", size=8.5)
    chip(slide, M + 2.91, 6.2, 1.35, 0.38, "Health badge", size=8.5)
    chip(slide, M + 4.39, 6.2, 1.35, 0.38, "Four tabs", size=8.5)


def build_testing(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "12  |  Validation", "79 language cases, 8 bridge tests",
           "The runner checks the JSON contract, not a substring of terminal output.", 13)

    stats = [
        ("30", "valid queries", TEAL),
        ("25", "semantic errors", AMBER),
        ("20", "syntax errors", ROSE),
        ("4", "lexical errors", PURPLE),
    ]
    for index, (value, label, color) in enumerate(stats):
        x = M if index % 2 == 0 else M + 2.62
        y = 1.88 if index < 2 else 2.98
        rect(slide, x, y, 2.4, 0.92, fill=PANEL, line=LINE, radius=0.07)
        textbox(slide, x + 0.18, y + 0.12, 0.85, 0.5, value, font=HEAD_FONT, size=25, color=color, bold=True)
        textbox(slide, x + 1.0, y + 0.27, 1.25, 0.35, label, size=9, color=SLATE, bold=True)

    rect(slide, M, 4.12, 5.02, 1.3, fill=PANEL, line=LINE, radius=0.06)
    textbox(slide, M + 0.22, 4.3, 4.55, 0.22, "BRIDGE SUITE", size=8.5, color=TEAL, bold=True)
    textbox(slide, M + 0.22, 4.6, 4.55, 0.65,
            "8 / 8 passed: health, static page, valid, semantic, syntax, invalid JSON, bad source, oversized source.",
            size=9.8, color=SLATE)

    rect(slide, M, 5.62, 5.02, 0.62, fill=PANEL_2, line=TEAL, radius=0.06)
    textbox(slide, M + 0.22, 5.77, 4.6, 0.3, "87 checks total  |  0 failures", size=11, color=WHITE, bold=True)

    picture_fit(slide, ASSETS / "07_test_results.png", 6.15, 1.78, 5.95, 4.5,
                caption="Compiler and bridge suites, 0 failures")


def build_results(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "13  |  Results", "A single implementation, fully validated",
           "The active path is small enough to audit and broad enough to demonstrate every phase.", 14)

    cards = [
        ("79 / 79", "compiler cases", "30 valid, 25 semantic, 20 syntax, 4 lexical."),
        ("8 / 8", "bridge tests", "Transport, validation, and error mapping."),
        ("0", "duplicated parsers", "The frontend has no compiler logic."),
    ]
    for index, (value, label, detail) in enumerate(cards):
        x = M + index * 3.75
        rect(slide, x, 1.95, 3.5, 2.15, fill=PANEL, line=LINE, radius=0.06)
        textbox(slide, x + 0.24, 2.14, 3.0, 0.6, value, font=HEAD_FONT, size=28, color=TEAL, bold=True)
        textbox(slide, x + 0.24, 2.78, 3.0, 0.25, label.upper(), size=8.5, color=WHITE, bold=True)
        textbox(slide, x + 0.24, 3.12, 3.0, 0.75, detail, size=9.8, color=SLATE)

    rect(slide, M, 4.45, 11.18, 1.25, fill=RGBColor(0x0B, 0x16, 0x2B), line=LINE, radius=0.06)
    textbox(slide, M + 0.24, 4.65, 10.7, 0.23, "HISTORICAL PROTOTYPES", size=8.5, color=AMBER, bold=True)
    textbox(slide, M + 0.24, 4.94, 10.6, 0.55,
            "The pure-C and browser implementations are archived for reference only. They are not built, tested, or claimed as active results.",
            size=10, color=SLATE)

    textbox(slide, M, 6.25, 11.2, 0.3,
            "One lexer, one parser, one semantic checker, one JSON contract.",
            size=11, color=TEAL, bold=True)


def build_limits(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "14  |  Limits and next steps", "What this front-end does not do",
           "The boundary is deliberate; the extension points are clear.", 15)

    rect(slide, M, 1.85, 5.85, 4.45, fill=PANEL, line=LINE, radius=0.05)
    textbox(slide, M + 0.28, 2.08, 5.2, 0.24, "CURRENT LIMITS", size=9, color=AMBER, bold=True)
    bullet_list(slide, M + 0.28, 2.48, 5.15, 3.55, [
        "No JOIN ... ON, subqueries, GROUP BY, or HAVING",
        "Fixed four-table catalog",
        "No query execution or storage engine",
        "No authentication or remote deployment",
        "No performance benchmark",
        "No editor syntax highlighting",
    ], size=11.3, space_after=10, bullet_color=AMBER)

    rect(slide, 7.05, 1.85, 5.0, 4.45, fill=PANEL, line=LINE, radius=0.05)
    textbox(slide, 7.33, 2.08, 4.4, 0.24, "NATURAL EXTENSIONS", size=9, color=TEAL, bold=True)
    bullet_list(slide, 7.33, 2.48, 4.35, 3.55, [
        "Add JOIN ... ON and GROUP BY ... HAVING",
        "Load the catalog from configuration",
        "Add visitor API and typed AST export",
        "Property-based tests and fuzzing",
        "Inline diagnostics in the editor",
        "One-command packaging for Windows and Linux",
    ], size=11.3, space_after=10)

    textbox(slide, M, 6.4, 11.2, 0.3,
            "The project stops at a validated front-end; execution is a separate system.",
            size=11, color=TEAL, bold=True)


def build_conclusion(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[6])
    background(slide)
    header(slide, "15  |  Conclusion", "One compiler, one contract, one demo",
           "The project covers the full front-end and keeps every layer honest.", 16)

    bullet_list(slide, M, 1.9, 6.0, 3.95, [
        ("Language: ", "a precise subset with aliases, predicates, and ordering."),
        ("Front-end: ", "Flex scanner, Bison LALR(1) parser, explicit AST."),
        ("Semantics: ", "catalog-aware resolution and type checks with stable codes."),
        ("Interface: ", "JSON, a bounded Python bridge, and a browser presentation layer."),
        ("Validation: ", "79 compiler cases and 8 bridge tests, all passing."),
    ], size=11.2, space_after=10)

    rect(slide, 7.15, 1.9, 4.9, 3.95, fill=PANEL, line=TEAL, radius=0.06)
    textbox(slide, 7.45, 2.18, 4.3, 0.23, "DELIVERED", size=8.5, color=TEAL, bold=True)
    textbox(slide, 7.45, 2.55, 4.25, 0.62, "One native compiler", font=HEAD_FONT, size=20, color=BRIGHT, bold=True)
    textbox(slide, 7.45, 3.3, 4.25, 1.1,
            "Flex + Bison + C, with JSON output that the bridge and frontend consume directly.",
            size=10.5, color=SLATE)
    line(slide, 7.45, 4.25, 11.7, 4.25, color=LINE, width=0.8)
    textbox(slide, 7.45, 4.42, 4.2, 1.15,
            "Report  ·  Presentation  ·  Screenshots\nTest suites  ·  Build scripts  ·  Archive",
            size=10.5, color=WHITE, bold=True)

    textbox(slide, M, 6.28, 11.2, 0.34,
            "One lexer  |  one parser  |  one semantic checker  |  one JSON contract",
            size=11, color=TEAL, bold=True)


def build():
    prs = Presentation()
    prs.slide_width = Inches(W)
    prs.slide_height = Inches(H)
    builders = [
        build_cover, build_motivation, build_scope, build_architecture,
        build_language, build_lexer, build_parser, build_ast,
        build_semantic, build_compiler, build_bridge, build_frontend,
        build_testing, build_results, build_limits, build_conclusion,
    ]
    for builder in builders:
        builder(prs)
    out = ROOT / "presentation" / "SQL_Mini_Compiler_Presentation.pptx"
    out.parent.mkdir(parents=True, exist_ok=True)
    prs.save(out)
    print("wrote %s" % out)


if __name__ == "__main__":
    build()
