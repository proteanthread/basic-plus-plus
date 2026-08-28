// FILENAME: packet_sniff.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_packet.c, stmt_sniff.c)
// NEEDS: libcore (memops.h, memops.c, packet_sniff.h, strops.h, strops.c)
// Implements packet sniffing and promiscuous capture interface.
//
// ---- Includes ----

#include "runtime/packet_sniff.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static SniffContext g_sniff_ctx = {0};

BppError packet_sniff_start(int channel, const char *filter) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    g_sniff_ctx.active = true;
    g_sniff_ctx.channel = (channel > 0) ? channel : 1;
    if (filter) {
        runtime_strncpy(g_sniff_ctx.filter, filter, sizeof(g_sniff_ctx.filter) - 1);
    } else {
        g_sniff_ctx.filter[0] = '\0';
    }

    // Populate simulated last packet for inspection
    runtime_strncpy(g_sniff_ctx.last_packet.src_mac, "AA:BB:CC:DD:EE:01", sizeof(g_sniff_ctx.last_packet.src_mac) - 1);
    runtime_strncpy(g_sniff_ctx.last_packet.dst_mac, "FF:FF:FF:FF:FF:FF", sizeof(g_sniff_ctx.last_packet.dst_mac) - 1);
    g_sniff_ctx.last_packet.rssi = -65;
    g_sniff_ctx.last_packet.channel = g_sniff_ctx.channel;
    g_sniff_ctx.last_packet.frame_type = 0; // Beacon frame
    runtime_strncpy(g_sniff_ctx.last_packet.src_ip, "192.168.1.1", sizeof(g_sniff_ctx.last_packet.src_ip) - 1);
    g_sniff_ctx.last_packet.src_port = 80;
    runtime_strncpy(g_sniff_ctx.last_packet.dst_ip, "192.168.1.255", sizeof(g_sniff_ctx.last_packet.dst_ip) - 1);
    g_sniff_ctx.last_packet.dst_port = 80;
    const char *payload_sim = "SSID:BASIC_NODE_AP";
    size_t plen = runtime_strlen(payload_sim);
    runtime_memcpy(g_sniff_ctx.last_packet.payload, payload_sim, plen);
    g_sniff_ctx.last_packet.payload_len = plen;
    g_sniff_ctx.packet_count = 1;

    return err;
}

BppError packet_sniff_stop(void) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    g_sniff_ctx.active = false;
    return err;
}

bool packet_sniff_is_active(void) {
    return g_sniff_ctx.active;
}

const SniffPacket *packet_sniff_get_last(void) {
    return &g_sniff_ctx.last_packet;
}

int packet_sniff_get_count(void) {
    return g_sniff_ctx.packet_count;
}

void packet_sniff_shutdown(void) {
    runtime_memset(&g_sniff_ctx, 0, sizeof(g_sniff_ctx));
}
