## Project Purpose

This project is primarily for learning low-level systems concepts.

## Development

- Target platform: Linux. The server uses `epoll`, POSIX sockets, and `pthread`; do not introduce platform abstractions unless explicitly needed.
- Build with GNU Make and a GNU C17 compiler: `make`, `make debug`, `make all`, and `make test`. `make test` runs assertion-based suites for the router, parser, response helpers, server setup, thread pool, and logging.
- Compiler warnings are errors. Keep code valid under the flags in `compile_flags.txt` and format C code with the repository's LLVM-based `.clang-format` style (4-space indentation, 120 columns).
- Generated build output lives in `bin/`, `debug/`, and `obj/`; use `make clean` to remove it.

## Architecture

This is a backend server only. This will not serve html files only JSON.

- `src/main.c` starts the server on the fixed port `8080`, initializes the router, and enters the listener loop.
- `src/server/server.c` owns TCP socket setup, bind, and listen. `src/server/listener.c` owns the `epoll` event loop, accepted-connection state, and incremental request parsing.
- `src/http/parser.c` parses request lines and headers from the per-connection buffer. Its fixed limits are defined in `include/http/http.h`: 8 KiB input buffer, 100 headers, and bounded request/header fields.
- `src/routes/router.c` provides dynamic exact-path registration and handler invocation. `setup_router` registers `/healthcheck`.
- `src/http/response.c` builds and sends responses, and `src/threadpool/threadpool.c` implements a bounded worker queue. Neither is wired into the listener yet.
- The executable currently logs complete request headers and closes the connection. It does not invoke route handlers or send HTTP responses; do not write tests or documentation that assume request bodies, keep-alive, graceful shutdown, or complete malformed-request handling exist.
- Keep ownership explicit for resources with lifetimes: listener/client file descriptors, heap-allocated connections and response buffers, router storage, and thread-pool tasks.

## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

When the user types `/graphify`, use the installed graphify skill or instructions before doing anything else.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- Dirty graphify-out/ files are expected after hooks or incremental updates; dirty graph files are not a reason to skip graphify. Only skip graphify if the task is about stale or incorrect graph output, or the user explicitly says not to use it.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).
