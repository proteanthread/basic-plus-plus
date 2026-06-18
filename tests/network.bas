REM ============================================================
REM  TEST: network.bas - Network I/O (Test 17)
REM ============================================================
REM  Tests module activation, network builtins, virtual
REM  network/device commands, and IOCTL dispatch.
REM
REM  IMPORTANT: Does NOT require a live network connection.
REM  All network functions return default/error values when
REM  no actual hardware or network stack is present.
REM
REM  SEQUENCE:
REM   1. MODULE: list registered modules (before activation)
REM   2. MODULE INFO: query FUJINET and UPNP module details
REM   3. NINFO$: query network info (defaults without module)
REM      - ip, ssid, wifi, version, mac
REM   4. Network builtins on invalid channel 99:
REM      - NSTATUS, NEOF, NCONNECTED, NERROR,
REM        NBYTESWAITING, NHTTPSTATUS
REM      All should return 0 or error for invalid channel.
REM   5. MODULE "FUJINET": activate FujiNet module
REM   6. MODULE: list again (should show FUJINET active)
REM   7. VNET: display virtual network status
REM   8. MODULE "UPNP": activate UPnP module
REM   9. VDEV: list virtual devices (should include
REM      N:, FUJI:, CLOCK:, UPNP:, SOAP:)
REM  10. NINFO$ with FUJINET active (may differ)
REM  11. MODULE UNLOAD: unload both modules
REM  12. MODULE: verify clean unload
REM
REM  EXPECTED: All commands execute without errors.
REM  Network builtins return 0 for invalid channels.
REM  Module lifecycle (load/unload) works cleanly.
REM  Ends with "=== Test 17 PASSED ==="
REM ============================================================
REM

REM --- List registered modules (before activation) ---
REM  Shows all available modules and their status.
MODULE

REM --- Query module metadata ---
REM  MODULE INFO displays version, description, and
REM  capabilities for each named module.
MODULE INFO "FUJINET"
MODULE INFO "UPNP"

REM --- NINFO$: network information strings ---
REM  Without FUJINET active, these return defaults
REM  (empty strings or "N/A").
PRINT "IP: "; NINFO$("ip")
PRINT "SSID: "; NINFO$("ssid")
PRINT "WiFi: "; NINFO$("wifi")
PRINT "Version: "; NINFO$("version")
PRINT "MAC: "; NINFO$("mac")

REM --- Network builtins on invalid channel ---
REM  Channel 99 is not open; all queries should return
REM  0 or an error indicator without crashing.
PRINT "NSTATUS(99)="; NSTATUS(99)
PRINT "NEOF(99)="; NEOF(99)
PRINT "NCONNECTED(99)="; NCONNECTED(99)
PRINT "NERROR(99)="; NERROR(99)
PRINT "NBYTESWAITING(99)="; NBYTESWAITING(99)
PRINT "NHTTPSTATUS(99)="; NHTTPSTATUS(99)

REM --- Activate FUJINET module ---
REM  MODULE "name" loads and activates the module.
REM  This registers virtual devices (N:, FUJI:, CLOCK:).
MODULE "FUJINET"
REM  Verify it appears in the active module list.
MODULE

REM --- Virtual network status ---
REM  VNET displays the virtual network adapter state.
VNET

REM --- Activate UPNP module ---
REM  Adds UPNP: and SOAP: virtual devices.
MODULE "UPNP"

REM --- Virtual device listing ---
REM  VDEV should now show all registered virtual devices
REM  from both FUJINET and UPNP modules.
VDEV

REM --- NINFO$ with FUJINET active ---
REM  With the module active, IP may return a virtual
REM  address instead of the default.
PRINT "IP (FN): "; NINFO$("ip")

REM --- Unload modules ---
REM  MODULE UNLOAD frees module resources and
REM  deregisters its virtual devices.
MODULE UNLOAD "UPNP"
MODULE UNLOAD "FUJINET"
REM  Verify clean unload - modules should no longer appear.
MODULE

PRINT "=== Test 17 PASSED ==="
