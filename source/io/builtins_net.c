/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: builtins_net.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Input/Output operations, screen print layout, file streams (sequential/binary/random), block I/O formatting, and record buffers.
 *
 * 2. WHAT TO EXPECT:
 *    Delegates file operations through the Virtual Device (VDev) mapping layer. Relies on standard ANSI C streams.
 *
 * 3. WHAT CAN BE CHANGED:
 *    File channel limit rules, I/O default buffers, record layout details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Binary packing format definitions (MKI$, MKS$, MKD$), file channel index lookups.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check file channel descriptor leaks (ensure CLOSE #ch is called). Verify binary file alignments and record record size boundaries.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - builtins_net.c
 // ---
 //
 // Network introspection builtin functions.
 //
 // These functions provide BASIC-level access to network
 // channel state. They work with both core vdev_net channels
 // and FujiNet N: device channels.
 //
 // Functions:
 //   NSTATUS(ch)          - Channel status (0=ok, -1=error)
 //   NHTTPSTATUS(ch)      - HTTP response code (200, 404, etc.)
 //   NEOF(ch)             - EOF flag (1=EOF, 0=data available)
 //   NBYTESWAITING(ch)    - Bytes available to read
 //   NCONNECTED(ch)       - Connection state (1=connected)
 //   NERROR(ch)           - Last error code
 //   NJSONQUERY$(ch, p$)  - JSON path query (FujiNet only)
 //   NINFO$(key$)         - Adapter info (ip, ssid, wifi, etc.)
 //
//
// HOW TO EXTEND:
//   To add a new built-in function:
//   1. Write a handler: BValue my_func(BValue *args, int argc, void *ctx)
//   2. Register it in the init function with funcreg_add().
//   3. Specify min/max argument counts and return type.
//
// TROUBLESHOOTING:
//   - Wrong arg count: check min_args/max_args in registration.
//   - Type mismatch: use bval_to_float/bval_to_int for conversion.
 // ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "builtins.h"
#include "fileio.h"
#include "vdev.h"
#include "module.h"
#include "security.h"
#include "stringpool.h"
#include "runtime.h"

 // Helper: Get the VDev for a channel, or NULL.
static VDev *net_get_channel_vdev(int chan)
{
 VDev *dev = fileio_get_channel_vdev(chan);
 if (dev == NULL) return NULL;
 // Verify it's a network device
 if (dev->dev_class != VDCLASS_NETWORK) return NULL;
 return dev;
}

 // NSTATUS(ch) - Network channel status.
 // Returns 0 if connected/ok, -1 if error or not a network channel.
BValue builtin_nstatus(BValue *args, int argc, void *rt)
{
 int chan;
 VDev *dev;
 (void)rt;

 if (argc < 1) return bval_int(-1);
 chan = bval_to_int(&args[0]);

 dev = net_get_channel_vdev(chan);
 if (dev == NULL) return bval_int(-1);

 if (dev->dev_status != NULL)
  return bval_int(dev->dev_status(dev));

 return bval_int(0);
}

 // NHTTPSTATUS(ch) - HTTP response status code.
 // Only meaningful when channel is an HTTP connection
 // through FujiNet. Returns 0 for non-HTTP channels.
BValue builtin_nhttpstatus(BValue *args, int argc, void *rt)
{
 int chan;
 VDev *dev;
 const char *val;
 (void)rt;

 if (argc < 1) return bval_int(0);
 chan = bval_to_int(&args[0]);

 dev = net_get_channel_vdev(chan);
 if (dev == NULL) return bval_int(0);

 if (dev->dev_info != NULL) {
  val = dev->dev_info(dev, "http_status");
  if (val != NULL) return bval_int(atoi(val));
 }

 return bval_int(0);
}

 // NEOF(ch) - Network EOF flag.
 // Returns 1 if EOF/disconnected, 0 if data may be available.
BValue builtin_neof(BValue *args, int argc, void *rt)
{
 int chan;
 VDev *dev;
 (void)rt;

 if (argc < 1) return bval_int(1);
 chan = bval_to_int(&args[0]);

 dev = net_get_channel_vdev(chan);
 if (dev == NULL) return bval_int(1);

 // Check poll if available
 if (dev->dev_poll != NULL)
  return bval_int(dev->dev_poll(dev) ? 0 : 1);

 // If connected, assume not EOF
 if (dev->dev_status != NULL)
  return bval_int(dev->dev_status(dev) == 0 ? 0 : 1);

 return bval_int(0);
}

 // NBYTESWAITING(ch) - Bytes available to read.
 // Returns the number of bytes in the receive buffer.
BValue builtin_nbyteswaiting(BValue *args, int argc,
 void *rt)
{
 int chan;
 VDev *dev;
 const char *val;
 (void)rt;

 if (argc < 1) return bval_int(0);
 chan = bval_to_int(&args[0]);

 dev = net_get_channel_vdev(chan);
 if (dev == NULL) return bval_int(0);

 if (dev->dev_info != NULL) {
  val = dev->dev_info(dev, "bytes_waiting");
  if (val != NULL) return bval_int(atoi(val));
 }

 return bval_int(0);
}

 // NCONNECTED(ch) - Connection state.
 // Returns 1 if connected, 0 if not.
BValue builtin_nconnected(BValue *args, int argc, void *rt)
{
 int chan;
 VDev *dev;
 const char *val;
 (void)rt;

 if (argc < 1) return bval_int(0);
 chan = bval_to_int(&args[0]);

 dev = net_get_channel_vdev(chan);
 if (dev == NULL) return bval_int(0);

 if (dev->dev_info != NULL) {
  val = dev->dev_info(dev, "connected");
  if (val != NULL && val[0] == '1')
   return bval_int(1);
 }

 // If status is 0, treat as connected
 if (dev->dev_status != NULL)
  return bval_int(dev->dev_status(dev) == 0 ? 1 : 0);

 return bval_int(0);
}

 // NERROR(ch) - Last network error code.
 // Returns 0 for no error, or a FujiNet FN_ERR_* value.
BValue builtin_nerror(BValue *args, int argc, void *rt)
{
 int chan;
 VDev *dev;
 const char *val;
 (void)rt;

 if (argc < 1) return bval_int(0);
 chan = bval_to_int(&args[0]);

 dev = net_get_channel_vdev(chan);
 if (dev == NULL) return bval_int(-1);

 if (dev->dev_info != NULL) {
  val = dev->dev_info(dev, "error");
  if (val != NULL) return bval_int(atoi(val));
 }

 return bval_int(0);
}

 // NJSONQUERY$(ch, path$) - JSON path query.
 // Requires FujiNet module for JSON parsing.
 // Returns the value at the specified JSON path,
 // or empty string if not available.
BValue builtin_njsonquery(BValue *args, int argc, void *rt)
{
 RuntimeState *rs = (RuntimeState *)rt;
 int chan;
 VDev *dev;
 const char *path;
 const char *result;

 if (argc < 2) return bval_string("", 0);

 chan = bval_to_int(&args[0]);

 if (!bval_is_string(&args[1]))
  return bval_string("", 0);
 path = args[1].v.sval.data;

 dev = net_get_channel_vdev(chan);
 if (dev == NULL) return bval_string("", 0);

 // JSON query requires FujiNet module
 if (!module_is_active("FUJINET")) {
  return bval_string("", 0);
 }

 if (dev->dev_info != NULL) {
  // Pass query path via dev_info.
   // FujiNet VDev handles "json_query:path" keys. 
  {
  char key[280];
  snprintf(key, sizeof(key), "json_query:%s", path);
  result = dev->dev_info(dev, key);
  if (result != NULL) {
   int rlen = (int)strlen(result);
   char *p = strpool_store(&rs->strpool,
    result, rlen);
   return bval_string(p, rlen);
  }
  }
 }

 return bval_string("", 0);
}

 // NINFO$(key$) - Network adapter info query.
 // Returns adapter information by key:
 //   "ip"      - local IP address
 //   "ssid"    - WiFi SSID (or "N/A" on wired)
 //   "wifi"    - WiFi status
 //   "version" - firmware/stack version
 //   "mac"     - MAC address
 //   "gateway" - default gateway
 //   "dns"     - DNS server
 //   "netmask" - subnet mask
 //
 // If FUJINET module is active, queries the FUJI: device.
 // Otherwise returns platform defaults.
BValue builtin_ninfo(BValue *args, int argc, void *rt)
{
 RuntimeState *rs = (RuntimeState *)rt;
 const char *key;
 const char *result = NULL;

 if (argc < 1 || !bval_is_string(&args[0]))
  return bval_string("", 0);

 key = args[0].v.sval.data;

 // Try FUJI: device first if active
 if (module_is_active("FUJINET")) {
  int dev_id = vdev_find_by_name("FUJI:");
  if (dev_id >= 0) {
   VDev *fd = vdev_get(dev_id);
   if (fd != NULL && fd->dev_info != NULL) {
    result = fd->dev_info(fd, key);
   }
  }
 }

 // Platform defaults for common queries
 if (result == NULL) {
  if (strcmp(key, "ip") == 0)
   result = "127.0.0.1";
  else if (strcmp(key, "ssid") == 0)
   result = "N/A";
  else if (strcmp(key, "wifi") == 0)
   result = "CONNECTED";
  else if (strcmp(key, "version") == 0)
   result = "BASIC++ Core";
  else if (strcmp(key, "mac") == 0)
   result = "00:00:00:00:00:00";
  else if (strcmp(key, "gateway") == 0)
   result = "0.0.0.0";
  else if (strcmp(key, "dns") == 0)
   result = "0.0.0.0";
  else if (strcmp(key, "netmask") == 0)
   result = "255.255.255.0";
  else
   result = "";
 }

 {
 int rlen = (int)strlen(result);
 char *p = strpool_store(&rs->strpool, result, rlen);
 return bval_string(p, rlen);
 }
}
