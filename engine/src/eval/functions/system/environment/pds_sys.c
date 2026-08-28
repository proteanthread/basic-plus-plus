// FILENAME: pds_sys.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (pds_sys.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the PDS_SYS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/environment/pds_sys.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include "memory/memory.h"
#include "runtime/string.h"
#include "runtime/memory.h"

static BppDirSearch *g_dir_search = NULL;

BValue func_dir_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    const char *spec = NULL;
    if (arg_count >= 1 && args[0].type == VAL_STRING && args[0].as.string) {
        spec = str_data(args[0].as.string);
    }

    char name_buf[260];
    name_buf[0] = '\0';

    if (spec && spec[0] != '\0') {
        if (g_dir_search) {
            platform_find_close(g_dir_search);
            g_dir_search = NULL;
        }
        g_dir_search = platform_find_first_file(spec, name_buf, sizeof(name_buf));
        if (!g_dir_search) {
            res.as.string = str_create(vm_get_str(vm), "", 0);
            return res;
        }
        res.as.string = str_create(vm_get_str(vm), name_buf, runtime_strlen(name_buf));
        return res;
    } else {
        if (!g_dir_search) {
            res.as.string = str_create(vm_get_str(vm), "", 0);
            return res;
        }
        if (platform_find_next_file(g_dir_search, name_buf, sizeof(name_buf))) {
            res.as.string = str_create(vm_get_str(vm), name_buf, runtime_strlen(name_buf));
            return res;
        } else {
            platform_find_close(g_dir_search);
            g_dir_search = NULL;
            res.as.string = str_create(vm_get_str(vm), "", 0);
            return res;
        }
    }
}

BValue func_curdir_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)arg_count; (void)args; (void)err;
    BValue res;
    res.type = VAL_STRING;
    char buf[1024];
    runtime_memset(buf, 0, sizeof(buf));
    if (platform_getcwd(buf, sizeof(buf) - 1)) {
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    } else {
        res.as.string = str_create(vm_get_str(vm), "C:\\", 3);
    }
    return res;
}

BValue func_setmem_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)arg_count; (void)args; (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = (double)mem_get_free_ram(vm_get_mem(vm));
    return res;
}

BValue func_sseg_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)arg_count; (void)args; (void)vm; (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0x1000; // Standard DOS data segment
    return res;
}

void func_pds_sys_register(void) {
    static const MicroLibMetadata meta_dir = {
        .name = "DIR$",
        .category = "File Functions",
        .syntax = "DIR$([filespec$])",
        .help_text = "Returns the next matching filename matching the filespec filter.",
        .error_codes = "None"
    };
    static const MicroLibMetadata meta_curdir = {
        .name = "CURDIR$",
        .category = "File Functions",
        .syntax = "CURDIR$([drive$])",
        .help_text = "Returns the current working directory path.",
        .error_codes = "None"
    };
    static const MicroLibMetadata meta_setmem = {
        .name = "SETMEM",
        .category = "System Functions",
        .syntax = "SETMEM(bytes&)",
        .help_text = "Adjusts the memory pool reservation and returns available RAM in bytes.",
        .error_codes = "None"
    };
    static const MicroLibMetadata meta_sseg = {
        .name = "SSEG",
        .category = "System Functions",
        .syntax = "SSEG(var)",
        .help_text = "Returns the segment address of the string descriptor.",
        .error_codes = "None"
    };
    microlib_register(&meta_dir);
    microlib_register(&meta_curdir);
    microlib_register(&meta_setmem);
    microlib_register(&meta_sseg);
}

