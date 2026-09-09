// FILENAME: dev_upnp.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel (vdev.c), libserver (vnet_nat.c)
// Declares UPnP virtual device interfaces.

#ifndef BASICPP_DEV_UPNP_H
#define BASICPP_DEV_UPNP_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

VDev dev_upnp_create(const char *name);

#ifdef __cplusplus
}
#endif

#endif // BASICPP_DEV_UPNP_H
