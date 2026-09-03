# HTTP Server in C

A work-in-progress systems programming project for learning how a network
server is built from the bottom up in C.

The project combines networking, concurrency, memory management, and the
boundary between operating-system primitives and application-level protocols.
It is intentionally small and close to the POSIX APIs rather than being a
production-ready HTTP implementation.

## Status

The main pieces currently exist, but they are not yet wired into one complete
request/response server:

- A TCP server can create, configure, bind, and listen on an IPv4 socket.
- An incremental, line-oriented HTTP request parser reads from a socket into a
  bounded buffer and parses the request line and headers.
- HTTP responses can be built with a status, dynamically allocated headers,
  and a body. There is a helper for JSON responses.
- A fixed-size thread pool uses a bounded ring queue, a mutex, and a condition
  variable to coordinate worker threads.
- A small logging module reports messages and `errno` values.

The accept-and-handle loop in `src/main.c` is currently commented out. Parsed
requests are not yet dispatched to handlers, and the executable currently
serves as a thread-pool smoke test rather than a usable HTTP server. There is
also no automated test suite yet.

## Current Runtime Behavior

Running the program currently:

1. Binds an IPv4 TCP listener to `0.0.0.0:8080`.
2. Starts 10 worker threads.
3. Enqueues 20 sample tasks; each task sleeps briefly and prints `Hello`.
4. Stops the pool and closes the listener.

It does not accept client connections or return HTTP responses yet.

## Building

The project requires a C compiler, GNU Make, POSIX threads, and a Linux-like
environment.

```sh
make          # build bin/server.out
make debug    # build debug/server.out with debug symbols and no optimization
make run      # build and run the current smoke-test executable
```

Build settings include GNU C17, strict compiler warnings treated as errors,
dependency generation, and `pthread` support. Generated object files,
dependency files, and executables are ignored by Git.

Available cleanup targets are `make clean`, `make clean-debug`, and
`make clean-all`.

## Project Layout

| Path | Purpose |
| --- | --- |
| `src/server/` | TCP socket setup and client acceptance primitives |
| `src/http/` | Request parsing and response construction |
| `src/threadpool/` | Worker threads and the bounded task queue |
| `src/log/` | Basic stderr logging helpers |
| `src/main.c` | Executable entry point and current experiments |
| `include/` | Public headers for each module |
| `Makefile` | Normal, debug, run, and cleanup targets |

## What This Project Explores

### Networking

- Creating and configuring sockets with `socket`, `setsockopt`, `bind`, and
  `listen`
- Treating TCP as a byte stream instead of assuming one `read` equals one
  request
- Buffering incomplete input and finding HTTP CRLF line boundaries
- Converting raw bytes into bounded C structures

### Concurrency

- Implementing a producer/consumer queue with a fixed capacity
- Coordinating workers with `pthread_mutex_t` and `pthread_cond_t`
- Handling worker startup, blocking, wakeup, and shutdown
- Thinking about ownership and lifetime when work is passed between threads

### Systems Programming

- Checking system-call and allocation failures
- Keeping protocol limits explicit in data structures
- Serializing structured data back into bytes for a socket
- Observing how small changes in buffer management and synchronization affect
  behavior

## Planned Directions

Likely next steps include:

- Restore the accept loop and enqueue client handling in the thread pool.
- Connect request parsing to routing and response generation.
- Handle request bodies, `Content-Length`, connection lifetimes, and malformed
  input more completely.
- Improve error paths, resource cleanup, and graceful server shutdown.
- Add unit and integration tests for parsing, responses, sockets, and the
  thread pool.
- Use benchmarks and load tests to investigate the effects of concurrency and
  buffering choices.

This project is expected to change as the concepts being explored become
clearer.
