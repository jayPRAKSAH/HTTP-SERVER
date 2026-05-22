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

- The project is renamed to HTTP SERVER.
- The Git repository is initialized and linked to GitHub.
- The local folder matches the repo name.
- MinGW is configured to use `D:\MinGW`.
- The C++ workspace config now points to `g++.exe`.
- A build task is set up in VS Code for local compilation.

Test 1: ✅ PASSED — GET / returned 200 OK

Test 2 had a timeout because server exits after one connection. Let me test again with fresh server:

✅ Both tests PASSED!

| Test | Path       | Expected        | Got             | Status   |
| ---- | ---------- | --------------- | --------------- | -------- |
| 1    | `/`        | `200 OK`        | `200 OK`        | ✅ PASSED |
| 2    | `/abcdefg` | `404 Not Found` | `404 Not Found` | ✅ PASSED |


Next step: continue implementing theAdd connection loop — Server stays running, handles multiple requests
