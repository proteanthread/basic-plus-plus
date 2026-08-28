// FILENAME: events_net.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (events_net.c, events_poll.c, on_timer.c)
// NEEDED BY: libengine (stmt_net_config.c, stmt_peer.c, stmt_port_trigger.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Declares network, port, peer, and sniffer event trapping structures and triggers.
//
// ---- Includes ----

#ifndef VM_EVENTS_NET_H
#define VM_EVENTS_NET_H

#include "types/types.h"
#include "vm/vm.h"
#include <stdbool.h>

#define MAX_PORT_TRAPS 16

typedef struct {
    int           port;
    BppLineNumber target_line;
    int           state; // 0=OFF, 1=ON, 2=STOP
    bool          pending;
} PortTrap;

typedef struct {
    BppLineNumber net_line;
    int           net_state;
    bool          net_pending;

    BppLineNumber peer_line;
    int           peer_state;
    bool          peer_pending;

    BppLineNumber sniff_line;
    int           sniff_state;
    bool          sniff_pending;

    PortTrap      ports[MAX_PORT_TRAPS];
    int           port_count;
} NetEventsContext;

void vm_set_net_trap(VMContext *vm, BppLineNumber line, int state);
void vm_set_peer_trap(VMContext *vm, BppLineNumber line, int state);
void vm_set_sniff_trap(VMContext *vm, BppLineNumber line, int state);
void vm_set_port_trap(VMContext *vm, int port, BppLineNumber line, int state);
void vm_trigger_port_event(VMContext *vm, int port);
bool vm_check_net_events(VMContext *vm, BppLineNumber *out_line);

#endif // VM_EVENTS_NET_H
