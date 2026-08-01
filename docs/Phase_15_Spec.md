# Phase 15 Specification — Combined GW-BASIC Core Completeness & Modular Security / Pentesting Infrastructure

## Executive Summary
This specification defines the design, architectural boundaries, and implementation guidelines for **Phase 15 (Combined 100% GW-BASIC Core Completeness & Modular Security/Pentesting Infrastructure)**.

The objective of Phase 15 is twofold:
1. **100% Non-Graphics/Sound/Memory GW-BASIC Parity**: Implement the final remaining classic GW-BASIC statements and functions (`RESET`, `DEF USR`/`USR(n)`, `ERDEV`/`ERDEV$`, `EXTERR`, `ON COM`/`ON PEN`/`ON STRIG`, and `IOCTL`), bringing BASIC++ to total feature completeness with vintage Microsoft BASIC dialects.
2. **Refined Advanced Networking, Security & Penetration Testing**: Implement a portable, OS-agnostic, modular security suite supporting both top-level native BASIC++ keywords and structured module namespace aliases (`SEC::...`).

---

## 1. Part A — GW-BASIC 100% Core Language Parity

### 1.1: File Channel Reset (`RESET`)
- **Syntax**: `RESET`
- **Description**: Closes all currently open file and device channels (file descriptors 1 through 255) and flushes any active file buffers. Equivalent to calling `CLOSE` with no arguments.
- **Implementation**: Iterates through the VM file channel array and executes `vfs_close` on active handles.

### 1.2: User Function Machine-Code Hooks (`DEF USR` / `USR(n)`)
- **Syntax**:
  - `DEF USR[n] = offset_or_address` (where `n` is 0 to 9, defaulting to 0 if omitted)
  - `result = USR[n](argument)`
- **Description**: Configures and invokes user-defined machine code routine pointers or native callback vectors.
- **Dual Execution Model**:
  1. **Mock BIOS Vector Table**: Maps integer offsets to registered virtual callbacks inside `mock_bios.c` or the internal VM dispatch table.
  2. **Native Symbol Binding**: Resolves native C library symbols via platform dynamic linking (`GetProcAddress` on Windows / `dlsym` on POSIX) for OS-isolated native calls while maintaining C17 64-bit pointer safety.

### 1.3: Device Error Inspection Variables (`ERDEV` & `ERDEV$`)
- **Syntax**:
  - `err_code = ERDEV`
  - `device_name$ = ERDEV$`
- **Description**:
  - `ERDEV`: Returns the 16-bit hardware/INT 24h error code or VFS error status from the last device operation.
  - `ERDEV$`: Returns the 8-character ASCII driver/device name string (e.g. `"LPT1    "`, `"COM1    "`, `"DSK1    "`) associated with the last device error.

### 1.4: Extended MS-DOS Error Inspection (`EXTERR`)
- **Syntax**: `val = EXTERR(n)`
- **Description**: Queries extended operating system error information from the last failed file or device operation.
  - `EXTERR(0)`: Extended error code (e.g. 2 = File not found, 5 = Access denied, 32 = Sharing violation).
  - `EXTERR(1)`: Error class (e.g. 1 = Out of resource, 2 = Temporary situation, 11 = Hardware failure).
  - `EXTERR(2)`: Suggested action (e.g. 1 = Retry, 3 = User re-entry, 4 = Abort/Clean up).
  - `EXTERR(3)`: Error locus (e.g. 1 = Unknown, 2 = Block device, 3 = Network, 4 = Serial device).

### 1.5: Peripheral Event Trapping (`ON COM`, `ON PEN`, `ON STRIG`)
- **Syntax**:
  - `ON COM(n) GOSUB line` / `COM(n) ON` / `COM(n) OFF` / `COM(n) STOP`
  - `ON PEN GOSUB line` / `PEN ON` / `PEN OFF` / `PEN STOP`
  - `ON STRIG(n) GOSUB line` / `STRIG(n) ON` / `STRIG(n) OFF` / `STRIG(n) STOP`
- **Description**: Event interrupt handlers for serial communication ports (`COM1` to `COM4`), lightpen/touch screen events, and joystick/gamepad trigger buttons.
- **VHAL / VDEV Integration**: Intercepts events dispatched by the Virtual Hardware Abstraction Layer (VHAL) or active Virtual Devices (`vdev_fujinet.c`, `vdev_console.c`, SDL gamepad handlers).

