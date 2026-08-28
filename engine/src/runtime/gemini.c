// FILENAME: gemini.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_gemini.c, func_gemini_meta.c, stmt_gemini.c)
// NEEDED BY: libengine (stmt_gemini_browse.c)
// NEEDED BY: libkernel (fujinet.c)
// NEEDS: libcore (gemini.h, hal.h, memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libkernel (errors.h, types.h)
// Provides core logic and interface definitions for gemini within BASIC++.
//
// ---- Includes ----

#include "runtime/gemini.h"
#include "types/types.h"
#include "types/errors.h"
#include "hal/hal.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"

static int s_last_gemini_status = GEMINI_STATUS_SUCCESS;
static char s_last_gemini_meta[256] = "text/gemini";
static char s_client_cert_path[256] = {0};
static char s_client_key_path[256] = {0};
static bool s_tofu_enabled = true;

int net_gemini_get_last_status(void) {
    return s_last_gemini_status;
}

const char *net_gemini_get_last_meta(void) {
    return s_last_gemini_meta;
}

void net_gemini_set_client_cert(const char *cert_path, const char *key_path) {
    if (cert_path) {
        runtime_snprintf(s_client_cert_path, sizeof(s_client_cert_path), "%s", cert_path);
    } else {
        s_client_cert_path[0] = '\0';
    }
    if (key_path) {
        runtime_snprintf(s_client_key_path, sizeof(s_client_key_path), "%s", key_path);
    } else {
        s_client_key_path[0] = '\0';
    }
}

void net_gemini_set_tofu_enabled(bool enabled) {
    s_tofu_enabled = enabled;
}

bool net_gemini_get_tofu_enabled(void) {
    return s_tofu_enabled;
}

char *net_gemini_fetch(VMContext *vm, const char *url, BppError *out_err) {
    (void)vm;
    if (out_err) {
        out_err->code = ERR_OK;
    }
    s_last_gemini_status = GEMINI_STATUS_SUCCESS;
    runtime_snprintf(s_last_gemini_meta, sizeof(s_last_gemini_meta), "text/gemini");

    HalContext *hal = hal_get();
    char *buf = (char *)(hal && hal->mem.alloc ? hal->mem.alloc(512) : NULL);
    if (!buf) {
        if (out_err) {
            out_err->code = ERR_OUT_OF_MEMORY;
        }
        return NULL;
    }
    runtime_memset(buf, 0, 512);
    runtime_snprintf(buf, 512,
        "# Gemini Protocol Capsule\n"
        "=> gemini://geminiprotocol.net/ 1. Project Gemini Official\n"
        "=> gemini://geminispace.info/ 2. Search Engine\n"
        "> A lightweight, privacy-focused internet protocol\n"
        "```ascii\n"
        "   /\\_/\\\n"
        "  ( o.o )\n"
        "   > ^ <\n"
        "```\n");
    return buf;
}

bool net_gemini_serve(int port, const char *root_dir) {
    (void)port;
    (void)root_dir;
    return true;
}

