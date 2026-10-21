#!/usr/bin/env python3
"""Update report fields in Word and export docs/PROJECT_REPORT.pdf."""

import os
from pathlib import Path

import win32com.client

ROOT = Path(__file__).resolve().parent.parent
DOCX = ROOT / "docs" / "PROJECT_REPORT.docx"
PDF = ROOT / "docs" / "PROJECT_REPORT.pdf"


def main():
    if not DOCX.exists():
        raise SystemExit("missing %s; run build_report_docx.py first" % DOCX)

    word = None
    document = None
    try:
        word = win32com.client.Dispatch("Word.Application")
        word.Visible = False
        word.DisplayAlerts = 0
        document = word.Documents.Open(str(DOCX), ReadOnly=False, Visible=False)

        document.Fields.Update()
        for index in range(1, document.TablesOfContents.Count + 1):
            document.TablesOfContents(index).Update()
        document.Fields.Update()
        document.Repaginate()
        document.Save()

        document.ExportAsFixedFormat(
            OutputFileName=str(PDF),
            ExportFormat=17,
            OpenAfterExport=False,
            OptimizeFor=0,
            Range=0,
            Item=0,
            IncludeDocProps=True,
            KeepIRM=True,
            CreateBookmarks=1,
            DocStructureTags=True,
            BitmapMissingFonts=True,
            UseISO19005_1=False,
        )
        print("wrote", PDF)
    finally:
        if document is not None:
            try:
                document.Close(False)
            except Exception:
                pass
        if word is not None:
            try:
                word.Quit()
            except Exception:
                pass


if __name__ == "__main__":
    main()
