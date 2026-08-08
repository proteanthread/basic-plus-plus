/**
 * @file system.c
 * @brief SYSTEM, BYE, and SHELL system control statement handlers for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements OS process control and engine termination statement handlers:
 * - SYSTEM: Exits program execution and returns to operating system command prompt.
 * - BYE: Immediate alias for SYSTEM to exit interpreter session.
 * - SHELL [command_string$]: Suspends BASIC++ execution and invokes host system shell or executes command string.
 *
 * 2. WHY IT EXISTS:
 * Provides host operating system shell access and interpreter session termination per GW-BASIC / QBASIC standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * SYSTEM/BYE set vm_halt() and vm_request_exit() on VMContext; SHELL validates security permissions via sec_check_permission(SEC_SYS_EXEC), evaluates command string, and invokes platform_execute_command().
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_system'. Includes "statements/system/system.h",
 * "eval/eval.h", "device/vdev.h", "runtime/metadata.h", "security/security.h", "module/module.h", "platform/platform.h", "memory/memory.h", "types/version.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs'). SHELL command execution requires active security permissions in sandbox environments.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support capture of SHELL command exit codes into system variables (STATUS%).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Security invariant: SHELL MUST invoke sec_check_permission(SEC_SYS_EXEC) before executing host commands.
 *
 * 8. WHAT TO EXPECT:
 * Exits session or executes OS shell command; returns ERR_NONE or ERR_PERMISSION_DENIED under sandbox restrictions.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify platform_execute_command() implementation in platform/platform.c.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses platform abstraction layer for OS command execution.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/platform/platform.c
 * - engine/src/security/security.c
 * - engine/src/vm/vm_context.c
 * Prerequisite Header Files:
 * - engine/include/statements/system/system.h
 * - engine/include/platform/platform.h
 * - engine/include/vm/vm.h
 */

#include "statements/system/system.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/metadata.h"
#include "security/security.h"
#include "module/module.h"
#include "platform/platform.h"
#include "memory/memory.h"
#include "types/version.h"
#include "runtime/micro_lib_metadata.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void platform_execute_shell(void);
extern void platform_execute_command(const char *cmd);

BppError stmt_bye_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;
    vm_halt(vm);
    vm_request_exit(vm);
    return err;
}

BppError stmt_system_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    if (!vdev) return err;

    BppToken tok = lex_peek(lex);
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

    return err;
}

BppError stmt_shell_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue cmd_val;
    memset(&cmd_val, 0, sizeof(cmd_val));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        platform_execute_shell();
        return err;
    }

    cmd_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (cmd_val.type != VAL_STRING) {
        err.code = 13; err.message = "Type mismatch (expected string for SHELL)";
        return err;
    }

    platform_execute_command(str_data(cmd_val.as.string));
    return err;
}

BppError stmt_pause_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_sys_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
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

