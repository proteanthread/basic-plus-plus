/**
 * @file stmt_module.c
 * @brief MODULE statement parser.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Parses the MODULE statement, supporting queries, activation, dynamic load, and unload commands.
 * - Why it exists: Exposes the module manager APIs to BASIC++ program space.
 * - Why it works this way: It verifies required security level gates (SECOP_MODULE / SECOP_SYSTEM) before activating
 *   or loading libraries.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Module list text formatting and diagnostic outputs.
 * - What cannot be changed: Security capability checking hooks.
 * - What to expect: Blocked modules print a security error.
 * - What to do if something breaks: Trace token dispatch blocks and check registry counts.
 */

#include "bpp_stmt.h"
#include "bpp_lexer.h"
#include "bpp_module.h"
#include "bpp_security.h"
#include "bpp_vdev.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>



BppError stmt_module_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    VDevContext *vdev = vm_get_vdev(vm);

    BppToken tok = lex_peek(lex);

    /* 1. MODULE (no args) -> MODULE LIST */
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        int mc = module_count();
        if (mc == 0) {
            vdev_printf(vdev, "No modules registered.\n");
        } else {
            vdev_printf(vdev, "%-16s %-10s %-8s %-6s %s\n", "Module", "Class", "Version", "Caps", "Status");
            vdev_printf(vdev, "%-16s %-10s %-8s %-6s %s\n", "------", "-----", "-------", "----", "------");
            for (int i = 0; i < mc; ++i) {
                const BppModuleInfo *m = module_get(i);
                if (!m) continue;
                char caps[12];
                module_caps_string(m->capabilities, caps, sizeof(caps));
                vdev_printf(vdev, "%-16s %-10s %-8s %-6s %s\n",
                            m->name,
                            module_class_name(m->mod_class),
                            m->version,
                            caps,
                            module_is_loaded(i) ? "ACTIVE" : "INACTIVE");
            }
        }
        return err;
    }

    /* 2. Check for subcommands: LOAD, UNLOAD, INFO, LIST */
    if (tok.type == TOK_KEYWORD) {
        /* Subcommand: LOAD */
        if (tok.as.keyword == KW_LOAD) {
            lex_next(lex); /* Consume LOAD */
            BppToken path_tok = lex_next(lex);
            if (path_tok.type != TOK_STRING) {
                err.code = 2; /* Syntax error */
                err.message = "Expected string path after LOAD";
                return err;
            }

            /* Security gates */
            if (security_check(SECOP_MODULE, 0) != 0) return err;
            if (security_check(SECOP_SYSTEM, 0) != 0) return err;

            char path[256];
            size_t len = (path_tok.length < sizeof(path) - 1) ? path_tok.length : sizeof(path) - 1;
            memcpy(path, path_tok.as.string, len);
            path[len] = '\0';

            /* Validate module path (relative and with whitelisted extension) */
            if (security_check_path(path, 0) != 0) {
                err.code = 70; err.message = "Permission denied";
                return err;
            }

            if (module_load_dynamic(vm, path) == 0) {
                vdev_printf(vdev, "Module loaded successfully: %s\n", path);
            }
            return err;
        }

        /* Subcommand: UNLOAD */
        if (tok.as.keyword == KW_UNLOAD) {
            lex_next(lex); /* Consume UNLOAD */
            BppToken name_tok = lex_next(lex);
            if (name_tok.type != TOK_STRING) {
                err.code = 2;
                err.message = "Expected module name string after UNLOAD";
                return err;
            }

            char mname[128];
            size_t len = (name_tok.length < sizeof(mname) - 1) ? name_tok.length : sizeof(mname) - 1;
            memcpy(mname, name_tok.as.string, len);
            mname[len] = '\0';

            if (module_deactivate(mname) == 0) {
                vdev_printf(vdev, "Module unloaded: %s\n", mname);
            } else {
                vdev_printf(vdev, "Module '%s' not found or not active.\n", mname);
            }
            return err;
        }

        /* Subcommand: INFO */
        if (tok.as.keyword == KW_INFO) {
            lex_next(lex); /* Consume INFO */
            BppToken name_tok = lex_next(lex);
            if (name_tok.type != TOK_STRING) {
                err.code = 2;
                err.message = "Expected module name string after INFO";
                return err;
            }

            char mname[128];
            size_t len = (name_tok.length < sizeof(mname) - 1) ? name_tok.length : sizeof(mname) - 1;
            memcpy(mname, name_tok.as.string, len);
            mname[len] = '\0';

            const BppModuleInfo *m = module_find(mname);
            if (!m) {
                vdev_printf(vdev, "Module '%s' not found.\n", mname);
                return err;
            }

            char caps[16];
            module_caps_string(m->capabilities, caps, sizeof(caps));
            vdev_printf(vdev, "Module:       %s\n", m->name);
            vdev_printf(vdev, "Version:      %s\n", m->version);
            vdev_printf(vdev, "Description:  %s\n", m->description);
            vdev_printf(vdev, "Class:        %s\n", module_class_name(m->mod_class));
            vdev_printf(vdev, "Capabilities: %s (0x%04X)\n", caps, m->capabilities);
            vdev_printf(vdev, "Status:       %s\n", module_is_active(mname) ? "ACTIVE" : "INACTIVE");
            vdev_printf(vdev, "Security:     %s at current level %s\n",
                        security_module_allowed(m->capabilities) ? "ALLOWED" : "BLOCKED",
                        security_level_name(security_get_level()));
            vdev_printf(vdev, "Required Lvl: %s (Pin Check: %s)\n",
                        security_level_name(m->required_level),
                        security_check_pinned_level(m->required_level) ? "PASS" : "FAIL");
            return err;
        }

        /* Subcommand: LIST */
        if (tok.as.keyword == KW_LIST) {
            lex_next(lex); /* Consume LIST */
            /* fall through to listing below */
            return stmt_module_handler(vm, lex);
        }
    }

    /* 3. Treat string as module name to activate: MODULE "name" */
    if (tok.type == TOK_STRING) {
        lex_next(lex);
        char mname[128];
        size_t len = (tok.length < sizeof(mname) - 1) ? tok.length : sizeof(mname) - 1;
        memcpy(mname, tok.as.string, len);
        mname[len] = '\0';

        if (security_check(SECOP_MODULE, 0) != 0) return err;

        if (module_activate(mname, vm) == 0) {
            vdev_printf(vdev, "Module '%s' activated.\n", mname);
        }
        return err;
    }

    err.code = 2;
    err.message = "Invalid MODULE parameter syntax";
    return err;
}
