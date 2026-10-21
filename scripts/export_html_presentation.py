#!/usr/bin/env python3
"""Export the interactive HTML presentation into high-res PPTX and PDF."""

import os
import sys
import time
from pathlib import Path

from PIL import Image
from playwright.sync_api import sync_playwright
from pptx import Presentation
from pptx.util import Inches

ROOT = Path(__file__).resolve().parent.parent
SLIDES_HTML = ROOT / "presentation" / "slides.html"
ASSETS_DIR = ROOT / "presentation" / "assets"
PPTX_TARGET = ROOT / "presentation" / "SQL_Mini_Compiler_Presentation.pptx"
PDF_TARGET = ROOT / "presentation" / "SQL_Mini_Compiler_Presentation.pdf"

TOTAL_SLIDES = 16
WIDTH_INCHES = 13.333
HEIGHT_INCHES = 7.5


def capture_slides():
    ASSETS_DIR.mkdir(parents=True, exist_ok=True)
    slide_images = []

    file_url = SLIDES_HTML.resolve().as_uri()

    with sync_playwright() as playwright:
        browser = playwright.chromium.launch(headless=True)
        # 16:9 1920x1080 viewport with 2x device scale for ultra crisp text
        page = browser.new_page(viewport={"width": 1333, "height": 750}, device_scale_factor=2)
        page.goto(file_url, wait_until="networkidle")
        time.sleep(1)

        # Hide floating navigation bar during export
        page.evaluate("document.querySelector('.nav-bar').style.display = 'none';")

        deck_locator = page.locator("#deck")

        for index in range(1, TOTAL_SLIDES + 1):
            page.evaluate(f"window.__gotoSlide({index});")
            time.sleep(0.3)
            out_img = ASSETS_DIR / f"slide_{index:02d}.png"
            deck_locator.screenshot(path=str(out_img))
            slide_images.append(out_img)
            print(f"Captured slide {index:02d}/{TOTAL_SLIDES} -> {out_img.name}")

        browser.close()

    return slide_images


def build_pptx(slide_images):
    prs = Presentation()
    prs.slide_width = Inches(WIDTH_INCHES)
    prs.slide_height = Inches(HEIGHT_INCHES)
    blank_layout = prs.slide_layouts[6]

    for img_path in slide_images:
        slide = prs.slides.add_slide(blank_layout)
        slide.shapes.add_picture(
            str(img_path),
            Inches(0),
            Inches(0),
            width=Inches(WIDTH_INCHES),
            height=Inches(HEIGHT_INCHES),
        )

    prs.save(str(PPTX_TARGET))
    print(f"Saved PPTX -> {PPTX_TARGET}")


def build_pdf(slide_images):
    # Method 1: Try PowerPoint COM if available for native PDF export
    exported = False
    try:
        import pythoncom
        import win32com.client

        pythoncom.CoInitialize()
        app = win32com.client.Dispatch("PowerPoint.Application")
        presentation = app.Presentations.Open(str(PPTX_TARGET), True, False, False)
        try:
            if PDF_TARGET.exists():
                PDF_TARGET.unlink()
            presentation.SaveAs(str(PDF_TARGET), 32)
            print(f"Saved PDF via PowerPoint COM -> {PDF_TARGET}")
            exported = True
        finally:
            presentation.Close()
            app.Quit()
    except Exception as exc:
        print(f"PowerPoint COM export skipped: {exc}")

    # Method 2: Combine high-res PIL slide images into single PDF
    if not exported:
        pil_images = []
        for img_path in slide_images:
            img = Image.open(img_path).convert("RGB")
            pil_images.append(img)
        if pil_images:
            first = pil_images[0]
            rest = pil_images[1:]
            first.save(str(PDF_TARGET), save_all=True, append_images=rest, resolution=150.0)
            print(f"Saved PDF via PIL multi-page -> {PDF_TARGET}")


def main():
    print(f"Exporting HTML slides from: {SLIDES_HTML}")
    slide_images = capture_slides()
    build_pptx(slide_images)
    build_pdf(slide_images)
    print("Export complete!")


if __name__ == "__main__":
    main()
