#!/usr/bin/env python3
"""Export the PowerPoint deck to PDF using PowerPoint COM."""

from pathlib import Path

import pythoncom
import win32com.client

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "presentation" / "SQL_Mini_Compiler_Presentation.pptx"
TARGET = ROOT / "presentation" / "SQL_Mini_Compiler_Presentation.pdf"


def main():
    if not SOURCE.exists():
        raise SystemExit(f"missing deck: {SOURCE}")

    pythoncom.CoInitialize()
    app = win32com.client.Dispatch("PowerPoint.Application")
    presentation = app.Presentations.Open(str(SOURCE), True, False, False)
    try:
        if TARGET.exists():
            TARGET.unlink()
        presentation.SaveAs(str(TARGET), 32)
    finally:
        presentation.Close()
        app.Quit()
    print("wrote", TARGET)


if __name__ == "__main__":
    main()
