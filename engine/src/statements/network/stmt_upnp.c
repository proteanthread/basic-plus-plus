// FILENAME: stmt_upnp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libserver (vnet_nat.h, stmt_upnp.h)
// NEEDS: libengine (eval.h, lexer.h, vm.h), libcore (memory.h, strings.h)
// Implements UPnP IGD and NAT-PMP port forwarding statements (UPNP.FORWARD, UPNP.UNFORWARD).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "statements/network/stmt_upnp.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "runtime/vnet_nat.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "types/types.h"
#include "vm/vm.h"

static bool is_subcommand(BppToken tok, const char *name) {
    if (!name) return false;
    size_t len = runtime_strlen(name);
    if (tok.type == TOK_KEYWORD && tok.start) {
        if (tok.length == len && runtime_strncasecmp(tok.start, name, len) == 0) return true;
    }
    if (tok.type == TOK_IDENT && tok.start) {
        if (tok.length == len && runtime_strncasecmp(tok.start, name, len) == 0) return true;
        if (tok.start[0] == '.' && tok.length == len + 1 && runtime_strncasecmp(tok.start + 1, name, len) == 0) return true;
    }
    return false;
}

BppError stmt_upnp_forward_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in UPNP.FORWARD";
        return err;
    }

    // 1. Evaluate external port
    BValue ext_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int ext_port = (int)ext_val.as.number;
    int int_port = ext_port;
    char proto[16] = "TCP";
    char desc[64] = "BASIC++ Forward";

    BppToken tok = lex_peek(lex);
    // 2. Optional internal port
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue int_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int_port = (int)int_val.as.number;
        tok = lex_peek(lex);

        // 3. Optional protocol
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue pr_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (pr_val.type == VAL_STRING && pr_val.as.string) {
                runtime_strncpy(proto, str_data(pr_val.as.string), sizeof(proto) - 1);
                proto[sizeof(proto) - 1] = '\0';
                str_release(vm_get_str(vm), pr_val.as.string);
            }
            tok = lex_peek(lex);

            // 4. Optional description
            if (tok.type == TOK_COMMA) {
                lex_next(lex);
                BValue ds_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (ds_val.type == VAL_STRING && ds_val.as.string) {
                    runtime_strncpy(desc, str_data(ds_val.as.string), sizeof(desc) - 1);
                    desc[sizeof(desc) - 1] = '\0';
                    str_release(vm_get_str(vm), ds_val.as.string);
                }
            }
        }
    }

    BppNatStatus st = vnet_nat_forward(ext_port, int_port, proto, desc, 0);
    if (st != NAT_STATUS_OK) {
        err.code = 57; // Device I/O error
        err.message = "UPnP port forward failed on gateway";
    }
    return err;
}

BppError stmt_upnp_unforward_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in UPNP.UNFORWARD";
        return err;
    }

    BValue ext_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int ext_port = (int)ext_val.as.number;
    char proto[16] = "TCP";

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue pr_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (pr_val.type == VAL_STRING && pr_val.as.string) {
            runtime_strncpy(proto, str_data(pr_val.as.string), sizeof(proto) - 1);
            proto[sizeof(proto) - 1] = '\0';
            str_release(vm_get_str(vm), pr_val.as.string);
        }
    }

    BppNatStatus st = vnet_nat_unforward(ext_port, proto);
    if (st != NAT_STATUS_OK) {
        err.code = 57;
        err.message = "UPnP port unforward failed on gateway";
    }
    return err;
}

BppError stmt_upnp_handler(VMContext *vm, LexerContext *lex) {
    if (!vm || !lex) {
        BppError err;
        runtime_memset(&err, 0, sizeof(err));
        err.code = 5; err.message = "Null context in UPNP";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (is_subcommand(tok, "FORWARD") || is_subcommand(tok, "FWD") || is_subcommand(tok, "MAP")) {
        lex_next(lex);
        return stmt_upnp_forward_handler(vm, lex);
    } else if (is_subcommand(tok, "UNFORWARD") || is_subcommand(tok, "UNFWD") || is_subcommand(tok, "UNMAP")) {
        lex_next(lex);
        return stmt_upnp_unforward_handler(vm, lex);
    }

    // Default: UPNP FORWARD
    return stmt_upnp_forward_handler(vm, lex);
}

static const LangDesc g_upnp_forward_desc = {
    .name = "UPNP.FORWARD",
    .category = "Hardware & Network",
    .syntax = "UPNP.FORWARD ext_port [, int_port [, proto$ [, desc$]]]",
    .description = "Configures dynamic UPnP / NAT-PMP port forwarding on the local gateway router.",
    .error_summary = "Error 57: UPnP port mapping failed on gateway",
    .subsystem = SUBSYSTEM_SERVER,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_upnp_unforward_desc = {
    .name = "UPNP.UNFORWARD",
    .category = "Hardware & Network",
    .syntax = "UPNP.UNFORWARD ext_port [, proto$]",
    .description = "Removes active UPnP / NAT-PMP port forwarding mapping from the gateway router.",
    .error_summary = "Error 57: UPnP port unforward failed on gateway",
    .subsystem = SUBSYSTEM_SERVER,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_upnp_register(void) {
    lang_desc_register(&g_upnp_forward_desc);
    lang_desc_register(&g_upnp_unforward_desc);
}
