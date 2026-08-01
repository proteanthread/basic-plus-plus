/**
 * @file stmt_infra.c
 * @brief Statement handlers for Virtual Infrastructure Integration (VFS, VNet, USB, VCON, Bus).
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements commands for MOUNT, UMOUNT, CHVT, NET, and OUT.
 * - Why it exists: Exposes the virtualized network, console, filesystem, and port I/O
 *   capabilities directly to BASIC++ program scripts.
 * - Why it works this way: It parses statement tokens ephemerally, executes parameters via
 *   eval_expression, and calls the respective runtime/device integration libraries.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Extension options (like mount type mappings), syntax extensions.
 * - What cannot be changed: Obligation to check sandbox capability limits prior to operations.
 * - What to expect: Invalid paths or negative port values will trigger execution errors.
 * - What to do if something breaks: If a mount fails, verify prefix format (must end with ':').
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Port numbers fit in 16-bit registers (0-65535). Sockets are cross-platform safe.
 * - Portability concerns: Sockets use platform wrappers inside vnet.c.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add plugin-hooks for external USB devices.
 * - How to write external extensions: External plugins call stmt_register to map custom commands.
 */

#include "bpp_stmt.h"
#include "bpp_vfs.h"
#include "bpp_vnet.h"
#include "bpp_usb.h"
#include "bpp_vcon.h"
#include "bpp_bus.h"
#include "bpp_eval.h"
#include "bpp_config.h"
#include "bpp_security.h"
#ifndef BPP_LITE_BUILD
#include "bpp_segmented_mem.h"
#endif
#include "bpp_variables.h"
#include "bpp_strings.h"
#include "bpp_gemini.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if BPP_SUPPORT_NET
/**
 * @brief MOUNT <prefix$>, <target$> [, <type>]
 */
BppError stmt_mount_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Sandbox Check: requires SECOP_FILE_READ/WRITE/BLOCK permission */
    if (security_check(SECOP_FILE_BLOCK, 0) != 0) {
        err.code = 70; err.message = "Permission denied: MOUNT is gated";
        return err;
    }

    BValue prefix_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (prefix_val.type != VAL_STRING) {
        err.code = 13; err.message = "MOUNT prefix must be a string";
        return err;
    }

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' after MOUNT prefix";
        str_release(vm_get_str(vm), prefix_val.as.string);
        return err;
    }

    BValue target_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        str_release(vm_get_str(vm), prefix_val.as.string);
        return err;
    }
    if (target_val.type != VAL_STRING) {
        err.code = 13; err.message = "MOUNT target must be a string";
        str_release(vm_get_str(vm), prefix_val.as.string);
        return err;
    }

    BppMountType type = MNT_DIR;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex); /* Consume ',' */
        BValue type_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            str_release(vm_get_str(vm), prefix_val.as.string);
            str_release(vm_get_str(vm), target_val.as.string);
            return err;
        }
        if (type_val.type == VAL_NUMBER) {
            int val = (int)type_val.as.number;
            if (val == 1) type = MNT_ZIP;
            else if (val == 2) type = MNT_DISK;
        }
    }

    const char *pstr = str_data(prefix_val.as.string);
    const char *tstr = str_data(target_val.as.string);

    if (!vfs_mount(vm_get_vfs(vm), pstr, tstr, type)) {
        err.code = 57; err.message = "MOUNT operation failed";
    }

    str_release(vm_get_str(vm), prefix_val.as.string);
    str_release(vm_get_str(vm), target_val.as.string);
    return err;
}

/**
 * @brief UMOUNT <prefix$>
 */
BppError stmt_umount_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (security_check(SECOP_FILE_BLOCK, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    BValue prefix_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (prefix_val.type != VAL_STRING) {
        err.code = 13; err.message = "UMOUNT prefix must be a string";
        return err;
    }

    const char *pstr = str_data(prefix_val.as.string);
    if (!vfs_umount(vm_get_vfs(vm), pstr)) {
        err.code = 57; err.message = "UMOUNT operation failed";
    }

    str_release(vm_get_str(vm), prefix_val.as.string);
    return err;
}
#endif

/**
 * @brief CHVT <index>
 */
BppError stmt_chvt_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue idx_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (idx_val.type == VAL_STRING) {
        err.code = 13; err.message = "CHVT index must be numeric";
        return err;
    }

    int idx = (int)idx_val.as.number;
    if (!vcon_select(vm_get_vcon(vm), idx)) {
        err.code = 5; err.message = "Invalid console index";
    }

    return err;
}

