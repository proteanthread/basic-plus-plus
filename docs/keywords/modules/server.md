# `server` Batch Runner & Server Profile

## 1. Architectural Purpose & Overview

The `server` profile targets the headless batch script runner (`bs` / `bs.exe`), automated CI/CD pipelines, PowerShell/Bash scripting, CGI web backends, and background network daemons.

### Key Architectural Specifications:
- **Default Memory Allocation**: 64 MB RAM (`67108864L` bytes).
- **Non-Interactive Batch Mode**: Zero banner, zero prompt, zero REPL loop iterations.
- **Fast Startup**: Minimal startup latency (< 2 ms) with stream-oriented stdin/stdout/stderr pipes.
- **Network Stack**: Links `libserver` for TCP/UDP sockets (`vnet`), Gemini protocol, and background tasks.

---

## 2. Command-Line Usage

```bash
# Non-interactive script execution
bs script.bas

# Pipeline integration
echo 'PRINT "Result: "; 42 * 2' | bs
```
