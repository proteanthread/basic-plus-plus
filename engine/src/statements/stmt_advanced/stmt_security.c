/**
 * @file stmt_security.c
 * @brief SECURITY statement parser.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Parses the SECURITY statement, allowing query and configuration of security sandboxes.
 * - Why it exists: Exposes the security manager APIs to BASIC++ program space.
 * - Why it works this way: It decodes sub-arguments (LEVEL, RESTRICT, LIST, RESET), checks for one-way ratchet
 *   validity, and prints diagnostics or raises errors.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Specific formatting of help text or printed status messages.
 * - What cannot be changed: Security ratchet validation (cannot lower level).
 * - What to expect: Invalid subcommands raise Type Mismatch or Syntax Errors.
 * - What to do if something breaks: Trace parsing tokens and check enum translations.
 */

#ifndef BASIC_LITE_BUILD

#include "stmt/stmt.h"
#include "lexer/lexer.h"
#include "security/security.h"
#include "device/vdev.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int str_case_compare(const char *a, const char *b) {
    while (*a && *b) {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) {
            return toupper((unsigned char)*a) - toupper((unsigned char)*b);
        }
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

BppError stmt_security_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    VDevContext *vdev = vm_get_vdev(vm);

    BppToken tok = lex_peek(lex);
    BppSecLevel cur_level = security_get_level();

    /* 1. SECURITY (no args) */
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        vdev_printf(vdev, "Security Level: %s", security_level_name(cur_level));
        switch (cur_level) {
            case SEC_OPEN:
                vdev_printf(vdev, " (no restrictions)\n");
                break;
            case SEC_SAFE:
                vdev_printf(vdev, " (no system shell/usb access)\n");
                break;
            case SEC_STANDARD:
                vdev_printf(vdev, " (controlled sandbox)\n");
                break;
            case SEC_EDUCATIONAL:
                vdev_printf(vdev, " (classroom mode)\n");
                break;
            case SEC_RESTRICTED:
                vdev_printf(vdev, " (very limited sandbox)\n");
                break;
            case SEC_PARANOID:
                vdev_printf(vdev, " (pure computation only)\n");
                break;
            default:
                vdev_printf(vdev, "\n");
                break;
        }
        return err;
    }

    /* 2. SECURITY "level_name" or SECURITY subcommands */
    if (tok.type == TOK_STRING) {
        lex_next(lex);
        char s_val[128];
        size_t len = (tok.length < sizeof(s_val) - 1) ? tok.length : sizeof(s_val) - 1;
        memcpy(s_val, tok.as.string, len);
        s_val[len] = '\0';

        /* Subcommand: RESTRICT */
        if (str_case_compare(s_val, "RESTRICT") == 0) {
            BppToken next_tok = lex_peek(lex);
            if (next_tok.type != TOK_STRING) {
                err.code = 2; /* Syntax error */
                err.message = "Expected string target for RESTRICT";
                return err;
            }
            lex_next(lex);
            char op_str[128];
            size_t op_len = (next_tok.length < sizeof(op_str) - 1) ? next_tok.length : sizeof(op_str) - 1;
            memcpy(op_str, next_tok.as.string, op_len);
            op_str[op_len] = '\0';

            /* Check if keyword restriction */
            if (str_case_compare(op_str, "KEYWORD") == 0) {
                BppToken kw_tok = lex_peek(lex);
                if (kw_tok.type != TOK_STRING) {
                    err.code = 2;
                    err.message = "Expected keyword name string";
                    return err;
                }
                lex_next(lex);
                char kw_name[128];
                size_t kw_len = (kw_tok.length < sizeof(kw_name) - 1) ? kw_tok.length : sizeof(kw_name) - 1;
                memcpy(kw_name, kw_tok.as.string, kw_len);
                kw_name[kw_len] = '\0';

                /* Map keyword name to ID */
                BppKeywordId target_kw = lex_find_keyword_by_name(kw_name);

                if (target_kw != KW_NONE) {
                    security_restrict_keyword((int)target_kw);
                    vdev_printf(vdev, "Restricted keyword: %s\n", kw_name);
                } else {
                    vdev_printf(vdev, "Keyword '%s' not restricted (not supported or unknown)\n", kw_name);
                }
                return err;
            }

            /* Map operation name to enum */
            BppSecOperation target_op = SECOP_COUNT;
            if (str_case_compare(op_str, "file read") == 0) target_op = SECOP_FILE_READ;
            else if (str_case_compare(op_str, "file write") == 0) target_op = SECOP_FILE_WRITE;
            else if (str_case_compare(op_str, "file management") == 0) target_op = SECOP_FILE_MGMT;
            else if (str_case_compare(op_str, "system") == 0) target_op = SECOP_SYSTEM;
            else if (str_case_compare(op_str, "network") == 0) target_op = SECOP_NETWORK;

            if (target_op != SECOP_COUNT) {
                security_restrict_op(target_op);
                vdev_printf(vdev, "Restricted operation: %s\n", op_str);
            } else {
                err.code = 2;
                err.message = "Unknown operation name for RESTRICT";
            }
            return err;
        }

        /* Subcommand: LIST */
        if (str_case_compare(s_val, "LIST") == 0) {
            security_restrict_list();
            return err;
        }

        /* Subcommand: RESET */
        if (str_case_compare(s_val, "RESET") == 0) {
            if (cur_level >= SEC_STANDARD) {
                vdev_printf(vdev, "Cannot RESET restrictions under security level: %s\n", security_level_name(cur_level));
            } else {
                security_init(cur_level);
                vdev_printf(vdev, "Security restrictions reset.\n");
            }
            return err;
        }

        /* Otherwise, treat as named security level */
        int idx = security_find_level_by_name(s_val);
        if (idx == -1) {
            err.code = 2;
            err.message = "Unknown security level name";
            return err;
        }

        if ((BppSecLevel)idx < cur_level) {
            vdev_printf(vdev, "Cannot lower security from %s to %s.\n",
                        security_level_name(cur_level),
                        security_level_name((BppSecLevel)idx));
            return err;
        }

        security_set_level((BppSecLevel)idx);
        vdev_printf(vdev, "Security Level raised to: %s\n", security_level_name((BppSecLevel)idx));
        return err;
    }

    /* 3. SECURITY LEVEL n */
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_LEVEL) {
        lex_next(lex);
        BppToken val_tok = lex_next(lex);
        if (val_tok.type != TOK_NUMBER) {
            err.code = 2;
            err.message = "Expected level number after LEVEL";
            return err;
        }
        int idx = (int)val_tok.as.number;
        if (idx < 0 || idx >= SEC_COUNT) {
            err.code = 2;
            err.message = "Invalid security level index";
            return err;
        }
        if ((BppSecLevel)idx < cur_level) {
            vdev_printf(vdev, "Cannot lower security from %s to %s.\n",
                        security_level_name(cur_level),
                        security_level_name((BppSecLevel)idx));
            return err;
        }
        security_set_level((BppSecLevel)idx);
        vdev_printf(vdev, "Security Level raised to: %s\n", security_level_name((BppSecLevel)idx));
        return err;
    }

    /* 4. SECURITY n (numeric form) */
    if (tok.type == TOK_NUMBER) {
        lex_next(lex);
        int idx = (int)tok.as.number;
        if (idx < 0 || idx >= SEC_COUNT) {
            err.code = 2;
            err.message = "Invalid security level index";
            return err;
        }
        if ((BppSecLevel)idx < cur_level) {
            vdev_printf(vdev, "Cannot lower security from %s to %s.\n",
                        security_level_name(cur_level),
                        security_level_name((BppSecLevel)idx));
            return err;
        }
        security_set_level((BppSecLevel)idx);
        vdev_printf(vdev, "Security Level raised to: %s\n", security_level_name((BppSecLevel)idx));
        return err;
    }

    err.code = 2;
    err.message = "Invalid SECURITY parameter syntax";
    return err;
}

#endif /* BASIC_LITE_BUILD */

