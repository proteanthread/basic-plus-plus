// FILENAME: func_record_lock.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch_call.c)
// NEEDS: libcore (language_descriptor.h, strings.h), libengine (func_record_lock.h)
// Provides runtime function implementation for READU, WRITEU, RELEASE, and LOCKED.
//
// ---- Includes ----

#include "eval/functions/filesystem/func_record_lock.h"
#include "statements/filesystem/file_ops/record_lock.h"
#include "statements/db/isam/isam.h"
#include "runtime/language_descriptor.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_fn_readu_desc = {
    .name = "READU",
    .category = "Pick & Business BASIC",
    .syntax = "status = READU(ch, id, var$) | rec$ = READU$(ch, id)",
    .description = "Function that locks and reads a record.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_fn_writeu_desc = {
    .name = "WRITEU",
    .category = "Pick & Business BASIC",
    .syntax = "status = WRITEU(ch, id, data$)",
    .description = "Function that writes a record and retains lock.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_fn_release_desc = {
    .name = "RELEASE",
    .category = "Pick & Business BASIC",
    .syntax = "status = RELEASE(ch, id) | RELEASE(ch) | RELEASE()",
    .description = "Function that releases record lock(s).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_fn_locked_desc = {
    .name = "LOCKED",
    .category = "Pick & Business BASIC",
    .syntax = "is_locked = LOCKED(ch, id) | LOCKED[ch, id]",
    .description = "Returns -1 if record is locked, 0 if unlocked.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_record_lock_register(void) {
    lang_desc_register(&g_fn_readu_desc);
    lang_desc_register(&g_fn_writeu_desc);
    lang_desc_register(&g_fn_release_desc);
    lang_desc_register(&g_fn_locked_desc);
}

BValue func_readu_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)err;
    bool is_str = (uname && (runtime_strcmp(uname, "READU$") == 0 || runtime_strcasecmp(uname, "READU$") == 0));
    BValue res;
    res.type = is_str ? VAL_STRING : VAL_NUMBER;
    if (is_str) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    } else {
        res.as.number = 0.0;
    }

    if (arg_count < 2 || !args) return res;

    int ch = (int)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0);
    char id_str[64] = {0};
    if (args[1].type == VAL_STRING && args[1].as.string) {
        const char *s = str_data(args[1].as.string);
        if (s) runtime_strncpy(id_str, s, sizeof(id_str) - 1);
    } else {
        runtime_snprintf(id_str, sizeof(id_str), "%g", args[1].as.number);
    }

    // Acquire lock
    record_lock_acquire(ch, id_str);

    BValue rec_val;
    rec_val.type = VAL_NONE;
    rec_val.as.number = 0.0;

    if (isam_is_active(ch)) {
        if (!isam_read_record(vm, ch, id_str, &rec_val)) {
            rec_val.type = VAL_STRING;
            rec_val.as.string = str_create(vm_get_str(vm), "", 0);
        }
    } else {
        const char *ds = record_lock_retrieve(ch, id_str);
        rec_val.type = VAL_STRING;
        rec_val.as.string = str_create(vm_get_str(vm), ds, runtime_strlen(ds));
    }

    if (is_str) {
        str_release(vm_get_str(vm), res.as.string);
        return rec_val;
    } else {
        res.as.number = 1.0;
        if (rec_val.type == VAL_STRING && rec_val.as.string) {
            str_release(vm_get_str(vm), rec_val.as.string);
        }
        return res;
    }
}

BValue func_writeu_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 3 || !args) return res;

    int ch = (int)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0);
    char id_str[64] = {0};
    if (args[1].type == VAL_STRING && args[1].as.string) {
        const char *s = str_data(args[1].as.string);
        if (s) runtime_strncpy(id_str, s, sizeof(id_str) - 1);
    } else {
        runtime_snprintf(id_str, sizeof(id_str), "%g", args[1].as.number);
    }

    record_lock_acquire(ch, id_str);

    if (isam_is_active(ch)) {
        if (isam_write_record(vm, ch, id_str, args[2])) {
            res.as.number = 1.0;
        }
    } else {
        if (args[2].type == VAL_STRING && args[2].as.string) {
            const char *ds = str_data(args[2].as.string);
            record_lock_store(ch, id_str, ds ? ds : "");
        } else {
            char num_buf[64] = {0};
            runtime_snprintf(num_buf, sizeof(num_buf), "%g", args[2].as.number);
            record_lock_store(ch, id_str, num_buf);
        }
        res.as.number = 1.0;
    }
    return res;
}

BValue func_release_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count == 0) {
        res.as.number = (double)record_lock_release_all();
        return res;
    }

    int ch = (int)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0);
    if (arg_count == 1) {
        res.as.number = (double)record_lock_release_channel(ch);
        return res;
    }

    char id_str[64] = {0};
    if (args[1].type == VAL_STRING && args[1].as.string) {
        const char *s = str_data(args[1].as.string);
        if (s) runtime_strncpy(id_str, s, sizeof(id_str) - 1);
    } else {
        runtime_snprintf(id_str, sizeof(id_str), "%g", args[1].as.number);
    }

    res.as.number = record_lock_release(ch, id_str) ? 1.0 : 0.0;
    return res;
}

BValue func_locked_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 2 || !args) return res;

    int ch = (int)((args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) ? args[0].as.number : 0);
    char id_str[64] = {0};
    if (args[1].type == VAL_STRING && args[1].as.string) {
        const char *s = str_data(args[1].as.string);
        if (s) runtime_strncpy(id_str, s, sizeof(id_str) - 1);
    } else {
        runtime_snprintf(id_str, sizeof(id_str), "%g", args[1].as.number);
    }

    // Vintage relational truth value invariant: -1 for true, 0 for false
    res.as.number = record_lock_is_locked(ch, id_str) ? -1.0 : 0.0;
    return res;
}
