// FILENAME: dev_log.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c, stmt_log.c
// NEEDS: libkernel (vdev.h), libcore (logger.h)
// Interface definitions for Virtual Logging Device (LOG:, LOG1..8:, SYSLOG:).
//
// ---- Includes ----

#ifndef DEVICE_DEV_LOG_H
#define DEVICE_DEV_LOG_H

#include "device/vdev.h"
#include "debug/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Creates a VDev virtual device instance for the LOG: logging stream.
// @param name Device identifier (e.g. "LOG:", "LOG1:", "SYSLOG:").
// @return Initialized VDev instance.
VDev dev_log_create(const char *name);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_LOG_H
