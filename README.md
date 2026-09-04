# HTTP Server in C

A work-in-progress systems programming project for learning how a network
server is built from the bottom up in C.

The project combines networking, concurrency, memory management, and the
boundary between operating-system primitives and application-level protocols.
It is intentionally small and close to the POSIX APIs rather than being a
production-ready HTTP implementation.

## Status

The project currently has the individual building blocks of a server, but they
are not yet wired into a complete request/response flow:

- A TCP server creates, configures, binds, and listens on an IPv4 socket.
- An epoll-based listener accepts connections and configures them as
  non-blocking sockets.
- An incremental, line-oriented HTTP request parser reads from a socket into a
  bounded buffer and parses the request line and headers.
- HTTP responses can be built with a status, dynamically allocated headers,
  and a body. There is a helper for JSON responses.
- A fixed-size thread pool uses a bounded ring queue, a mutex, and a condition
  variable to coordinate worker threads.
- A small logging module reports messages and `errno` values.

The listener currently closes client connections when they become readable;
requests are not yet passed to the parser, dispatched to handlers, or answered
with HTTP responses. The thread pool is implemented but is not currently used
by the executable. There is also no automated test suite yet.

## Current Runtime Behavior

Running the program currently:

1. Binds an IPv4 TCP listener to `0.0.0.0:8080`.
2. Creates an epoll instance and waits for the listener to become readable.
3. Accepts incoming client connections and registers them with epoll as
   non-blocking sockets.
4. Closes a client connection when an event is received for it.

The program runs until it encounters an error or is stopped. It does not yet
parse requests, start the thread pool, or return HTTP responses.

## Building

The project requires a C compiler, GNU Make, POSIX threads, and a Linux-like
environment.

```sh
make          # build bin/server.out
make debug    # build debug/server.out with debug symbols and no optimization
make all      # build both normal and debug executables
make run      # build and run the server
```

Build settings include GNU C17, strict compiler warnings treated as errors,
dependency generation, and `pthread` support. Generated object files,
dependency files, and executables are ignored by Git.

Available cleanup targets are `make clean`, `make clean-debug`, and
`make clean-bin`.

## Project Layout

| Path | Purpose |
| --- | --- |
| `src/server/server.c` | TCP socket setup, binding, and listening |
| `src/server/listener.c` | Epoll event loop, client acceptance, and non-blocking sockets |
| `src/http/` | Request parsing and response construction |
| `src/threadpool/` | Worker threads and the bounded task queue |
| `src/log/` | Basic stderr logging helpers |
| `src/main.c` | Executable entry point |
| `include/http/http.h` | HTTP request, response, and parser interfaces |
| `include/server/` | TCP server and listener interfaces |
| `include/threadpool/` | Thread-pool interface |
| `include/log/` | Logging interface |
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

- Read and parse client requests from the listener event loop.
- Enqueue client handling in the thread pool.
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
