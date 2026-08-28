// FILENAME: gemini.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (gemini.c)
// NEEDED BY: libengine (func_gemini.c, func_gemini_meta.c, stmt_gemini.c)
// NEEDED BY: libengine (stmt_gemini_browse.c)
// NEEDED BY: libkernel (fujinet.c)
// NEEDS: libengine (vm.h)
// Provides core logic and interface definitions for gemini within BASIC++.
//
// ---- Includes ----

#ifndef GEMINI_H
#define GEMINI_H

#include "vm/vm.h"
#include <stdbool.h>

#define GEMINI_STATUS_INPUT                10
#define GEMINI_STATUS_SENSITIVE_INPUT      11
#define GEMINI_STATUS_SUCCESS              20
#define GEMINI_STATUS_REDIRECT_TEMP        30
#define GEMINI_STATUS_REDIRECT_PERM        31
#define GEMINI_STATUS_TEMP_FAIL            40
#define GEMINI_STATUS_SERVER_UNAVAILABLE   41
#define GEMINI_STATUS_CGI_ERROR            42
#define GEMINI_STATUS_PROXY_ERROR          43
#define GEMINI_STATUS_SLOW_DOWN            44
#define GEMINI_STATUS_PERM_FAIL            50
#define GEMINI_STATUS_NOT_FOUND            51
#define GEMINI_STATUS_GONE                 52
#define GEMINI_STATUS_PROXY_REFUSED        53
#define GEMINI_STATUS_BAD_REQUEST          59
#define GEMINI_STATUS_CLIENT_CERT_REQ      60
#define GEMINI_STATUS_CERT_NOT_AUTH        61
#define GEMINI_STATUS_CERT_NOT_VALID       62

// @brief Fetch the payload of a gemini:// URL.
// @param vm The VM Context.
// @param url The gemini:// URL.
// @param out_err Pointer to retrieve any runtime errors.
// @return The allocated string response payload (must be freed by caller), or NULL on failure.
char *net_gemini_fetch(VMContext *vm, const char *url, BppError *out_err);

// @brief Starts an embedded Gemini protocol server on given port (default 1965).
bool net_gemini_serve(int port, const char *root_dir);

int net_gemini_get_last_status(void);
const char *net_gemini_get_last_meta(void);
void net_gemini_set_client_cert(const char *cert_path, const char *key_path);
void net_gemini_set_tofu_enabled(bool enabled);
bool net_gemini_get_tofu_enabled(void);

#endif // GEMINI_H
