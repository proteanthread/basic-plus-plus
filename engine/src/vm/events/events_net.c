// FILENAME: events_net.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (events_poll.c, on_timer.c, stmt_net_config.c)
// NEEDED BY: libengine (stmt_peer.c, stmt_port_trigger.c)
// NEEDS: libcore (memops.h, memops.c, peer.h, peer.c)
// NEEDS: libengine (events_net.h)
// Implements network, port, peer, and sniffer event trapping.
//
// ---- Includes ----

#include "vm/events_net.h"
#include "runtime/peer.h"
#include "runtime/string/memops.h"

static NetEventsContext g_net_events = {0};

void vm_set_net_trap(VMContext *vm, BppLineNumber line, int state) {
    (void)vm;
    g_net_events.net_line = line;
    g_net_events.net_state = state;
}

void vm_set_peer_trap(VMContext *vm, BppLineNumber line, int state) {
    (void)vm;
    g_net_events.peer_line = line;
    g_net_events.peer_state = state;
}

void vm_set_sniff_trap(VMContext *vm, BppLineNumber line, int state) {
    (void)vm;
    g_net_events.sniff_line = line;
    g_net_events.sniff_state = state;
}

void vm_set_port_trap(VMContext *vm, int port, BppLineNumber line, int state) {
    (void)vm;
    for (int i = 0; i < g_net_events.port_count; ++i) {
        if (g_net_events.ports[i].port == port) {
            g_net_events.ports[i].target_line = line;
            g_net_events.ports[i].state = state;
            return;
        }
    }
    if (g_net_events.port_count < MAX_PORT_TRAPS) {
        int idx = g_net_events.port_count++;
        g_net_events.ports[idx].port = port;
        g_net_events.ports[idx].target_line = line;
        g_net_events.ports[idx].state = state;
        g_net_events.ports[idx].pending = false;
    }
}

void vm_trigger_port_event(VMContext *vm, int port) {
    (void)vm;
    for (int i = 0; i < g_net_events.port_count; ++i) {
        if (g_net_events.ports[i].port == port && g_net_events.ports[i].state == 1) {
            g_net_events.ports[i].pending = true;
        }
    }
}

bool vm_check_net_events(VMContext *vm, BppLineNumber *out_line) {
    (void)vm;
    if (!out_line) return false;

    // Check Peer Trap
    if (g_net_events.peer_state == 1 && (g_net_events.peer_pending || peer_is_frame_pending())) {
        g_net_events.peer_pending = false;
        *out_line = g_net_events.peer_line;
        return true;
    }

    // Check Port Traps
    for (int i = 0; i < g_net_events.port_count; ++i) {
        if (g_net_events.ports[i].state == 1 && g_net_events.ports[i].pending) {
            g_net_events.ports[i].pending = false;
            *out_line = g_net_events.ports[i].target_line;
            return true;
        }
    }

    // Check Generic Net Trap
    if (g_net_events.net_state == 1 && g_net_events.net_pending) {
        g_net_events.net_pending = false;
        *out_line = g_net_events.net_line;
        return true;
    }

    // Check Sniff Trap
    if (g_net_events.sniff_state == 1 && g_net_events.sniff_pending) {
        g_net_events.sniff_pending = false;
        *out_line = g_net_events.sniff_line;
        return true;
    }

    return false;
}
