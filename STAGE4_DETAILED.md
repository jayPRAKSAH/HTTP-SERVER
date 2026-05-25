# Stage 4: Connection Loop - Detailed Explanation

## What IS This Stage?

A **connection loop** is the fundamental pattern that makes a server a *server*. Right now, your code:
```
1. Start
2. Accept ONE connection
3. Respond to that ONE request
4. Exit (DONE)
```

With a connection loop, it becomes:
```
1. Start
2. Accept connection → Respond → Go back to step 2
3. Accept connection → Respond → Go back to step 2
4. Repeat until server is stopped
```

This is the difference between a **one-time program** and a **service**.

---

## What Will We ACHIEVE?

### Before Connection Loop:
```bash
$ ./http-server &
$ curl http://localhost:4221/     # Works ✓
$ curl http://localhost:4221/test # Fails ✗ (server already exited)
```

### After Connection Loop:
```bash
$ ./http-server &
$ curl http://localhost:4221/     # Works ✓
$ curl http://localhost:4221/test # Works ✓
$ curl http://localhost:4221/foo  # Works ✓
$ curl http://localhost:4221/bar  # Works ✓
# Keep making requests... all work!
```

**Real Achievement:** Server stays running and handles **infinite requests** until you stop it.

---

## WHY IS THIS IMPORTANT?

### 1. **Web Servers Must Be Long-Running**
Real web servers (Apache, Nginx, Node.js) run **forever** accepting requests. They don't exit after one client.

```
Web Server Architecture:
┌─────────────────────────────────────┐
│  Server Startup (Once)              │
│  - Create socket                    │
│  - Bind to port                     │
│  - Listen                           │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│  Connection Loop (FOREVER)          │
│  ┌───────────────────────────────┐  │
│  │ 1. Accept a client            │  │
│  │ 2. Read their request         │  │
│  │ 3. Send response              │  │
│  │ 4. Close connection           │  │
│  │ 5. Go to step 1               │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
```

### 2. **Multiple Clients Expect It**
When you use `curl` multiple times, you expect ONE server to serve all of them. Without a loop, you'd need to restart the server for each request!

### 3. **Foundation for Everything**
- Persistent connections (keep-alive)
- Concurrent connections (threads)
- Load balancing
- All require a running loop

---

## HOW WILL WE DO IT?

### Simple Pseudo-Code:
```cpp
// Setup (do ONCE)
create_socket()
bind_to_port()
listen()

// Main Loop (do FOREVER)
while (true) {
    client_fd = accept()           // Wait for client
    read_request(client_fd)        // Get their request
    send_response(client_fd)       // Send response
    close(client_fd)               // Close connection
    // Loop back to accept() - ready for NEXT client
}
```

### Key Insight:
The loop doesn't loop on **requests from same client**. It loops on **accepting new clients**.

---

## CURRENT CODE vs NEW CODE

### Current (NO loop):
```cpp
int client_fd = accept(server_fd, ...);  // Accept ONE client
// Handle the request
close(client_fd);
close(server_fd);  // Shut down server
return 0;          // Exit program
```

### New (WITH loop):
```cpp
while (true) {                           // Loop forever
    int client_fd = accept(server_fd, ...);  // Accept a client
    // Handle the request
    close(client_fd);                    // Close THIS client only
    // DON'T close server_fd - keep it open!
    // DON'T exit - loop back to accept more clients
}
```

---

## WHAT YOU'LL LEARN

### 1. **Event-Driven Programming**
The server doesn't do anything until something happens (a client connects). This is fundamental to all networking.

### 2. **Resource Management**
- Keep server socket OPEN (accept many clients)
- Close CLIENT socket (after serving that client)
- Never confuse the two!

### 3. **The Main Loop Pattern**
This pattern appears everywhere:
- Game loops: `while (game_running) { update(), render() }`
- Event loops: `while (has_events) { process_event() }`
- Server loops: `while (true) { accept_client() }`

### 4. **Server Lifecycle**
```
SETUP PHASE (once)        → RUNNING PHASE (forever) → SHUTDOWN (external signal)
create_socket()             while(true)                Ctrl+C to stop
bind()                        accept()                 or kill signal
listen()                      respond()                (we'll add this later)
                              loop_back()
```

### 5. **Blocking Operations**
- `accept()` **blocks** (waits) until a client connects
- This is efficient! CPU doesn't waste cycles checking "is there a client yet?"
- When client arrives, `accept()` returns immediately

---

## IMPLEMENTATION DETAILS

### What Changes in Code:
1. **Add `while (true)` loop** around accept/handle logic
2. **Keep `server_fd` open** - don't close it inside loop
3. **Close only `client_fd`** - the connection to this one client
4. **Remove the final `close(server_fd)`** - it never runs now (infinite loop)

### What Stays Same:
- Socket creation (before loop)
- Socket binding (before loop)
- Request parsing (inside loop, same code)
- Response sending (inside loop, same code)

### Testing the Loop:
```bash
# Terminal 1: Run server
./http-server

# Terminal 2: Make requests (while server is running)
curl http://localhost:4221/          # Request 1
curl http://localhost:4221/test      # Request 2
curl http://localhost:4221/foo       # Request 3
# All work! Server is still running
```

---

## LEARNING OUTCOMES

After this stage, you'll understand:
✅ How real servers work (they loop forever)
✅ The difference between server socket and client socket
✅ Event-driven architecture basics
✅ Why `accept()` blocking is good
✅ Resource lifecycle (create → setup → loop → shutdown)

---

## COMMON MISTAKES (Don't Do These!)

❌ **Closing server_fd inside the loop** - server dies after 1 client
❌ **Exiting the loop with `return 0`** - server stops working
❌ **Not closing client_fd** - resource leak, connection hangs
❌ **Trying to accept 2 clients at once** - need threading for that (later stage)

---

## NEXT: We'll Modify Code

Once you're ready, we'll:
1. Add `while (true)` around the accept/response logic
2. Test it handles multiple requests
3. Commit to GitHub

Does this make sense? Any questions before we code?