### 1.6: Enhanced Device Channel Control (`IOCTL` Statement & `IOCTL$` Function)
- **Syntax**:
  - `IOCTL [#]channel, control_string$`
  - `response$ = IOCTL$([#]channel)`
- **Description**: Sends control commands to an open device channel driver or reads status control strings from the virtual device buffer.

---

## 2. Part B — Advanced Networking, Security & Penetration Testing

### 2.1: Hybrid Dialect Syntax Model
All security routines support a dual hybrid syntax:
1. **Top-Level Native Keywords**: Classic BASIC++ keyword syntax (e.g., `NET SCAN`, `HASH$`).
2. **Modular Namespace Aliases**: Object/module syntax (e.g., `SEC::SCAN`, `SEC::HASH$`).

### 2.2: Network Discovery & Reconnaissance
- **Keywords**:
  - `NET SCAN target_subnet$ [FOR ports$] INTO results$()` / `SEC::SCAN`
  - `PORT SCAN host$, start_port, end_port INTO open_ports()` / `SEC::PORTSCAN`
  - `host$ = NET HOST$(ip$)` / `ip$ = NET IP$(host$)`
- **Description**: Performs non-blocking asynchronous TCP/UDP subnet sweeps, port availability audits, and DNS host resolutions.

### 2.3: Raw Packet Capture & Traffic Analysis
- **Keywords**:
  - `PACKET CAPTURE interface$ FOR count INTO packet_buffer$()` / `SEC::CAPTURE`
  - `PACKET INJECT interface$, raw_payload$` / `SEC::INJECT`
- **Description**: Interfaces with raw socket abstractions to sniff network headers (IP, TCP, UDP, ICMP) and inject custom test payloads into virtual networks.

### 2.4: Cryptographic & Password Auditing
- **Keywords**:
  - `digest$ = HASH$(algorithm$, data$)` / `SEC::HASH$` (Supports `"MD5"`, `"SHA1"`, `"SHA256"`, `"SHA512"`, `"CRC32"`)
  - `salted$ = SALT$(data$, salt_string$)` / `SEC::SALT$`
  - `match_index = AUDIT CRACK(hash$, wordlist_file$, algo$)` / `SEC::CRACK`
- **Description**: Portable cryptographic hashing routines and dictionary attack validation loops for password safety testing.

### 2.5: Sandbox Auditing & Evasion Testing
- **Keywords**:
  - `is_sandboxed = SANDBOX AUDIT()` / `SEC::SANDBOX`
  - `is_vm = VM CHECK()` / `SEC::VMCHECK`
- **Description**: Inspects runtime environment metrics (CPU timing skew, hypervisor artifacts, virtual MAC addresses) to detect if execution is taking place inside a VM, container, or analysis sandbox.

---

## 3. Architecture & Portability Constraints
1. **Strict ISO C17 Compliance**: All platform-dependent network or socket APIs must be completely isolated inside platform modules (`src/platform/platform.c` and `src/runtime/vnet.c`). Core parsers and VM logic must remain OS-agnostic.
2. **Thread and Memory Safety**: Packet buffers, hash state objects, and socket tables must be zero-initialized (`calloc`) and safely bounded to avoid heap corruption.
3. **No Direct System Bypasses**: Dynamic symbol lookups in `DEF USR` must validate target handle bounds before execution.

---

## 4. Documentation & Parity Requirements
1. **Interactive HELP System**: Register all new keywords in `src/statements/help_data.h` and ensure interactive `HELP` returns detailed syntax and examples.
2. **Environment CATALOG**: Update `catalog.TXT` and `help.TXT` to include all Phase 15 statements and functions.
3. **User Guides**: Create Markdown documentation files under `docs/` and synced plaintext files under `help/`.

---

## 5. Verification & Testing Plan
1. **Integration Test Suites**:
   - `tests/system/test_phase15_gwbasic.bas`: Tests `RESET`, `DEF USR`/`USR(n)`, `ERDEV`, `EXTERR`, `ON COM`/`ON PEN`/`ON STRIG`, and `IOCTL`.
   - `tests/system/test_phase15_security.bas`: Tests `NET SCAN`, `PORT SCAN`, `HASH$`, `AUDIT CRACK`, `SANDBOX AUDIT`, and `VM CHECK`.
2. **Build Targets**: Build and verify `baspp` executable target under Windows (MSVC) and Linux (GCC).
3. **SELFTEST Execution**: Execute `baspp.exe -c "SELFTEST"` to verify system diagnostics pass cleanly.
