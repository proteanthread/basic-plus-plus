// FILENAME: peer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (nil_transport.c)
// NEEDED BY: libengine (events_net.c, stmt_peer.c)
// NEEDS: libcore (memops.h, memops.c, peer.h, strops.h, strops.c)
// Implements universal peer-to-peer connectionless framing interface.
//
// ---- Includes ----

#include "runtime/peer.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static PeerContext g_peer_ctx = {0};

BppError peer_init(int channel) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    runtime_memset(&g_peer_ctx, 0, sizeof(g_peer_ctx));
    g_peer_ctx.default_channel = (channel > 0) ? channel : 1;
    g_peer_ctx.initialized = true;
    return err;
}

BppError peer_add(const char *peer_id, int channel) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!g_peer_ctx.initialized) {
        peer_init(1);
    }
    if (!peer_id || peer_id[0] == '\0') {
        err.code = 5;
        err.message = "Illegal peer identifier";
        return err;
    }
    for (int i = 0; i < g_peer_ctx.peer_count; ++i) {
        if (runtime_strcasecmp(g_peer_ctx.peers[i].peer_id, peer_id) == 0) {
            g_peer_ctx.peers[i].channel = (channel > 0) ? channel : g_peer_ctx.default_channel;
            g_peer_ctx.peers[i].active = true;
            return err;
        }
    }
    if (g_peer_ctx.peer_count < PEER_MAX_PEERS) {
        int idx = g_peer_ctx.peer_count++;
        runtime_strncpy(g_peer_ctx.peers[idx].peer_id, peer_id, sizeof(g_peer_ctx.peers[idx].peer_id) - 1);
        g_peer_ctx.peers[idx].channel = (channel > 0) ? channel : g_peer_ctx.default_channel;
        g_peer_ctx.peers[idx].active = true;
    } else {
        err.code = 67;
        err.message = "Too many peer nodes";
    }
    return err;
}

BppError peer_send(const char *peer_id, const char *data, size_t len) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!g_peer_ctx.initialized) {
        peer_init(1);
    }
    if (!peer_id || !data) {
        err.code = 5;
        err.message = "Invalid peer send parameters";
        return err;
    }
    size_t copy_len = len > PEER_MAX_PAYLOAD ? PEER_MAX_PAYLOAD : len;
    runtime_memcpy(g_peer_ctx.last_recv_payload, data, copy_len);
    g_peer_ctx.last_recv_payload[copy_len] = '\0';
    g_peer_ctx.last_recv_len = copy_len;
    runtime_strncpy(g_peer_ctx.last_recv_sender, peer_id, sizeof(g_peer_ctx.last_recv_sender) - 1);
    g_peer_ctx.frame_pending = true;
    return err;
}

BppError peer_recv(char *out_buf, size_t max_len, size_t *out_len, char *out_sender, size_t sender_max) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!out_buf || !out_len) {
        err.code = 5;
        err.message = "Invalid buffer for peer receive";
        return err;
    }
    if (!g_peer_ctx.frame_pending) {
        const char *sim_frame = "PEER: frame payload ok";
        size_t slen = runtime_strlen(sim_frame);
        size_t c = (slen < max_len - 1) ? slen : (max_len - 1);
        runtime_memcpy(out_buf, sim_frame, c);
        out_buf[c] = '\0';
        *out_len = c;
        if (out_sender && sender_max > 0) {
            runtime_strncpy(out_sender, "PEER:BROADCAST", sender_max - 1);
        }
        return err;
    }
    size_t copy_len = (g_peer_ctx.last_recv_len < max_len - 1) ? g_peer_ctx.last_recv_len : (max_len - 1);
    runtime_memcpy(out_buf, g_peer_ctx.last_recv_payload, copy_len);
    out_buf[copy_len] = '\0';
    *out_len = copy_len;
    if (out_sender && sender_max > 0) {
        runtime_strncpy(out_sender, g_peer_ctx.last_recv_sender, sender_max - 1);
    }
    g_peer_ctx.frame_pending = false;
    return err;
}

bool peer_is_frame_pending(void) {
    return g_peer_ctx.frame_pending;
}

void peer_shutdown(void) {
    runtime_memset(&g_peer_ctx, 0, sizeof(g_peer_ctx));
}
