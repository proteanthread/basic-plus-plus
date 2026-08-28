// FILENAME: module.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, help.c, system.c)
// NEEDS: libcore (ctype.h, ctype.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (module.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, scope.h, scope.c, stmt.h, string.c)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the MODULE statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "lexer/lexer.h"
#include "module/module.h"
#include "scope/scope.h"
#include "security/security.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#if defined(_MSC_VER)
#define strcasecmp _stricmp
#endif

BppError stmt_module_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    VDevContext *vdev = vm_get_vdev(vm);

    BppToken tok = lex_peek(lex);

    // 1. MODULE (no args) -> MODULE LIST
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

    // Check for END / KW_END subcommand -> exit module scope
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
        lex_next(lex);
        scope_namespace_exit(vm);
        vdev_printf(vdev, "Exited MODULE scope namespace.\n");
        return err;
    }

    // 2. Check for subcommands: LOAD, UNLOAD, INFO, LIST, IMPORT
    if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
        char word[64];
        size_t len = tok.length < 63 ? tok.length : 63;
        memcpy(word, tok.start, len);
        word[len] = '\0';

        if (strcasecmp(word, "END") == 0) {
            lex_next(lex);
            scope_namespace_exit(vm);
            vdev_printf(vdev, "Exited MODULE scope namespace.\n");
            return err;
        }

        if (strcasecmp(word, "IMPORT") == 0) {
            lex_next(lex);
            BppToken path_tok = lex_next(lex);
            if (path_tok.type != TOK_STRING && path_tok.type != TOK_IDENT) {
                err.code = 2;
                err.message = "Expected string or identifier after MODULE IMPORT";
                return err;
            }
            char path[256];
            size_t plen = path_tok.length < 255 ? path_tok.length : 255;
            memcpy(path, path_tok.start, plen);
            path[plen] = '\0';

            if (module_activate(path, vm) == 0) {
                vdev_printf(vdev, "Activated module '%s'.\n", path);
            } else {
                BppError load_err = vm_load_library_file(vm, path);
                if (load_err.code != 0) return load_err;
                vdev_printf(vdev, "Loaded module file '%s'.\n", path);
            }
            return err;
        }

        // Subcommand: LOAD
        if (tok.as.keyword == KW_LOAD || strcasecmp(word, "LOAD") == 0) {
            lex_next(lex);
            BppToken path_tok = lex_next(lex);
            if (path_tok.type != TOK_STRING && path_tok.type != TOK_IDENT) {
                err.code = 2;
                err.message = "Expected string path after LOAD";
                return err;
            }

            if (security_check(SECOP_MODULE, 0) != 0) return err;
            if (security_check(SECOP_SYSTEM, 0) != 0) return err;

            char path[256];
            size_t plen = path_tok.length < 255 ? path_tok.length : 255;
            memcpy(path, path_tok.start, plen);
            path[plen] = '\0';

            if (security_check_path(path, 0) != 0) {
                err.code = 70; err.message = "Permission denied";
                return err;
            }

            if (module_load_dynamic(vm, path) == 0) {
                vdev_printf(vdev, "Module loaded successfully: %s\n", path);
            }
            return err;
        }

        // Subcommand: UNLOAD
        if (tok.as.keyword == KW_UNLOAD || strcasecmp(word, "UNLOAD") == 0) {
            lex_next(lex);
            BppToken name_tok = lex_next(lex);
            if (name_tok.type != TOK_STRING && name_tok.type != TOK_IDENT) {
                err.code = 2;
                err.message = "Expected module name after UNLOAD";
                return err;
            }

            char mname[128];
            size_t mlen = name_tok.length < 127 ? name_tok.length : 127;
            memcpy(mname, name_tok.start, mlen);
            mname[mlen] = '\0';

            if (module_deactivate(mname) == 0) {
                vdev_printf(vdev, "Module unloaded: %s\n", mname);
            } else {
                vdev_printf(vdev, "Module '%s' not found or not active.\n", mname);
            }
            return err;
        }

        // Subcommand: INFO
        if (tok.as.keyword == KW_INFO || strcasecmp(word, "INFO") == 0) {
            lex_next(lex);
            BppToken name_tok = lex_next(lex);
            if (name_tok.type != TOK_STRING && name_tok.type != TOK_IDENT) {
                err.code = 2;
                err.message = "Expected module name after INFO";
                return err;
            }

            char mname[128];
            size_t mlen = name_tok.length < 127 ? name_tok.length : 127;
            memcpy(mname, name_tok.start, mlen);
            mname[mlen] = '\0';

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

        // Subcommand: LIST
        if (tok.as.keyword == KW_LIST || strcasecmp(word, "LIST") == 0) {
            lex_next(lex);
            return stmt_module_handler(vm, lex);
        }

        // Generic Identifier / Keyword -> Enter MODULE scope namespace or skip module block during program execution
        BppToken mod_name_tok = lex_next(lex);
        char mod_name[64];
        size_t nlen = mod_name_tok.length < 63 ? mod_name_tok.length : 63;
        memcpy(mod_name, mod_name_tok.start, nlen);
        mod_name[nlen] = '\0';

        if (vm_is_running(vm)) {
            MemoryContext *mem = vm_get_mem(vm);
            size_t count = 0;
            BppProgramLine *lines = mem_program_get_all(mem, &count);
            BppLineNumber cur_ln = vm_get_current_line(vm);
            size_t start_idx = 0;
            for (size_t i = 0; i < count; ++i) {
                if (lines[i].line_number == cur_ln) { start_idx = i; break; }
            }
            int nest = 0;
            for (size_t i = start_idx + 1; i < count; ++i) {
                LexerContext *scan = lex_init(mem, lines[i].text);
                if (!scan) continue;
                BppToken scan_tok = lex_next(scan);
                if (scan_tok.type == TOK_NUMBER) scan_tok = lex_next(scan);
                if (scan_tok.type == TOK_KEYWORD) {
                    if (scan_tok.as.keyword == KW_MODULE) nest++;
                    else if (scan_tok.as.keyword == KW_END) {
                        BppToken ntok = lex_next(scan);
                        if (ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_MODULE) {
                            if (nest > 0) nest--;
                            else {
                                vm_jump(vm, lines[i].line_number, lines[i].text);
                                lex_shutdown(scan);
                                return err;
                            }
                        }
                    }
                }
                lex_shutdown(scan);
            }
        }

        scope_namespace_enter(vm, mod_name);
        return err;
    }

    // 3. Treat string as module name to activate or enter: MODULE "name"
    if (tok.type == TOK_STRING) {
        lex_next(lex);
        char mname[128];
        size_t mlen = tok.length < 127 ? tok.length : 127;
        memcpy(mname, tok.start, mlen);
        mname[mlen] = '\0';

        scope_namespace_enter(vm, mname);
        vdev_printf(vdev, "Entered MODULE scope namespace '%s'.\n", mname);
        return err;
    }

    err.code = 2;
    err.message = "Invalid MODULE parameter syntax";
    return err;
}

