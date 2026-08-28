// FILENAME: nil_transport.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (nil_transport.c)
// NEEDED BY: libengine (func_remote.c, stmt_remote.c)
// NEEDED BY: libkernel (vdev_remote.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Declares Universal Multi-Transport Router across Wi-Fi, BT, BLE, NFC, PEER, MQTT, Serial.
//
// ---- Includes ----

#ifndef RUNTIME_NIL_TRANSPORT_H
#define RUNTIME_NIL_TRANSPORT_H

#include "vm/vm.h"
#include "types/types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    NIL_TRANS_UNKNOWN = 0,
    NIL_TRANS_SERIAL,
    NIL_TRANS_TCP,
    NIL_TRANS_UDP,
    NIL_TRANS_PEER,
    NIL_TRANS_BLUETOOTH,
    NIL_TRANS_BLE,
    NIL_TRANS_NFC,
    NIL_TRANS_MQTT,
    NIL_TRANS_FUJINET,
    NIL_TRANS_IPC
} NilTransportType;

NilTransportType nil_transport_detect(const char *uri);
bool             nil_transport_send(VMContext *vm, const char *target_uri, const uint8_t *data, size_t len);
size_t           nil_transport_recv(VMContext *vm, const char *target_uri, uint8_t *buf, size_t max_len, uint32_t timeout_ms);

#endif // RUNTIME_NIL_TRANSPORT_H
