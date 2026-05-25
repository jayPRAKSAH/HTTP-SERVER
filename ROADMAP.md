# HTTP Server Implementation Roadmap

## Phase 1: Core HTTP Server (Foundation)

✅ **Stage 1: Bind to Port 4221**

- [x] Create TCP socket
- [x] Bind to localhost:4221
- [x] Listen for connections
- Test: `nc localhost 4221` should connect

✅ **Stage 2: Respond with 200**

- [x] Accept client connection
- [x] Send HTTP/1.1 200 OK response
- Test: `curl http://localhost:4221/` returns 200

✅ **Stage 3: Extract URL Path**

- [x] Parse HTTP request line
- [x] Extract path from request
- [x] Return 200 for `/`, 404 for other paths
- Test:
  - `curl http://localhost:4221/` → 200
  - `curl http://localhost:4221/test` → 404

## Phase 2: Request/Response Enhancement

✅ **Stage 4: Connection Loop**

- [x] Wrap accept() in a loop
- [x] Server handles multiple sequential connections
- [x] Graceful shutdown (Ctrl+C)
- [x] Error handling for malformed requests
- [x] Proper resource cleanup (close only client_fd inside loop)
- Tests: ✅ Multiple requests work, ✅ Malformed requests return 400

### Stage 5: Respond with Body

- [x] Send response body with status line
- [x] Return different content for different paths
- [x] Add Content-Type header (text/html, application/json)
- [x] Add Content-Length header
- Example: `GET /` → HTML page, `GET /api` → JSON
- Tests: ✅ HTML response, ✅ JSON response, ✅ 404 with body

**Stage 6: Read and Parse Headers**

- [ ] Extract headers from HTTP request
- [ ] Use headers for logic (Content-Length, User-Agent, etc.)
- [ ] Echo back headers in response
- Test: `curl -H "X-Custom: value" http://localhost:4221/` shows header

**Stage 7: POST Request Body**

- [ ] Read request body for POST/PUT requests
- [ ] Parse `Content-Length` header
- [ ] Handle form data or JSON
- Test: `curl -X POST -d "data=test" http://localhost:4221/`

## Phase 3: Advanced Features

**Stage 8: File Serving**

- [ ] Serve static files from disk
- [ ] Return 404 for missing files
- [ ] Set correct Content-Type headers
- Test: Serve index.html, style.css, etc.

**Stage 9: Concurrent Connections**

- [ ] Use threads or async I/O
- [ ] Handle multiple clients simultaneously
- [ ] Thread-safe resource access
- Test: Multiple curl requests in parallel

**Stage 10: Persistent Connections (Keep-Alive)**

- [ ] Parse Connection header
- [ ] Keep socket open for multiple requests
- [ ] Close connection on "Connection: close"
- Test: Single TCP connection handles multiple HTTP requests

**Stage 11: HTTP Compression**

- [ ] Parse Accept-Encoding header
- [ ] Implement gzip compression
- [ ] Send Content-Encoding header
- Test: `curl --compressed http://localhost:4221/` receives compressed data

## Implementation Strategy

1. Implement Phase 1 & 2 sequentially
2. Test each stage before moving to next
3. Keep code modular (separate request parsing, response building)
4. Add logging for debugging

## Testing Approach

```bash
# Basic connectivity
nc localhost 4221

# HTTP responses
curl -v http://localhost:4221/
curl -v http://localhost:4221/test

# Headers
curl -v -H "X-Custom: value" http://localhost:4221/

# POST data
curl -X POST -d "key=value" http://localhost:4221/

# Concurrent (in background)
./http-server &
for i in {1..5}; do curl http://localhost:4221/ & done
```

## Current Status

- ✅ Stages 1-5 Complete
- ⏳ Stage 6 (Read and Parse Headers) - NEXT
