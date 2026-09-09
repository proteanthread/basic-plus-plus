// FILENAME: dev_cpm.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libkernel
// NEEDS: libkernel (vdev.h)
// CP/M 2.2 and MP/M peripheral virtual device driver (RDR, PUN, LST, AXI, AXO, BAT).
//
// ---- Includes ----

#ifndef DEVICE_DEV_CPM_H
#define DEVICE_DEV_CPM_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create a CP/M peripheral device instance:
// name: "PUN:" (Paper Tape Punch), "RDR:" (Paper Tape Reader), "LST:" (List Printer), "AXI:", "AXO:", "BAT:"
VDev dev_cpm_create(const char *name);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_CPM_H
