#!/usr/bin/env python3
import os, sys
method = os.environ.get("REQUEST_METHOD", "")
query = os.environ.get("QUERY_STRING", "")
length = int(os.environ.get("CONTENT_LENGTH") or 0)
body = sys.stdin.read(length) if length > 0 else ""
print("Content-Type: text/html")
print()
print("<!DOCTYPE html><html><head><meta charset='utf-8'><title>CGI</title>"
      "<link rel='stylesheet' href='/style.css'></head><body><main>")
print("<h1>Hello from CGI</h1>")
print(f"<p>Method: <code>{method}</code></p>")
print(f"<p>Query string: <code>{query}</code></p>")
print(f"<p>Body ({length} bytes): <code>{body}</code></p>")
print("<p><a href='/cgi.html'>Back</a></p></main></body></html>")
