#!/usr/bin/env python3
"""Create the single-engine architecture diagram used in the report and slides."""

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
OUTPUT = ROOT / "docs" / "diagrams" / "architecture.png"
FONT = Path(r"C:\Windows\Fonts\segoeui.ttf")
FONT_BOLD = Path(r"C:\Windows\Fonts\segoeuib.ttf")


def font(size, bold=False):
    path = FONT_BOLD if bold else FONT
    return ImageFont.truetype(str(path), size)


def centered(draw, box, lines, fill):
    left, top, right, bottom = box
    heights = []
    for line, line_font in lines:
        bbox = draw.textbbox((0, 0), line, font=line_font)
        heights.append(bbox[3] - bbox[1])
    gap = 8
    total = sum(heights) + gap * (len(lines) - 1)
    y = top + (bottom - top - total) / 2
    for (line, line_font), height in zip(lines, heights):
        width = draw.textlength(line, font=line_font)
        draw.text((left + (right - left - width) / 2, y), line, font=line_font, fill=fill)
        y += height + gap


def arrow(draw, start, end, color="#64748b", width=4):
    draw.line([start, end], fill=color, width=width)
    x1, y1 = start
    x2, y2 = end
    if x2 == x1:
        direction = 1 if y2 > y1 else -1
        points = [(x2, y2), (x2 - 11, y2 - 18 * direction), (x2 + 11, y2 - 18 * direction)]
    else:
        direction = 1 if x2 > x1 else -1
        points = [(x2, y2), (x2 - 18 * direction, y2 - 11), (x2 - 18 * direction, y2 + 11)]
    draw.polygon(points, fill=color)


def panel(draw, box, fill, outline, title, lines, title_font, body_font, title_color, body_color):
    draw.rounded_rectangle((box[0], box[1] + 8, box[2], box[3] + 8), radius=18, fill="#dbe3ee")
    draw.rounded_rectangle(box, radius=18, fill=fill, outline=outline, width=2)
    draw.rounded_rectangle((box[0], box[1], box[0] + 12, box[3]), radius=6, fill=outline)
    draw.text((box[0] + 34, box[1] + 18), title, font=title_font, fill=title_color)
    y = box[1] + 58
    for line in lines:
        draw.text((box[0] + 34, y), line, font=body_font, fill=body_color)
        y += 28


def main():
    width, height = 1600, 1120
    image = Image.new("RGB", (width, height), "#f8fafc")
    draw = ImageDraw.Draw(image)

    for x in range(0, width, 48):
        draw.line((x, 0, x, height), fill="#eef2f7")
    for y in range(0, height, 48):
        draw.line((0, y, width, y), fill="#eef2f7")

    title_font = font(40, True)
    subtitle_font = font(20)
    section_font = font(26, True)
    body_font = font(18)
    small_font = font(16)
    phase_font = font(19, True)

    draw.text((70, 34), "SQL Mini Compiler - Single-Engine Architecture", font=title_font, fill="#0f172a")
    draw.text((72, 84), "Browser  ->  local Python bridge  ->  Flex/Bison C compiler  ->  JSON",
              font=subtitle_font, fill="#475569")

    left, right = 70, 1530

    panel(draw, (left, 145, right, 265), "#0f172a", "#38bdf8",
          "Frontend  |  HTML + CSS + JavaScript",
          [
              "Editor, examples, AST tree, token table, diagnostics, schema, bridge status",
              "Presentation only: it does not lex, parse, or evaluate SQL",
          ],
          section_font, body_font, "#e2e8f0", "#cbd5e1")

    arrow(draw, (800, 273), (800, 322), color="#38bdf8")
    draw.text((820, 287), "fetch POST /api/analyze", font=small_font, fill="#0369a1")

    panel(draw, (left, 330, right, 438), "#ecfeff", "#0891b2",
          "HTTP boundary  |  127.0.0.1 only",
          [
              'Request:  {"source": "SELECT ..."}      Response:  {"ok": true, "tokens": [...], "ast": {...}, ...}',
          ],
          section_font, body_font, "#155e75", "#164e63")

    arrow(draw, (800, 446), (800, 495), color="#0891b2")
    draw.text((820, 460), "threaded HTTP handler", font=small_font, fill="#0e7490")

    panel(draw, (left, 503, right, 635), "#f5f3ff", "#7c3aed",
          "Python bridge  |  backend/server.py",
          [
              "Standard library only  |  static file server  |  JSON validation  |  subprocess.run(shell=False)",
              "Limits: 64 KiB request body  |  60,000 source characters  |  5 second compiler timeout",
          ],
          section_font, body_font, "#4c1d95", "#5b21b6")

    arrow(draw, (800, 643), (800, 694), color="#7c3aed")
    draw.text((820, 657), "stdin: SQL source", font=small_font, fill="#6d28d9")

    draw.rounded_rectangle((left, 702, right, 890), radius=18, fill="#f0fdf4", outline="#16a34a", width=3)
    draw.text((left + 28, 720), "Native compiler  |  compiler/bin/sqlc.exe --json", font=section_font, fill="#14532d")

    phase_boxes = [
        ("Flex", "tokens + positions", "#0f766e"),
        ("Bison", "LALR(1) grammar", "#1d4ed8"),
        ("AST", "C structures", "#6d28d9"),
        ("Semantic", "catalog + types", "#b45309"),
        ("JSON", "tokens, ast, diagnostics", "#047857"),
    ]
    box_width = 244
    gap = 28
    start_x = left + 40
    for index, (name, note, color) in enumerate(phase_boxes):
        x = start_x + index * (box_width + gap)
        draw.rounded_rectangle((x, 770, x + box_width, 862), radius=14, fill="#ffffff", outline=color, width=3)
        centered(draw, (x + 8, 776, x + box_width - 8, 820), [(name, phase_font)], color)
        centered(draw, (x + 8, 815, x + box_width - 8, 858), [(note, small_font)], "#475569")
        if index < len(phase_boxes) - 1:
            arrow(draw, (x + box_width + 4, 816), (x + box_width + gap - 4, 816), color="#94a3b8", width=3)

    draw.text((left + 28, 866), "One Flex token stream feeds both Bison and the JSON serializer.",
              font=small_font, fill="#166534")

    arrow(draw, (1500, 698), (1500, 270), color="#16a34a", width=4)
    draw.text((1370, 470), "JSON stdout", font=small_font, fill="#15803d")

    draw.rounded_rectangle((left, 930, right, 1042), radius=16, fill="#e0f2fe", outline="#38bdf8", width=2)
    centered(draw, (left + 20, 940, right - 20, 1032), [
        ("Shared contract", font(24, True)),
        ("tokens  |  syntax  |  ast  |  semantics  |  diagnostics", font(19)),
        ("The frontend renders the bridge response; there is no second compiler implementation.", font(17)),
    ], "#0c4a6e")

    draw.text((70, 1068),
              "Validation only: no query execution, no storage engine, no database connection.",
              font=font(19), fill="#475569")

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    image.save(OUTPUT, optimize=True)
    print("wrote", OUTPUT)


if __name__ == "__main__":
    main()
