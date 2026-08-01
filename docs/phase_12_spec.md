# Phase 12 Specification — Verification & Stress-Testing

This document details the verification and stress-testing strategy for Phase 12, ensuring that all features implemented in Phase 11 (C Embedding API, Virtual Devices, Sound, Mouse, Clipboard, Networking, Compression, and Session Saving) are thoroughly validated, leaving no stone unturned.

---

## 1. Automated Regression Test Suites

We will introduce new automated tests under the `tests/` directory to run natively via the `SELFTEST` suite or as standalone test runner scripts.

### A. Test Suite 1: Hardware & I/O (`tests/basic/56_io_devices_test.bas`)
- **Sound & Audio**: Verify MML parsing, queue lengths, and beep limits without blocking execution.
- **Mouse & Input**: Test mouse position queries, mouse button checks, and queue length bounds.
- **Clipboard Access**:
  - Assign a standard ASCII string to `_CLIPBOARD$` and read it back to confirm a match.
  - Test binary safety: write strings with special character escape sequences.
- **Window Controls**: Verify that calling `_TITLE`, `_SCREENMOVE`, and `_RESIZABLE` runs without throwing errors or crashes.

### B. Test Suite 2: Networking & Utilities (`tests/basic/57_networking_test.bas`)
- **LZ77 Compression**:
  - Compress a simple string and verify that decompressing it returns the exact original content.
  - Compress a large block of repeating characters and verify high compression ratio.
  - Compress binary data containing null characters (`\0`) and verify round-trip integrity.
- **Session State Saving**:
  - Save the VM state (`_STATESAVE "session.dat"`).
  - Verify that the snapshot file exists, has a valid header, and can be loaded cleanly using `_STATELOAD "session.dat"`.
- **TCP/IP Sockets**:
  - Attempt to open connection to local loopback `127.0.0.1`.
  - Verify error handling on unreachable hosts or invalid socket syntax.

---

## 2. Documentation & API Verification

To verify that the documentation is completely accurate and aligned with the codebase, Phase 12 will include an automated documentation audit system.

### A. Documentation Code Snippet Compilation Tests
- **Verification Harness (`tests/docs/test_snippets.py`)**: An automated Python test harness will:
  - Parse every Markdown file in `docs/api/` and `docs/api/tutorials/` to extract all fenced `c` code blocks.
  - Generate a temporary `.c` source file for each block.
  - Compile the temporary files against the `include/` directory using MSVC (`cl.exe`) on Windows and GCC (`gcc`) on Linux.
  - Assert that all code blocks compile with **zero compilation warnings and zero errors**.

### B. API Alignment Audit
- **Public API Completeness Check**: A validation script will verify that every public function, struct, and enum cataloged in [PUBLIC_API_AUDIT.md](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/api/PUBLIC_API_AUDIT.md) is mentioned and documented in its respective subsystem reference sheet (e.g. all functions from `bpp_boot.h` must have matching entries in `boot.md`).

### C. Cross-Platform Executable Regression Testing
- **Target Boundaries**: Test the standard interactive/graphical editions:
  - `baspp.exe` on Windows (MSVC build).
  - `baspp` on Linux (GCC/Clang build).
  - **Excluded Targets**: The external compiler (`bppc`), transpiler (`trans`), and detokenizer (`detok`) are explicitly excluded from this verification phase.

---

## 3. C Unit Integration Tests

We will expand the C unit test suite in `tests/c_unit/` to test embedding functions:
- **Embedding Lifecycle**: Create, run, and destroy 100 consecutive `BootContext` environments to verify zero heap leaks during initialization/shutdown.
- **Custom VDev Registry**: Register, write to, and read from a mock virtual device, checking correct byte counts and buffering.
- **Module Sandboxing**: Attempt to load invalid or unsigned dynamic plugins and assert that the security subsystem blocks them.

---

## 3. Stress-Testing & Security Auditing

To ensure production-level robustness:
- **Valgrind/ASAN Memory Checks**: Run standard tests with AddressSanitizer enabled to detect out-of-bounds reads on corrupted LZ77 inputs, buffer overflow vulnerabilities, or uninitialized variables.
- **Buffer Overflow Resistance**: Assign a 10MB string to `_CLIPBOARD$` and verify that memory growth bounds prevent host crashes.
- **Fuzzing LZ77 Decompressor**: Feed random binary junk into `_INFLATE$` to ensure it rejects corrupted inputs gracefully rather than segfaulting.
