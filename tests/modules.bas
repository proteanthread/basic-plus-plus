REM ============================================================
REM  TEST: modules.bas - Module System Lifecycle
REM ============================================================
REM  Tests the MODULE command for listing, querying, loading,
REM  and unloading interpreter extension modules.
REM
REM  SEQUENCE:
REM   1. MODULE: list all registered modules and status
REM   2. MODULE INFO "STDLIB": query built-in stdlib module
REM      Shows version, description, and registered functions.
REM   3. MODULE INFO "USB": query USB module metadata
REM   4. MODULE "USB": activate the USB module
REM      Registers USB-specific virtual devices and commands.
REM   5. MODULE: list again (USB should appear as active)
REM   6. MODULE UNLOAD "USB": deactivate and free USB module
REM      Deregisters its virtual devices and frees resources.
REM   7. MODULE: list again (USB should no longer be active)
REM
REM  EXPECTED: All commands execute without errors.
REM  Module state transitions: inactive -> active -> unloaded.
REM  No memory leaks from load/unload cycle.
REM ============================================================
REM

REM --- List all registered modules ---
MODULE

REM --- Query module info ---
REM  STDLIB is always registered (built-in core library).
MODULE INFO "STDLIB"

REM  USB module provides USB device access (if available).
MODULE INFO "USB"

REM --- Activate USB module ---
MODULE "USB"

REM --- Verify activation ---
MODULE

REM --- Unload USB module ---
MODULE UNLOAD "USB"

REM --- Verify clean unload ---
MODULE
