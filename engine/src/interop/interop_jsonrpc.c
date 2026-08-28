// FILENAME: interop_jsonrpc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: libcore (interop_ipc.c)
// NEEDS: libcore (interop_jsonrpc.h, string.h)
// NEEDS: libengine (string.c)
// Provides core logic and interface definitions for interop_jsonrpc within BASIC++.
//
// ---- Includes ----

#include "interop/interop_jsonrpc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char* skip_whitespace(const char* p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static bool parse_string(const char** p, char* out, size_t max_len) {
    const char* str = *p;
    if (*str != '\"') return false;
    str++;
    size_t i = 0;
    while (*str && *str != '\"') {
        if (i < max_len - 1) {
            out[i++] = *str;
        }
        str++;
    }
    if (*str == '\"') {
        str++;
        out[i] = '\0';
        *p = str;
        return true;
    }
    return false;
}

bool jsonrpc_parse_request(const char* json_str, JsonRpcRequest* req) {
    if (!json_str || !req) return false;
    memset(req, 0, sizeof(JsonRpcRequest));
    
    const char* p = skip_whitespace(json_str);
    if (*p != '{') return false;
    p++;
    
    char key[64];
    while (*p && *p != '}') {
        p = skip_whitespace(p);
        if (*p == '\"') {
            if (parse_string(&p, key, sizeof(key))) {
                p = skip_whitespace(p);
                if (*p == ':') {
                    p++;
                    p = skip_whitespace(p);
                    if (strncmp(key, "method", 64) == 0) {
                        parse_string(&p, req->method, sizeof(req->method));
                    } else if (strncmp(key, "id", 64) == 0) {
                        req->id = atoi(p);
                        while (*p >= '0' && *p <= '9') p++;
                    } else {
                        while (*p && *p != ',' && *p != '}') p++;
                    }
                }
            }
        } else {
            p++;
        }
        p = skip_whitespace(p);
        if (*p == ',') p++;
    }
    
    return strlen(req->method) > 0;
}

void jsonrpc_format_response(const JsonRpcResponse* res, char* buffer, size_t buf_size) {
    if (!buffer || buf_size == 0) return;
    
    if (res->error_code != 0) {
        snprintf(buffer, buf_size, 
                 "{\"jsonrpc\":\"2.0\",\"error\":{\"code\":%d,\"message\":\"%s\"},\"id\":%d}",
                 res->error_code, res->error_message, res->id);
    } else {
        snprintf(buffer, buf_size, 
                 "{\"jsonrpc\":\"2.0\",\"result\":%s,\"id\":%d}",
                 res->result[0] ? res->result : "null", res->id);
    }
}

void jsonrpc_format_error(JsonRpcResponse* res, int error_code, const char* error_message) {
    if (!res) return;
    memset(res, 0, sizeof(JsonRpcResponse));
    res->error_code = error_code;
    if (error_message) {
        strncpy(res->error_message, error_message, sizeof(res->error_message) - 1);
    }
}
