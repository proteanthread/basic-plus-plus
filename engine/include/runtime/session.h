// FILENAME: session.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (session.c)
// NEEDED BY: libengine (echo.c, session_stmts.c, vm_internal.h)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for session within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_SESSION_H
#define RUNTIME_SESSION_H

#include "types/types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BppSessionContext {
    char     username[64];
    char     account[64];
    int      job_id;
    int      tty_id;
    int      priority;
    bool     echo_enabled;
    uint64_t login_timestamp;
    bool     logged_in;
} BppSessionContext;

void session_init(BppSessionContext *ctx);
bool session_login(BppSessionContext *ctx, const char *username, const char *account);
void session_logout(BppSessionContext *ctx);
const char *session_get_username(const BppSessionContext *ctx);
const char *session_get_account(const BppSessionContext *ctx);
int session_get_job_id(const BppSessionContext *ctx);
int session_get_tty_id(const BppSessionContext *ctx);
int session_get_priority(const BppSessionContext *ctx);
void session_set_priority(BppSessionContext *ctx, int prio);
bool session_get_echo(const BppSessionContext *ctx);
void session_set_echo(BppSessionContext *ctx, bool echo);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_SESSION_H
