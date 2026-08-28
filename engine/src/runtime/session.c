// FILENAME: session.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (echo.c, session_stmts.c, vm_internal.h)
// NEEDS: libcore (session.h, string.h)
// NEEDS: libengine (string.c, time.h, time.c)
// Provides core logic and interface definitions for session within BASIC++.
//
// ---- Includes ----

#include "runtime/session.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

void session_init(BppSessionContext *ctx) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(BppSessionContext));
    strncpy(ctx->username, "USER", sizeof(ctx->username) - 1);
    strncpy(ctx->account, "PUBLIC", sizeof(ctx->account) - 1);
    ctx->job_id = 1;
    ctx->tty_id = 0;
    ctx->priority = 5;
    ctx->echo_enabled = true;
    ctx->login_timestamp = (uint64_t)time(NULL);
    ctx->logged_in = true;
}

bool session_login(BppSessionContext *ctx, const char *username, const char *account) {
    if (!ctx) return false;
    if (username && username[0]) {
        strncpy(ctx->username, username, sizeof(ctx->username) - 1);
        ctx->username[sizeof(ctx->username) - 1] = '\0';
    }
    if (account && account[0]) {
        strncpy(ctx->account, account, sizeof(ctx->account) - 1);
        ctx->account[sizeof(ctx->account) - 1] = '\0';
    }
    ctx->login_timestamp = (uint64_t)time(NULL);
    ctx->logged_in = true;
    return true;
}

void session_logout(BppSessionContext *ctx) {
    if (!ctx) return;
    strncpy(ctx->username, "GUEST", sizeof(ctx->username) - 1);
    strncpy(ctx->account, "NONE", sizeof(ctx->account) - 1);
    ctx->logged_in = false;
}

const char *session_get_username(const BppSessionContext *ctx) {
    return ctx ? ctx->username : "USER";
}

const char *session_get_account(const BppSessionContext *ctx) {
    return ctx ? ctx->account : "PUBLIC";
}

int session_get_job_id(const BppSessionContext *ctx) {
    return ctx ? ctx->job_id : 1;
}

int session_get_tty_id(const BppSessionContext *ctx) {
    return ctx ? ctx->tty_id : 0;
}

int session_get_priority(const BppSessionContext *ctx) {
    return ctx ? ctx->priority : 5;
}

void session_set_priority(BppSessionContext *ctx, int prio) {
    if (!ctx) return;
    if (prio < 1) prio = 1;
    if (prio > 10) prio = 10;
    ctx->priority = prio;
}

bool session_get_echo(const BppSessionContext *ctx) {
    return ctx ? ctx->echo_enabled : true;
}

void session_set_echo(BppSessionContext *ctx, bool echo) {
    if (!ctx) return;
    ctx->echo_enabled = echo;
}
