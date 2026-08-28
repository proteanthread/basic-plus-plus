# C17 API Reference: Versioning & Target Identity (`types/version.h`)

## 1. Subsystem Overview & Responsibilities

The Versioning & Target Identity Header (`types/version.h`) defines semantic version constants, release build dates, codename strings, and clean macro identifiers for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Semantic Versioning (v6.5.2)**: Declares integer major (`6`), minor (`5`), and patch (`2`) numbers alongside canonical release string `"6.5.2"`.
- **Clean Identifier Naming Invariant**: In accordance with project architecture rules, provides clean, non-prefixed macro aliases (`VERSION_MAJOR`, `VERSION_MINOR`, `VERSION_PATCH`, `VERSION_STRING`, `VERSION_DATE`, `VERSION_CODENAME`), prohibiting the introduction of new `BPP_` / `bpp_` prefixes.
- **Build Identity Reflection**: Fuels the `_VERSION$` system variable, `VERSION` statement, startup banners, and compiler artifact headers.

## 2. Header Inclusion & Prerequisites

```c
#include "types/version.h"
```

## 3. Version Constants & Definitions

```c
#define BASIC_VERSION_MAJOR    6
#define BASIC_VERSION_MINOR    5
#define BASIC_VERSION_PATCH    2
#define BASIC_VERSION_STRING   "6.5.2"
#define BASIC_VERSION_DATE     "2026-08-06"
#define BASIC_VERSION_CODENAME "Phoenix"

/* Clean non-prefixed aliases per project rules */
#define VERSION_MAJOR          BASIC_VERSION_MAJOR
#define VERSION_MINOR          BASIC_VERSION_MINOR
#define VERSION_PATCH          BASIC_VERSION_PATCH
#define VERSION_STRING         BASIC_VERSION_STRING
#define VERSION_DATE           BASIC_VERSION_DATE
#define VERSION_CODENAME       BASIC_VERSION_CODENAME
```

## 4. Architectural Invariants

- **No `BPP` or `BPP_` Prefix Rule**: New code must NOT use `BPP_` as a prefix for version macros, identifiers, or types.
- **Consistent Banner Output**: Startup banners across all executables (`baspp`, `bpp`) must format the version string strictly matching `VERSION_STRING` (`v6.5.2`).

## 5. Code Example: Verifying Version Compatibility in C

```c
#include "types/version.h"
#include <stdio.h>

void check_engine_version(void) {
    printf("BASIC++ Engine %s (%s, Release: %s)\n",
           VERSION_STRING, VERSION_CODENAME, VERSION_DATE);
#if VERSION_MAJOR < 6
#error "BASIC++ requires v6.x engine infrastructure"
#endif
}
```