/**
 * @brief OUT <port>, <value>
 */
BppError stmt_out_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Gated under system level sandbox */
    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err.code = 70; err.message = "Permission denied: Port I/O is restricted";
        return err;
    }

    BValue port_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (port_val.type == VAL_STRING) {
        err.code = 13; err.message = "Port number must be numeric";
        return err;
    }

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' after port number";
        return err;
    }

    BValue val_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val_val.type == VAL_STRING) {
        err.code = 13; err.message = "Port value must be numeric";
        return err;
    }

    vdev_bus_out((int)port_val.as.number, (int)val_val.as.number);
    return err;
}

#if BPP_SUPPORT_NET
/**
 * @brief NET OPEN <channel>, <protocol$>, <host$>, <port>
 *        NET SEND <channel>, <data$>
 *        NET RECV <channel>, <var$>
 *        NET CLOSE <channel>
 */
BppError stmt_net_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Network access is capability gated under standard sandbox rules */
    if (security_check(SECOP_NETWORK, 0) != 0) {
        err.code = 70; err.message = "Permission denied: Network access restricted";
        return err;
    }

    BppToken tok = lex_next(lex);
    char action[32] = {0};
    size_t alen = (tok.length < sizeof(action) - 1) ? tok.length : sizeof(action) - 1;
    memcpy(action, tok.start, alen);
    for (size_t i = 0; i < alen; i++) action[i] = (char)toupper((unsigned char)action[i]);

    if (strcmp(action, "OPEN") == 0) {
        /* NET OPEN channel, protocol$, host$, port */
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' after channel";
            return err;
        }

        BValue proto_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' after protocol";
            str_release(vm_get_str(vm), proto_val.as.string);
            return err;
        }

        BValue host_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            str_release(vm_get_str(vm), proto_val.as.string);
            return err;
        }

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' after host";
            str_release(vm_get_str(vm), proto_val.as.string);
            str_release(vm_get_str(vm), host_val.as.string);
            return err;
        }

        BValue port_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            str_release(vm_get_str(vm), proto_val.as.string);
            str_release(vm_get_str(vm), host_val.as.string);
            return err;
        }

        err = vnet_open(vm_get_vnet(vm), (int)ch_val.as.number,
                        str_data(proto_val.as.string),
                        str_data(host_val.as.string),
                        (int)port_val.as.number);

        str_release(vm_get_str(vm), proto_val.as.string);
        str_release(vm_get_str(vm), host_val.as.string);

    } else if (strcmp(action, "SEND") == 0) {
        /* NET SEND channel, data$ */
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' after channel";
            return err;
        }

        BValue data_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (data_val.type != VAL_STRING) {
            err.code = 13; err.message = "NET SEND data must be a string";
            return err;
        }

        err = vnet_send(vm_get_vnet(vm), (int)ch_val.as.number,
                        str_data(data_val.as.string),
                        str_len(data_val.as.string));

        str_release(vm_get_str(vm), data_val.as.string);

    } else if (strcmp(action, "RECV") == 0) {
        /* NET RECV channel, var$ */
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' after channel";
            return err;
        }

        tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected target variable identifier in NET RECV";
            return err;
        }

        char var_name[256];
        size_t vlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
        memcpy(var_name, tok.as.string, vlen);
        var_name[vlen] = '\0';

        BValue *target_var = var_lookup(vm_get_var(vm), var_name, true);
        if (!target_var) {
            err.code = 2; err.message = "Undeclared variable in NET RECV (OPTION EXPLICIT)";
            return err;
        }

        char buf[2048];
        size_t out_len = 0;
        err = vnet_recv(vm_get_vnet(vm), (int)ch_val.as.number, buf, sizeof(buf), &out_len);
        if (err.code == 0) {
            BValue assign_val;
            memset(&assign_val, 0, sizeof(assign_val));
            assign_val.type = VAL_STRING;
            assign_val.as.string = str_create(vm_get_str(vm), buf, out_len);

            if (!var_assign(vm_get_var(vm), var_name, assign_val)) {
                err.code = 13; err.message = "Type mismatch assigning NET RECV to variable";
            }
            str_release(vm_get_str(vm), assign_val.as.string);
        }

    } else if (strcmp(action, "CLOSE") == 0) {
        /* NET CLOSE channel */
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        vnet_close(vm_get_vnet(vm), (int)ch_val.as.number);

    } else if (strcmp(action, "SCAN") == 0) {
        /* NET SCAN target$ [FOR ports$] INTO results$() */
        BValue target_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (target_val.type != VAL_STRING) {
            err.code = 13; err.message = "NET SCAN target must be a string";
            return err;
        }
        str_release(vm_get_str(vm), target_val.as.string);

        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_KEYWORD && next_tok.as.keyword == KW_FOR) {
            lex_next(lex);
            BValue ports_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (ports_val.type == VAL_STRING) {
                str_release(vm_get_str(vm), ports_val.as.string);
            }
            next_tok = lex_peek(lex);
        }

        if (next_tok.type == TOK_IDENT) {
            char tname[32] = {0};
            size_t tlen = (next_tok.length < sizeof(tname)-1) ? next_tok.length : sizeof(tname)-1;
            memcpy(tname, next_tok.start, tlen);
            for (size_t i=0; i<tlen; i++) tname[i] = (char)toupper((unsigned char)tname[i]);
            if (strcmp(tname, "INTO") == 0) {
                lex_next(lex);
                BppToken arr_tok = lex_next(lex);
                (void)arr_tok;
            }
        }

    } else if (strcmp(action, "CAPTURE") == 0 || strcmp(action, "INJECT") == 0) {
        /* NET CAPTURE / NET INJECT dummy handler */
        BValue dummy = eval_expression(vm, lex, &err);
        if (err.code == 0 && dummy.type == VAL_STRING) {
            str_release(vm_get_str(vm), dummy.as.string);
        }
    } else {
        err.code = 2; err.message = "Unknown NET command action";
    }

    return err;
}
#endif

