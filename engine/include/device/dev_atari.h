// FILENAME: dev_atari.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libkernel, libengine
// NEEDS: libkernel (vdev.h)
// Atari 8-bit Central Input/Output (CIO) device drivers and XIO handler.
//
// ---- Includes ----

#ifndef DEVICE_DEV_ATARI_H
#define DEVICE_DEV_ATARI_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create an Atari CIO device instance:
// dev_type: "E:" (Editor), "K:" (Keyboard), "S:" (Screen), "P:" (Printer), "C:" (Cassette), "R:" (RS-232), "V:" (Voice)
VDev dev_atari_create(const char *name);

// Handle Atari XIO command via virtual device bus
int dev_atari_xio_exec(VDev *dev, int cmd, int aux1, int aux2, const char *filespec);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_ATARI_H
