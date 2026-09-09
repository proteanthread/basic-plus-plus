// FILENAME: func_input_str.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for INPUT$ function in BASIC++.

#include "eval/functions/io/func_input_str.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/string/memops.h"
#include "runtime/file.h"
#include "hal/hal.h"
#include "hal/io_hal.h"
#include "memory/memory.h"
#include "runtime/memory/alloc.h"

static const LangDesc g_input_str_desc = {
    .name = "INPUT$",
    .category = "File & Console I/O",
    .syntax = "INPUT$(n% [, [#]channel%])",
    .description = "Reads a fixed number of bytes from console input or open file channel.",
    .error_summary = "Error 5: Illegal Function Call, Error 52: Bad File Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_input_str_register(void) {
    lang_desc_register(&g_input_str_desc);
}

BValue func_input_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_STRING, .as.string = NULL };
    if (!vm || !err) return res;

    if (arg_count < 1 || arg_count > 2) {
        err->code = 5;
        err->message = "Illegal Function Call (Expected 1 or 2 arguments)";
        return res;
    }

    int n = 0;
    if (args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) {
        n = (int)args[0].as.number;
    } else {
        err->code = 13;
        err->message = "Type Mismatch";
        return res;
    }

    if (n <= 0) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }

    // Limit single read chunk to 64KB for safety
    if (n > 65536) n = 65536;

    char *buf = (char *)runtime_malloc((size_t)n + 1);
    if (!buf) {
        err->code = 7;
        err->message = "Out of Memory";
        return res;
    }
    runtime_memset(buf, 0, (size_t)n + 1);

    if (arg_count == 1) {
        // Read n characters from console input
        int bytes_read = 0;
        const HalContext *hal = hal_get();
        while (bytes_read < n) {
            int ch = -1;
            if (hal && hal->io.console_getchar) {
                ch = hal->io.console_getchar();
            }
            if (ch < 0) {
                // No more immediate characters available
                break;
            }
            buf[bytes_read++] = (char)ch;
        }
        res.as.string = str_create(vm_get_str(vm), buf, (size_t)bytes_read);
    } else {
        // Read n characters from specified file channel
        int channel = 0;
        if (args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) {
            channel = (int)args[1].as.number;
        } else {
            runtime_free(buf);
            err->code = 13;
            err->message = "Type Mismatch";
            return res;
        }

        FileContext *fc = vm_get_file(vm);
        if (!fc || !file_is_open(fc, channel)) {
            runtime_free(buf);
            err->code = 52;
            err->message = "Bad File Number";
            return res;
        }

        IoHandle h = file_get_handle(fc, channel);
        const HalContext *hal = hal_get();
        size_t bytes_read = 0;
        if (hal && hal->io.file_read) {
            bytes_read = hal->io.file_read(h, buf, 1, (size_t)n);
        }
        res.as.string = str_create(vm_get_str(vm), buf, bytes_read);
    }

    runtime_free(buf);
    return res;
}
