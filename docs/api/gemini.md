# C17 API Reference: Gemini Protocol Client (`runtime/gemini.h`)

## 1. Subsystem Overview & Responsibilities

The Gemini Protocol Subsystem (`runtime/gemini.h`, implemented in `engine/src/runtime/gemini.c`) provides an embedded TLS client for querying, streaming, and fetching capsules and text over the lightweight Gemini internet protocol (`gemini://`) in BASIC++ v6.5.2.

Key architectural responsibilities include:
- **`gemini://` URL Resolution**: Connects to remote Gemini hosts over port 1965 with TLS encryption.
- **Protocol Header Parsing**: Inspects Gemini status codes (`1x` Input, `20` Success, `3x` Redirect, `4x` Temporary Failure, `5x` Permanent Failure, `6x` Client Certificate Required).
- **Virtual Stream Integration**: Exposes Gemini connections as virtual stream channels accessible via `OPEN "GEMINI://..."`, `LINE INPUT#`, and `INPUT$`.
- **Sandboxed Network Privilege**: Verifies `CAP_NET` capability in `SecurityContext` prior to opening remote TLS sockets.

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/gemini.h"
#include "vm/vm.h"
#include "types/errors.h"
```

## 3. Function Prototypes & Operational Contracts

```c
/**
 * @brief Fetches the response payload of a gemini:// URL.
 * @param vm Pointer to active VMContext.
 * @param url The gemini:// URL string (e.g. "gemini://geminiprotocol.net").
 * @param out_err Pointer to BppError structure populated on connection or protocol error.
 * @return Dynamically allocated null-terminated response payload (caller must free), or NULL on failure.
 */
char *net_gemini_fetch(VMContext *vm, const char *url, BppError *out_err);
```

## 4. Architectural Invariants

- **Capability Enforcement**: Invoking Gemini network calls without `CAP_NET` returns Error 70 (`ERR_PERMISSION_DENIED`).
- **Memory Safety**: Response payloads returned by `net_gemini_fetch()` are heap-allocated and must be freed by the caller via `free()` or converted to a managed string via `str_create()`.

## 5. Code Example: Fetching Gemini Capsule in BASIC++

```basic
10 OPEN "GEMINI://geminiprotocol.net" FOR INPUT AS #1
20 WHILE NOT EOF(1)
30   LINE INPUT #1, L$
40   PRINT L$
50 WEND
60 CLOSE #1
```
