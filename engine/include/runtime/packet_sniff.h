// FILENAME: packet_sniff.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (packet_sniff.c)
// NEEDED BY: libengine (func_packet.c, stmt_sniff.c)
// NEEDS: libkernel (types.h)
// Declares packet sniffing and promiscuous capture interface.
//
// ---- Includes ----

#ifndef RUNTIME_PACKET_SNIFF_H
#define RUNTIME_PACKET_SNIFF_H

#include "types/types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SNIFF_MAX_PACKET_SIZE 1500

typedef struct {
    char     src_mac[32];
    char     dst_mac[32];
    int      rssi;
    int      channel;
    int      frame_type; // 0=mgmt, 1=control, 2=data
    char     src_ip[64];
    int      src_port;
    char     dst_ip[64];
    int      dst_port;
    uint8_t  payload[SNIFF_MAX_PACKET_SIZE];
    size_t   payload_len;
} SniffPacket;

typedef struct {
    bool        active;
    int         channel;
    char        filter[64];
    SniffPacket last_packet;
    int         packet_count;
} SniffContext;

BppError packet_sniff_start(int channel, const char *filter);
BppError packet_sniff_stop(void);
bool     packet_sniff_is_active(void);
const SniffPacket *packet_sniff_get_last(void);
int      packet_sniff_get_count(void);
void     packet_sniff_shutdown(void);

#endif // RUNTIME_PACKET_SNIFF_H
