// FILENAME: peer.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (nil_transport.c, peer.c)
// NEEDED BY: libengine (events_net.c, stmt_peer.c)
// NEEDS: libkernel (types.h)
// Declares universal peer-to-peer connectionless framing interface.
//
// ---- Includes ----

#ifndef RUNTIME_PEER_H
#define RUNTIME_PEER_H

#include "types/types.h"
#include <stddef.h>
#include <stdbool.h>

#define PEER_MAX_PEERS 16
#define PEER_MAX_PAYLOAD 250

typedef struct {
    char peer_id[64];
    int  channel;
    bool active;
} PeerNode;

typedef struct {
    PeerNode peers[PEER_MAX_PEERS];
    int      peer_count;
    int      default_channel;
    bool     initialized;
    char     last_recv_payload[PEER_MAX_PAYLOAD + 1];
    char     last_recv_sender[64];
    size_t   last_recv_len;
    bool     frame_pending;
} PeerContext;

BppError peer_init(int channel);
BppError peer_add(const char *peer_id, int channel);
BppError peer_send(const char *peer_id, const char *data, size_t len);
BppError peer_recv(char *out_buf, size_t max_len, size_t *out_len, char *out_sender, size_t sender_max);
bool     peer_is_frame_pending(void);
void     peer_shutdown(void);

#endif // RUNTIME_PEER_H
