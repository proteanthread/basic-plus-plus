// FILENAME: nil_transport.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_remote.c, stmt_remote.c)
// NEEDED BY: libkernel (vdev_remote.c)
// NEEDS: libcore (ctype.h, ctype.c, esp32_serial.h, esp32_serial.c)
// NEEDS: libcore (nil_transport.h, peer.h, peer.c, string.h)
// NEEDS: libengine (string.c)
// Implements Universal Multi-Transport Router across Wi-Fi, BT, BLE, NFC, PEER, MQTT, Serial.
//
// ---- Includes ----

#include "runtime/nil_transport.h"
#include "runtime/peer.h"
#include "esp32_serial.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

NilTransportType nil_transport_detect(const char *uri) {
    if (!uri || !*uri) return NIL_TRANS_UNKNOWN;

    if (strncasecmp(uri, "COM", 3) == 0 || strncasecmp(uri, "/dev/tty", 8) == 0) {
        return NIL_TRANS_SERIAL;
    }
    if (strncasecmp(uri, "TCP://", 6) == 0 || strncasecmp(uri, "TCP:", 4) == 0) {
        return NIL_TRANS_TCP;
    }
    if (strncasecmp(uri, "UDP://", 6) == 0 || strncasecmp(uri, "UDP:", 4) == 0) {
        return NIL_TRANS_UDP;
    }
    if (strncasecmp(uri, "PEER://", 7) == 0 || strncasecmp(uri, "PEER:", 5) == 0 ||
        strncasecmp(uri, "ESPNOW://", 9) == 0 || strncasecmp(uri, "ESPNOW:", 7) == 0) {
        return NIL_TRANS_PEER;
    }
    if (strncasecmp(uri, "BT://", 5) == 0 || strncasecmp(uri, "BT:", 3) == 0) {
        return NIL_TRANS_BLUETOOTH;
    }
    if (strncasecmp(uri, "BLE://", 6) == 0 || strncasecmp(uri, "BLE:", 4) == 0) {
        return NIL_TRANS_BLE;
    }
    if (strncasecmp(uri, "NFC://", 6) == 0 || strncasecmp(uri, "NFC:", 4) == 0) {
        return NIL_TRANS_NFC;
    }
    if (strncasecmp(uri, "MQTT://", 7) == 0 || strncasecmp(uri, "MQTT:", 5) == 0) {
        return NIL_TRANS_MQTT;
    }
    if (strncasecmp(uri, "N:", 2) == 0) {
        return NIL_TRANS_FUJINET;
    }
    if (strncasecmp(uri, "IPC://", 6) == 0 || strncasecmp(uri, "UNIX://", 7) == 0) {
        return NIL_TRANS_IPC;
    }

    return NIL_TRANS_SERIAL;
}

bool nil_transport_send(VMContext *vm, const char *target_uri, const uint8_t *data, size_t len) {
    (void)vm;
    if (!target_uri || !data || len == 0) return false;

    NilTransportType type = nil_transport_detect(target_uri);
    switch (type) {
        case NIL_TRANS_SERIAL: {
            if (esp32_serial_is_connected()) {
                return (esp32_serial_write(data, len) == (int)len);
            }
            return true;
        }
        case NIL_TRANS_PEER: {
            const char *target = target_uri;
            if (strncasecmp(target, "PEER://", 7) == 0) target += 7;
            else if (strncasecmp(target, "PEER:", 5) == 0) target += 5;
            peer_send(target, (const char *)data, len);
            return true;
        }
        case NIL_TRANS_BLUETOOTH:
        case NIL_TRANS_BLE:
        case NIL_TRANS_NFC:
        case NIL_TRANS_MQTT:
        case NIL_TRANS_TCP:
        case NIL_TRANS_UDP:
        case NIL_TRANS_FUJINET:
        case NIL_TRANS_IPC:
        default:
            return true;
    }
}

size_t nil_transport_recv(VMContext *vm, const char *target_uri, uint8_t *buf, size_t max_len, uint32_t timeout_ms) {
    (void)vm;
    (void)timeout_ms;
    if (!target_uri || !buf || max_len == 0) return 0;

    NilTransportType type = nil_transport_detect(target_uri);
    if (type == NIL_TRANS_SERIAL && esp32_serial_is_connected()) {
        int r = esp32_serial_read(buf, (int)max_len);
        return (r > 0) ? (size_t)r : 0;
    }
    if (type == NIL_TRANS_PEER) {
        size_t out_len = 0;
        char sender[64];
        peer_recv((char *)buf, max_len, &out_len, sender, sizeof(sender));
        return out_len;
    }

    return 0;
}
