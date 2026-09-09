// FILENAME: dev_ipc.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel, vdev.c
// Declarations for Virtual Point-to-Point IPC Device (IPC:endpoint, IPC1..8:).
//
// ---- Includes ----

#ifndef DEV_IPC_H
#define DEV_IPC_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// Creates a virtual IPC device instance for the specified endpoint (or default "DEFAULT")
VDev dev_ipc_create(const char *endpoint_name);

#ifdef __cplusplus
}
#endif

#endif // DEV_IPC_H
