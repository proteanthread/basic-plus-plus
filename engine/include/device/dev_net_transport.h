// FILENAME: dev_net_transport.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libkernel
// NEEDS: libkernel (vdev.h)
// Direct network transport device drivers (MAC, TCP, UDP, NET).
//
// ---- Includes ----

#ifndef DEVICE_DEV_NET_TRANSPORT_H
#define DEVICE_DEV_NET_TRANSPORT_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create network transport device (e.g. "MAC:", "TCP:", "UDP:", "NET:")
VDev dev_net_transport_create(const char *name);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_NET_TRANSPORT_H
