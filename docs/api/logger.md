# C17 API Reference: Diagnostic Logger (`debug/logger.h`)

## 1. Subsystem Overview & Responsibilities

The Diagnostic Logger Subsystem (`debug/logger.h`, implemented in `engine/src/debug/logger.c`) provides an opt-in logging framework for tracing VM execution, recording warning/error diagnostics, capturing test traces, and replicating console output in BASIC++ v6.5.2.

Key architectural responsibilities include:
- **Opt-In File Logging Invariant**: All executables (`baspp`, `bpp`, `bs`, `bppc`) MUST initialize logging with `logger_init(NULL, NULL)` by default. No `.LOG` or `.OUT` files may be generated on disk unless the user explicitly passes `--log`, `--log=<path>`, `--debug`, or `--trace` on the command line.
- **Multi-Channel Diagnostic Logging**: Formats informational (`log_info`), warning (`log_warn`), and fatal error (`log_error`) messages with millisecond timestamps and subsystem tags.
- **Console Replication Stream**: Records raw interactive console input and output text to `.OUT` session files via `log_write_out()` for automated regression verification.
- **Dynamic Diagnostic Flags**: Manages global debug mode (`DEBUG ON`/`OFF`), execution trace (`TRON`/`TROFF`), and dry-run syntax verification flags.

## 2. Header Inclusion & Prerequisites

```c
#include "debug/logger.h"
#include <stdbool.h>
```

## 3. Function Prototypes & Operational Contracts

### Lifecycle & Initialization
```c
/**
 * @brief Initializes the logging subsystem with optional disk paths.
 * @param log_path Path to .LOG file (or NULL to disable disk logging).
 * @param out_path Path to .OUT console capture file (or NULL to disable).
 * @return true if initialized successfully, false if files could not be opened.
 */
bool logger_init(const char *log_path, const char *out_path);

/**
 * @brief Closes all active log file descriptors and flushes pending data.
 */
void logger_close(void);
```

### Diagnostic Logging APIs
```c
/**
 * @brief Logs an informational message to the active log stream.
 */
void log_info(const char *fmt, ...);

/**
 * @brief Logs a warning or non-fatal engine condition.
 */
void log_warn(const char *fmt, ...);

/**
 * @brief Logs a fatal or trapped runtime error.
 */
void log_error(const char *fmt, ...);

/**
 * @brief Replicates raw console/terminal text to the .OUT file stream.
 */
void log_write_out(const char *buf, size_t len);
```

### Global Diagnostic Mode Flags
```c
void logger_set_debug(bool debug);
bool logger_is_debug(void);

void logger_set_dry_run(bool dry_run);
bool logger_is_dry_run(void);

void logger_set_trace(bool trace);
bool logger_is_trace(void);
```

## 4. Architectural Invariants

- **Zero Unsolicited File Creation**: Never create disk logs when `log_path` is `NULL`.
- **Thread Safety**: Formats log messages safely without corrupting interpreter state.

## 5. Code Example: Initializing Logging on Command-Line Flag

```c
#include "debug/logger.h"

void handle_cli_options(int argc, char **argv) {
    const char *log_file = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            logger_set_debug(true);
            log_file = "basicpp_debug.log";
        }
    }

    /* Pass NULL by default to prevent disk file generation */
    logger_init(log_file, NULL);
    log_info("BASIC++ v6.5.2 engine started");
}
```
