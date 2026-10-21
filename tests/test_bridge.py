import json
import sys
import threading
import unittest
import urllib.error
import urllib.request
from http.server import ThreadingHTTPServer
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from backend.server import BridgeHandler


def request(base, path, method="GET", payload=None):
    data = None
    headers = {}
    if payload is not None:
        data = json.dumps(payload).encode("utf-8")
        headers["Content-Type"] = "application/json"
    req = urllib.request.Request(base + path, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req, timeout=15) as response:
            return response.status, response.headers.get("Content-Type", ""), response.read()
    except urllib.error.HTTPError as exc:
        return exc.code, exc.headers.get("Content-Type", ""), exc.read()


class BridgeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.server = ThreadingHTTPServer(("127.0.0.1", 0), BridgeHandler)
        cls.thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()
        cls.base = "http://127.0.0.1:%d" % cls.server.server_address[1]

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()
        cls.server.server_close()
        cls.thread.join(timeout=5)

    def test_health(self):
        status, content_type, body = request(self.base, "/api/health")
        self.assertEqual(status, 200)
        self.assertIn("application/json", content_type)
        payload = json.loads(body)
        self.assertTrue(payload["ok"])
        self.assertTrue(payload["compilerAvailable"])

    def test_static_frontend(self):
        status, content_type, body = request(self.base, "/")
        self.assertEqual(status, 200)
        self.assertIn("text/html", content_type)
        self.assertIn(b"SQL Mini Compiler", body)

    def test_valid_query(self):
        status, _, body = request(
            self.base,
            "/api/analyze",
            "POST",
            {"source": "SELECT s.name FROM students s WHERE s.age > 20 ORDER BY s.name;"},
        )
        self.assertEqual(status, 200)
        payload = json.loads(body)
        self.assertTrue(payload["ok"])
        self.assertEqual(payload["stage"], "valid")
        self.assertTrue(payload["syntax"]["ok"])
        self.assertTrue(payload["semantics"]["ok"])
        self.assertTrue(payload["ast"])
        self.assertGreater(len(payload["tokens"]), 0)
        for index, token in enumerate(payload["tokens"], start=1):
            self.assertEqual(token["index"], index)
            self.assertGreaterEqual(token["line"], 1)
            self.assertGreaterEqual(token["column"], 1)

    def test_semantic_error(self):
        status, _, body = request(
            self.base,
            "/api/analyze",
            "POST",
            {"source": "SELECT missing FROM students;"},
        )
        self.assertEqual(status, 200)
        payload = json.loads(body)
        self.assertFalse(payload["ok"])
        self.assertEqual(payload["stage"], "semantic")
        self.assertEqual(payload["diagnostics"][0]["code"], "S003")
        self.assertTrue(payload["ast"])

    def test_syntax_error(self):
        status, _, body = request(
            self.base,
            "/api/analyze",
            "POST",
            {"source": "SELECT name FROM"},
        )
        self.assertEqual(status, 200)
        payload = json.loads(body)
        self.assertFalse(payload["ok"])
        self.assertEqual(payload["stage"], "syntax")
        self.assertIsNone(payload["ast"])
        self.assertTrue(payload["diagnostics"][0]["code"].startswith("P"))

    def test_invalid_json(self):
        req = urllib.request.Request(
            self.base + "/api/analyze",
            data=b"{",
            headers={"Content-Type": "application/json"},
            method="POST",
        )
        with self.assertRaises(urllib.error.HTTPError) as context:
            urllib.request.urlopen(req, timeout=15)
        self.assertEqual(context.exception.code, 400)
        payload = json.loads(context.exception.read())
        self.assertEqual(payload["error"]["code"], "INVALID_JSON")

    def test_non_string_source(self):
        status, _, body = request(self.base, "/api/analyze", "POST", {"source": 42})
        self.assertEqual(status, 400)
        self.assertEqual(json.loads(body)["error"]["code"], "INVALID_SOURCE")

    def test_source_too_large(self):
        status, _, body = request(self.base, "/api/analyze", "POST", {"source": "x" * 60001})
        self.assertEqual(status, 413)
        self.assertEqual(json.loads(body)["error"]["code"], "SOURCE_TOO_LARGE")


if __name__ == "__main__":
    unittest.main()
