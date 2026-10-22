from __future__ import annotations

import argparse
import json
import mimetypes
import os
import subprocess
import sys
import threading
import time
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit

ROOT = Path(__file__).resolve().parents[1]
FRONTEND = ROOT / "frontend"
COMPILER_NAMES = ["switchc.exe", "sqlc.exe"] if os.name == "nt" else ["switchc", "sqlc"]
COMPILER = next(
    (ROOT / "compiler" / "bin" / name for name in COMPILER_NAMES if (ROOT / "compiler" / "bin" / name).is_file()),
    ROOT / "compiler" / "bin" / ("switchc.exe" if os.name == "nt" else "switchc")
)
MAX_BODY_BYTES = 65536
MAX_SOURCE_BYTES = 60000
COMPILER_TIMEOUT_SECONDS = 5


class BridgeError(Exception):
    def __init__(self, status: int, code: str, message: str) -> None:
        super().__init__(message)
        self.status = status
        self.code = code
        self.message = message


def run_compiler(source: str) -> dict:
    if not COMPILER.is_file():
        raise BridgeError(503, "COMPILER_MISSING", f"Compiler binary not found: {COMPILER}")

    creation_flags = 0
    if os.name == "nt":
        creation_flags = getattr(subprocess, "CREATE_NO_WINDOW", 0x08000000)

    try:
        completed = subprocess.run(
            [str(COMPILER), "--json"],
            input=source,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=COMPILER_TIMEOUT_SECONDS,
            shell=False,
            creationflags=creation_flags,
            cwd=str(ROOT),
        )
    except subprocess.TimeoutExpired as exc:
        raise BridgeError(504, "COMPILER_TIMEOUT", "Compiler timed out after 5 seconds") from exc
    except OSError as exc:
        raise BridgeError(503, "COMPILER_START_FAILED", f"Could not start compiler: {exc}") from exc

    if completed.returncode not in (0, 1, 2):
        detail = (completed.stderr or "").strip()
        message = f"Compiler failed with exit code {completed.returncode}"
        if detail:
            message = f"{message}: {detail}"
        raise BridgeError(500, "COMPILER_FAILED", message)

    try:
        result = json.loads(completed.stdout)
    except json.JSONDecodeError as exc:
        raise BridgeError(500, "INVALID_COMPILER_OUTPUT", "Compiler did not return valid JSON") from exc

    if not isinstance(result, dict):
        raise BridgeError(500, "INVALID_COMPILER_OUTPUT", "Compiler JSON must be an object")
    return result


class BridgeHandler(BaseHTTPRequestHandler):
    server_version = "SwitchCompilerBridge/1.0"

    def log_message(self, fmt: str, *args) -> None:
        sys.stderr.write("[bridge] " + (fmt % args) + "\n")

    def send_json(self, status: int, payload: dict) -> None:
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def send_error_json(self, status: int, code: str, message: str) -> None:
        self.send_json(status, {"error": {"code": code, "message": message}})

    def send_frontend_file(self, relative_path: str) -> None:
        target = (FRONTEND / relative_path).resolve()
        try:
            target.relative_to(FRONTEND.resolve())
        except ValueError:
            self.send_error_json(404, "NOT_FOUND", "File not found")
            return
        if not target.is_file():
            self.send_error_json(404, "NOT_FOUND", "File not found")
            return

        content_type = mimetypes.guess_type(str(target))[0] or "application/octet-stream"
        body = target.read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def read_json_body(self) -> dict:
        try:
            length = int(self.headers.get("Content-Length", "0"))
        except ValueError as exc:
            raise BridgeError(400, "INVALID_LENGTH", "Invalid Content-Length header") from exc
        if length <= 0:
            raise BridgeError(400, "EMPTY_BODY", "Request body is empty")
        if length > MAX_BODY_BYTES:
            raise BridgeError(413, "BODY_TOO_LARGE", "Request body exceeds 64 KiB")
        raw = self.rfile.read(length)
        try:
            payload = json.loads(raw.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError) as exc:
            raise BridgeError(400, "INVALID_JSON", "Request body must be valid UTF-8 JSON") from exc
        if not isinstance(payload, dict):
            raise BridgeError(400, "INVALID_PAYLOAD", "Request body must be a JSON object")
        return payload

    def do_GET(self) -> None:
        path = urlsplit(self.path).path
        if path == "/api/health":
            self.send_json(
                200,
                {
                    "ok": True,
                    "compiler": str(COMPILER),
                    "compilerAvailable": COMPILER.is_file(),
                },
            )
            return
        if path == "/":
            self.send_frontend_file("index.html")
            return
        self.send_frontend_file(path.lstrip("/"))

    def do_POST(self) -> None:
        path = urlsplit(self.path).path
        if path != "/api/analyze":
            self.send_error_json(404, "NOT_FOUND", "Endpoint not found")
            return
        try:
            payload = self.read_json_body()
            source = payload.get("source")
            if not isinstance(source, str):
                raise BridgeError(400, "INVALID_SOURCE", "Field 'source' must be a string")
            if len(source.encode("utf-8")) > MAX_SOURCE_BYTES:
                raise BridgeError(413, "SOURCE_TOO_LARGE", "Source exceeds 60 KiB")
            result = run_compiler(source)
        except BridgeError as exc:
            self.send_error_json(exc.status, exc.code, exc.message)
            return
        except Exception as exc:
            self.send_error_json(500, "INTERNAL_ERROR", f"Unexpected bridge error: {exc}")
        else:
            self.send_json(200, result)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Local HTTP bridge for the Switch Statement Compiler")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8000)
    parser.add_argument("--open", action="store_true", help="Open the frontend in the default browser")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv if argv is not None else sys.argv[1:])
    server = ThreadingHTTPServer((args.host, args.port), BridgeHandler)
    url = f"http://{args.host}:{args.port}/"
    print(f"Switch Statement Compiler bridge listening at {url}")
    print(f"Compiler: {COMPILER} ({'found' if COMPILER.is_file() else 'missing'})")
    if args.open:
        threading.Thread(target=lambda: (time.sleep(0.5), webbrowser.open(url)), daemon=True).start()
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping bridge.")
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
