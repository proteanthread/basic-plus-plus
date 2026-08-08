# BASIC++ Cloud Server & Scripting Edition (`baspp` / `bs`)

## Overview
The **Cloud Server & Scripting Edition** is optimized for headless Linux containers (Docker, Podman, Kubernetes), CGI web pipelines, automated job runners, and continuous integration services.

## Binary Targets
- **`baspp`**: Headless Cloud Workstation (`--export-docs`, `--export-api-json`, batch runner)
- **`bs`**: Non-Interactive Batch Script Runner (Zero REPL, zero prompt, instant script termination)

## Features & Constraints
- **Memory Footprint**: 64 MB (`67108864L` bytes) for `bs`, 256 MB for `baspp`
- **Zero Display Server Dependency**: Operates 100% headless with zero X11/Wayland/SDL2 dependencies (`NO_SDL2`)
- **Pipeline Ready**: Reads from `stdin`, writes clean 7-bit ASCII to `stdout`, logs errors to `stderr`
- **Network Stack**: Full support for Gemini protocol (`vnet`), TCP/UDP socket channels, HTTP, and regex engines

## Server Pipeline Usage Example
```bash
# Execute BASIC++ script directly in CGI or bash pipeline
bs -c "FOR I = 1 TO 5: PRINT \"Server Node Response \"; I: NEXT I"
```