BppError stmt_import_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    BppToken tok = lex_next(lex);
    if (tok.type == TOK_STRING) {
        char path[256] = {0};
        if (tok.as.string) {
            size_t len = (tok.length < sizeof(path) - 1) ? tok.length : sizeof(path) - 1;
            memcpy(path, tok.as.string, len);
        } else if (tok.start) {
            const char *src = tok.start;
            size_t len = tok.length;
            if (len >= 2 && src[0] == '"' && src[len - 1] == '"') {
                src++;
                len -= 2;
            }
            if (len >= sizeof(path)) len = sizeof(path) - 1;
            memcpy(path, src, len);
        }
        err = vm_load_library_file(vm, path);
        return err;
    } else if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char mod_name[128] = {0};
        size_t len = (tok.length < sizeof(mod_name) - 1) ? tok.length : sizeof(mod_name) - 1;
        memcpy(mod_name, tok.start, len);
        scope_namespace_enter(vm, mod_name);
        module_activate(mod_name, vm);
        return err;
    }
    err.code = 2; err.message = "Expected module name or file path after IMPORT";
    return err;
}

void stmt_module_register(void) {
    static const MicroLibMetadata meta = {
        .name = "MODULE",
        .category = "Introspection",
        .syntax = "MODULE name | MODULE LOAD path | MODULE UNLOAD name | MODULE INFO name",
        .help_text = "Defines a module scope namespace, or loads/unloads dynamic extension modules.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied"
    };
    microlib_register(&meta);

    static const MicroLibMetadata import_meta = {
        .name = "IMPORT",
        .category = "Modular Execution",
        .syntax = "IMPORT module_name | IMPORT \"file.bas\"",
        .help_text = "Imports a module scope namespace or external BASIC++ source module.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found"
    };
    microlib_register(&import_meta);
}

