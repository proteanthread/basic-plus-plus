# `gemini` Gemini Protocol Server Subsystem (`libserver`)

## 1. Architectural Purpose & Overview

The `gemini` subsystem (`engine/src/server/gemini.c`) implements lightweight, privacy-preserving Gemini protocol network client and server engines for BASIC++.

### Key Architectural Invariants:
- **TLS Security**: Handles TLS 1.2 / 1.3 handshake encryption over port 1965.
- **Gemtext Parser**: Automatically parses and renders `text/gemini` hyperdocuments in terminal and TUI text modes.

---

## 2. Technical API Signatures (C17)

```c
GeminiServer *gemini_create_server(int port, const char *cert_path, const char *key_path);
int gemini_server_poll(GeminiServer *server);
void gemini_destroy_server(GeminiServer *server);
```
