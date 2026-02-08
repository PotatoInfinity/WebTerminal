Web Terminal

## Requirements
- A C++ compiler (g++ or clang++)
- Make (usually installed on macOS)

## Compilation
To compile the server, simply run:

```bash
make
```

This will download `mongoose.c` and `mongoose.h` automatically and compile the `server` binary.

## Running
To start the server:

```bash
./server
```

When started, it will automatically open your default browser to `http://localhost:8000`.

## Features
- Fast C++ implementation
- WebSocket support for real-time terminal interaction
- Automatic browser launch
- Single executable deployment (after compilation)
