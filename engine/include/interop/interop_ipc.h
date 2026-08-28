// FILENAME: interop_ipc.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: libcore (interop_ipc.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for interop_ipc within BASIC++.
//
// ---- Includes ----

#ifndef INTEROP_IPC_H
#define INTEROP_IPC_H

#include <stdbool.h>

// Forward declaration for VM context
typedef struct VMContext VMContext;

typedef enum {
    IPC_MODE_STDIO,
    IPC_MODE_TCP,
    IPC_MODE_PIPE
} InteropIpcMode;

typedef struct InteropIpcServer InteropIpcServer;

InteropIpcServer* interop_ipc_create(InteropIpcMode mode, const char* address, int port);
void interop_ipc_run(InteropIpcServer* server);
void interop_ipc_stop(InteropIpcServer* server);
void interop_ipc_destroy(InteropIpcServer* server);

// Convenience: create server, run event loop, destroy on exit.
// If port == 0, uses stdin/stdout mode. Otherwise uses TCP on the given port.
// Returns 0 on clean exit, non-zero on error.
int interop_ipc_serve(VMContext *vm, int port);

#endif // INTEROP_IPC_H

