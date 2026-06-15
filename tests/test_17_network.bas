REM === Test 17: Network I/O ===
REM Tests module activation, network builtins,
REM N: device routing, and IOCTL dispatch.
REM Does NOT require live network connection.

REM --- Module listing (before activation) ---
MODULE

REM --- Module info ---
MODULE INFO "FUJINET"
MODULE INFO "UPNP"

REM --- NINFO$ defaults (no FUJINET active) ---
PRINT "IP: "; NINFO$("ip")
PRINT "SSID: "; NINFO$("ssid")
PRINT "WiFi: "; NINFO$("wifi")
PRINT "Version: "; NINFO$("version")
PRINT "MAC: "; NINFO$("mac")

REM --- Network builtins on invalid channel ---
PRINT "NSTATUS(99)="; NSTATUS(99)
PRINT "NEOF(99)="; NEOF(99)
PRINT "NCONNECTED(99)="; NCONNECTED(99)
PRINT "NERROR(99)="; NERROR(99)
PRINT "NBYTESWAITING(99)="; NBYTESWAITING(99)
PRINT "NHTTPSTATUS(99)="; NHTTPSTATUS(99)

REM --- Activate FUJINET module ---
MODULE "FUJINET"
MODULE

REM --- VNET status display ---
VNET

REM --- Activate UPNP module ---
MODULE "UPNP"

REM --- VDEV should now show N:, FUJI:, CLOCK:, UPNP:, SOAP: ---
VDEV

REM --- NINFO$ with FUJINET active ---
PRINT "IP (FN): "; NINFO$("ip")

REM --- Unload modules ---
MODULE UNLOAD "UPNP"
MODULE UNLOAD "FUJINET"
MODULE

PRINT "=== Test 17 PASSED ==="
