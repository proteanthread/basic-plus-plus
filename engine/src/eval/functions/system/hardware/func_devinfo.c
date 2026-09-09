// FILENAME: func_devinfo.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, sys_fn.c
// NEEDS: libkernel (types.h, errors.h, vdev.h), libcore (strings.h, language_descriptor.h)
// Implementation for DEVINFO$ and DEVCAPS introspection functions.
//
// ---- Includes ----

#include "eval/functions/func_devinfo.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"

static const LangDesc g_devinfo_desc = {
    .name = "DEVINFO$",
    .category = "System & Hardware",
    .syntax = "DEVINFO$(dev_name$, property$)",
    .description = "Returns property metadata (CLASS, DRIVER, VERSION, TARGET, STATUS) for a registered virtual device.",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_devcaps_desc = {
    .name = "DEVCAPS",
    .category = "System & Hardware",
    .syntax = "DEVCAPS(dev_name$)",
    .description = "Returns integer capability bitmask for a registered virtual device.",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_FUNCTION
};

void func_devinfo_register(void) {
    lang_desc_register(&g_devinfo_desc);

    lang_desc_register(&g_devcaps_desc);
}

BValue func_devinfo_eval(VMContext *vm, const char *name, int arg_count, BValue *args, BppError *err) {
    (void)name;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (!vm || !err || arg_count < 1 || !args) {
        if (err) {
            err->code = ERR_ILLEGAL_FUNCTION_CALL;
            err->message = "DEVINFO$ requires at least 1 argument";
        }
        return res;
    }

    if (args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *dev_name = str_data(args[0].as.string);
    const char *prop = (arg_count >= 2 && args[1].type == VAL_STRING && args[1].as.string) ?
                       str_data(args[1].as.string) : "CLASS";

    VDevContext *ctx = vm_get_vdev(vm);
    if (!ctx) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }

    const char *out_str = "";

    if (runtime_strcasecmp(prop, "TARGET") == 0) {
        out_str = vdev_alias_resolve(ctx, dev_name);
    } else {
        VDev *dev = vdev_get(ctx, dev_name);
        if (dev) {
            if (dev->dev_info) {
                const char *custom = dev->dev_info(dev, prop);
                if (custom) {
                    out_str = custom;
                    res.as.string = str_create(vm_get_str(vm), out_str, runtime_strlen(out_str));
                    return res;
                }
            }
            if (runtime_strcasecmp(prop, "CLASS") == 0) {
                out_str = vdev_class_name(dev->dev_class);
            } else if (runtime_strcasecmp(prop, "DRIVER") == 0 || runtime_strcasecmp(prop, "DESCRIPTION") == 0 ||
                       runtime_strcasecmp(prop, "DESC") == 0) {
                out_str = dev->dev_description ? dev->dev_description : "Virtual Device";
            } else if (runtime_strcasecmp(prop, "VERSION") == 0 || runtime_strcasecmp(prop, "VER") == 0) {
                out_str = dev->dev_version ? dev->dev_version : "6.5.2";
            } else if (runtime_strcasecmp(prop, "STATUS") == 0) {
                int stat = dev->dev_status ? dev->dev_status(dev) : 1;
                out_str = (stat > 0) ? "1" : "0";
            } else {
                out_str = vdev_class_name(dev->dev_class);
            }
        } else {
            out_str = "None";
        }
    }

    res.as.string = str_create(vm_get_str(vm), out_str, runtime_strlen(out_str));
    return res;
}

BValue func_devcaps_eval(VMContext *vm, const char *name, int arg_count, BValue *args, BppError *err) {
    (void)name;
    BValue res;
    res.type = VAL_INTEGER;
    res.as.number = 0.0;

    if (!vm || !err || arg_count < 1 || !args) {
        if (err) {
            err->code = ERR_ILLEGAL_FUNCTION_CALL;
            err->message = "DEVCAPS requires 1 argument";
        }
        return res;
    }

    if (args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *dev_name = str_data(args[0].as.string);
    VDevContext *ctx = vm_get_vdev(vm);
    if (!ctx) return res;

    VDev *dev = vdev_get(ctx, dev_name);
    if (dev) {
        res.as.number = (double)dev->dev_caps;
    }
    return res;
}
