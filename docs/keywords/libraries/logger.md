# `logger` Opt-In Logging Subsystem (`libplatform`)

## 1. Architectural Purpose & Overview

The `logger` subsystem (`engine/src/platform/logger.c`) provides diagnostic and trace logging for debugging the VM engine.

### Key Architectural Invariants:
- **Opt-In Invariant**: All executables (`baspp`, `bpp`, `bs`) MUST initialize logging with `logger_init(NULL, NULL)` by default. No `.LOG` or `.OUT` files are created on disk unless explicitly requested via command line flags (`--log`, `--debug`, `--trace`).
- **Log Levels**: Supports `LOG_DEBUG`, `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`.

---

## 2. Technical API Signatures (C17)

```c
void logger_init(const char *log_file_path, const char *level_str);
void logger_shutdown(void);
void logger_log(LogLevel level, const char *fmt, ...);
```