/**
 * @brief POKE <address>, <value>
 */
BppError stmt_poke_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* POKE requires SECOP_MEM_WRITE permission */
    if (security_check(SECOP_MEM_WRITE, 0) != 0) {
        err.code = 70; err.message = "Permission denied: POKE is restricted";
        return err;
    }

    BValue addr_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (addr_val.type == VAL_STRING) {
        err.code = 13; err.message = "POKE address must be numeric";
        return err;
    }

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' after address in POKE";
        return err;
    }

    BValue val_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val_val.type == VAL_STRING) {
        err.code = 13; err.message = "POKE value must be numeric";
        return err;
    }

#ifndef BPP_LITE_BUILD
    /* 3. Delegate to vmem engine first, fallback to bus */
    if (vmem_poke(vm_get_vmem(vm), (uint16_t)addr_val.as.number, (uint8_t)val_val.as.number) == 1) {
        return err;
    }
#endif
    bool intercepted = false;
    vdev_bus_poke((unsigned long)addr_val.as.number, (uint8_t)val_val.as.number, &intercepted);
    return err;
}

#if BPP_SUPPORT_GEMINI
BppError stmt_gemini_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* 1. Parse URL expression */
    BValue url_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (url_val.type != VAL_STRING) {
        err.code = 2; err.message = "Expected string expression for URL";
        return err;
    }

    /* Expect comma separator */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' between URL and variable";
        if (url_val.type == VAL_STRING) str_release(vm_get_str(vm), url_val.as.string);
        return err;
    }

    /* 2. Parse destination variable name */
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && (tok.type != TOK_KEYWORD || tok.as.keyword < 1000)) {
        err.code = 2; err.message = "Expected variable name for response payload";
        if (url_val.type == VAL_STRING) str_release(vm_get_str(vm), url_val.as.string);
        return err;
    }

    char var_name[256];
    size_t len = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
    memcpy(var_name, tok.as.string, len);
    var_name[len] = '\0';

    /* Fetch payload */
    const char *url_str = str_data(url_val.as.string);
    char *payload = net_gemini_fetch(vm, url_str, &err);
    str_release(vm_get_str(vm), url_val.as.string);

    if (err.code != 0) {
        if (payload) free(payload);
        return err;
    }

    /* Assign payload to variable */
    BValue pay_val;
    memset(&pay_val, 0, sizeof(pay_val));
    pay_val.type = VAL_STRING;
    pay_val.as.string = str_create(vm_get_str(vm), payload, strlen(payload));
    free(payload);

    BppError assign_err;
    memset(&assign_err, 0, sizeof(assign_err));
    if (!var_assign(vm_get_var(vm), var_name, pay_val)) {
        assign_err.code = 2; assign_err.message = "Variable assignment failed";
    }
    if (pay_val.as.string) str_release(vm_get_str(vm), pay_val.as.string);

    return assign_err;
}
#endif
