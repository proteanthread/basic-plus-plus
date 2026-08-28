// FILENAME: system.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the SYSTEM statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/system.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/metadata.h"
#include "runtime/task.h"
#include "runtime/file.h"
#include "security/security.h"
#include "module/module.h"
#include "platform/platform.h"
#include "memory/memory.h"
#include "types/version.h"
#include "runtime/micro_lib_metadata.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

extern void platform_execute_shell(void);
BppError stmt_bye_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_BACKSLASH) {
        err.code = 2;
        err.message = "Unexpected argument after BYE";
        return err;
    }
    task_mgr_shutdown();
    FileContext *fc = vm_get_file(vm);
    if (fc) {
        file_close_all(fc);
    }
    vm_halt(vm);
    vm_request_exit(vm);
    return err;
}

BppError stmt_system_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOF || tok.type == TOK_EOL || tok.type == TOK_BACKSLASH) {
        vm_halt(vm);
        vm_request_exit(vm);
        return err;
    }

    if (tok.type == TOK_NUMBER) {
        BValue exit_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        (void)exit_val;
        vm_halt(vm);
        vm_request_exit(vm);
        return err;
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (!vdev) return err;

    if (tok.type == TOK_LPAREN) {
        lex_next(lex); // Consume '('
        BValue code_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (code_val.type != VAL_NUMBER && code_val.type != VAL_INTEGER) {
            if (code_val.type == VAL_STRING && code_val.as.string) {
                str_release(vm_get_str(vm), code_val.as.string);
            }
            err.code = 13; err.message = "Type mismatch (expected numeric code for SYSTEM)";
            return err;
        }

        BppToken rtok = lex_next(lex);
        if (rtok.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after SYSTEM code";
            return err;
        }

        int code = (int)code_val.as.number;
        switch (code) {
            case 0:
#if defined(_WIN32)
                vdev_printf(vdev, "Process ID: %lu\n", (unsigned long)GetCurrentProcessId());
#else
                vdev_printf(vdev, "Process ID: %ld\n", (long)getpid());
#endif
                break;
            case 1:
                vdev_printf(vdev, "Platform: %s (%s)\n", platform_name(), BASIC_PROFILE_NAME);
#if defined(_MSC_VER)
                vdev_printf(vdev, "Compiler: MSVC %d\n", _MSC_VER);
#elif defined(__GNUC__) && !defined(__clang__)
                vdev_printf(vdev, "Compiler: GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__clang__)
                vdev_printf(vdev, "Compiler: Clang %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#else
                vdev_puts(vdev, "Compiler: Unknown C17 Compiler\n");
#endif
                vdev_printf(vdev, "Word size: %d-bit (ptr=%d int=%d long=%d)\n",
                            (int)(sizeof(void*) * 8),
                            (int)sizeof(void*),
                            (int)sizeof(int),
                            (int)sizeof(long));
                vdev_printf(vdev, "%s v%s \"%s\"\n", BASIC_NAME, BASIC_VERSION_STRING, BASIC_VERSION_CODENAME);
                vdev_printf(vdev, "Security: %s\n", security_level_name(security_get_level()));
                vdev_printf(vdev, "Modules: %d registered\n", module_count());
                break;
            case 2: {
                size_t free_ram = mem_get_free_ram(vm_get_mem(vm));
                size_t total_ram = free_ram + mem_get_used_ram(vm_get_mem(vm));
                char free_buf[64], total_buf[64];
                mem_format_size(free_ram, free_buf, sizeof(free_buf));
                mem_format_size(total_ram, total_buf, sizeof(total_buf));
                vdev_printf(vdev, "Memory: Free RAM %s / %s Total RAM\n", free_buf, total_buf);
                break;
            }
            case 3:
                vdev_printf(vdev, "Security Level: %s (%d)\n",
                            security_level_name(security_get_level()),
                            (int)security_get_level());
                break;
            case 4:
                vdev_printf(vdev, "Modules: %d registered\n", module_count());
                break;
            case 5:
                vdev_printf(vdev, "%s v%s \"%s\" (Built %s %s)\n",
                            BASIC_NAME, BASIC_VERSION_STRING, BASIC_VERSION_CODENAME,
                            __DATE__, __TIME__);
                break;
            default:
                vdev_printf(vdev, "SYSTEM(%d): Valid codes are 0=PID, 1=Platform, 2=Memory, 3=Security, 4=Modules, 5=Version\n", code);
                break;
        }
        return err;
    }

    if (tok.type == TOK_STRING || tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        lex_next(lex);
        char query[256];
        const char *src = (tok.type == TOK_STRING) ? tok.as.string : tok.start;
        int len = (int)(tok.length < sizeof(query) - 1 ? tok.length : sizeof(query) - 1);
        memcpy(query, src, len);
        query[len] = '\0';

        for (int i = 0; query[i]; i++) {
            if (query[i] >= 'a' && query[i] <= 'z') query[i] -= 32;
        }

        if (strcmp(query, "PLATFORM") == 0) {
            vdev_printf(vdev, "%s (%s)\n", platform_name(), BASIC_PROFILE_NAME);
        } else if (strcmp(query, "VERSION") == 0) {
            vdev_printf(vdev, "%s v%s \"%s\"\n", BASIC_NAME, BASIC_VERSION_STRING, BASIC_VERSION_CODENAME);
        } else if (strcmp(query, "MEMORY") == 0) {
            size_t free_ram = mem_get_free_ram(vm_get_mem(vm));
            size_t total_ram = free_ram + mem_get_used_ram(vm_get_mem(vm));
            char free_buf[64], total_buf[64];
            mem_format_size(free_ram, free_buf, sizeof(free_buf));
            mem_format_size(total_ram, total_buf, sizeof(total_buf));
            vdev_printf(vdev, "Free RAM: %s / %s total\n", free_buf, total_buf);
        } else if (strcmp(query, "COMPILER") == 0) {
#if defined(_MSC_VER)
            vdev_printf(vdev, "MSVC %d\n", _MSC_VER);
#elif defined(__GNUC__) && !defined(__clang__)
            vdev_printf(vdev, "GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__clang__)
            vdev_printf(vdev, "Clang %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#else
            vdev_puts(vdev, "Unknown C17 Compiler\n");
#endif
        } else if (strcmp(query, "WORDSIZE") == 0) {
            vdev_printf(vdev, "%d-bit\n", (int)(sizeof(void*) * 8));
        } else {
            vdev_printf(vdev, "Unknown query '%s'. Use PLATFORM, VERSION, MEMORY, COMPILER, or WORDSIZE.\n", query);
        }
        return err;
    }

    err.code = 2;
    err.message = "Syntax error in SYSTEM (expected no args, (code), or query string)";
    return err;
}


BppError stmt_devices_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_nwrite_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_statesave_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_stateload_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

void stmt_system_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SYSTEM",
        .category = "System & Environ",
        .syntax = "SYSTEM | BYE | SHELL [command_string$]",
        .help_text = "Exits BASIC++ interpreter session or executes an operating system command.",
        .error_codes = "Error 2: Syntax Error, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}

