// FILENAME: dev_msg_bus.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel, vdev.c
// Declarations for Virtual Message Bus Device (BUS:topic, BUS1..8:).
//
// ---- Includes ----

#ifndef DEV_MSG_BUS_H
#define DEV_MSG_BUS_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// Creates a virtual message bus device instance for the specified topic (or default "GLOBAL")
VDev dev_msg_bus_create(const char *topic_name);

#ifdef __cplusplus
}
#endif

#endif // DEV_MSG_BUS_H
