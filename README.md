# HTTP SERVER

[![progress-banner](https://backend.codecrafters.io/progress/http-server/518ef942-76ba-4a94-8e81-564d06ddd140)](https://app.codecrafters.io/users/codecrafters-bot?r=2qF)

This is a starting point for C++ solutions to the
["Build Your Own HTTP server" Challenge](https://app.codecrafters.io/courses/http-server/overview).

[HTTP](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol) is the
protocol that powers the web. In this challenge, you'll build a HTTP/1.1 server
that is capable of serving multiple clients.

Along the way you'll learn about TCP servers,
[HTTP request syntax](https://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html),
and more.


## What’s done so far

### Setup & repository

- The project is renamed to HTTP SERVER and the Git repository is initialized.
- Local workspace configured for MinGW and `g++` with a VS Code build task.

### Roadmap progress (completed)

- Stage 1 — Bind to port 4221: ✅ implemented (socket, bind, listen)
- Stage 2 — Respond with 200: ✅ implemented (returns 200 OK)
- Stage 3 — Extract URL path: ✅ implemented (returns 200 for `/`, 404 otherwise)
- Stage 4 — Connection loop: ✅ implemented (server stays running, handles sequential connections, graceful shutdown)
- Stage 5 — Respond with body: ✅ implemented (sends response body, `Content-Type`, `Content-Length`)
- Stage 6 — Read and parse headers: ✅ implemented (parses headers into a map, extracts `User-Agent` and `Content-Length`)

### Current behavior

- `GET /` returns an HTML page with optional echoed headers and `User-Agent`.
- `GET /api` returns JSON `{ "message": "Hello from API", "status": "success" }`.
- Unknown paths return `404 Not Found` with an HTML body.
- Malformed requests receive `400 Bad Request` and the server logs errors.
- Basic curl tests for `/` and `/abcdefg` have passed locally.

### Next steps

- Stage 7 — Read and handle POST request bodies: next priority.
- Later: static file serving, concurrent clients (threads/async), persistent connections (keep-alive), and HTTP compression.

If you want, I can now implement Stage 7 (POST body handling) next.
