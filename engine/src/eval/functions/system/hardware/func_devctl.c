// FILENAME: func_devctl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, sys_fn.c
// NEEDS: libkernel (types.h, errors.h, vdev.h, dev_user.h), libcore (strings.h)
// Implementation for DEVCTL, DEVCTL$, and MESG built-in functions.
//
// ---- Includes ----

#include "eval/functions/func_devctl.h"
#include "device/vdev.h"
#include "device/dev_user.h"
#include "vm/vm.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_devctl_desc = {
    .name = "DEVCTL",
    .category = "Device Control",
    .syntax = "DEVCTL(dev$, cmd[, arg1, arg2])",
    .description = "Sends hardware device control command and returns integer status code.",
    .error_summary = "Error 5: Illegal Function Call (invalid device or command)",
    .subsystem = SUBSYSTEM_KERNEL,
    .safety = SAFETY_UNSAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_devctl_str_desc = {
    .name = "DEVCTL$",
    .category = "Device Control",
    .syntax = "DEVCTL$(dev$, cmd[, arg1, arg2])",
    .description = "Sends hardware device control command and returns response string.",
    .error_summary = "Error 5: Illegal Function Call (invalid device or command)",
    .subsystem = SUBSYSTEM_KERNEL,
    .safety = SAFETY_UNSAFE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_mesg_desc = {
    .name = "MESG",
    .category = "Device Control",
    .syntax = "MESG / MESG$()",
    .description = "Inspects whether current user has message reception privileges.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_KERNEL,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

void func_devctl_register(void) {
    lang_desc_register(&g_devctl_desc);
    lang_desc_register(&g_devctl_str_desc);
    lang_desc_register(&g_mesg_desc);
}

static int parse_cmd_code_arg(const BValue *val) {
    if (!val) return 0;
    if (val->type == VAL_NUMBER || val->type == VAL_INTEGER) {
        return (int)val->as.number;
    }
    if (val->type == VAL_STRING && val->as.string) {
        const char *s = str_data(val->as.string);
        if (!s) return 0;
        if (runtime_strcasecmp(s, "RESET") == 0) return 0;
        if (runtime_strcasecmp(s, "STATUS") == 0) return 13;
        if (runtime_strcasecmp(s, "SET_BAUD") == 0) return 70;
        if (runtime_strcasecmp(s, "MOTOR_ON") == 0) return 45;
        if (runtime_strcasecmp(s, "MOTOR_OFF") == 0) return 44;
        if (runtime_strcasecmp(s, "REWIND") == 0) return 43;
        if (runtime_strcasecmp(s, "DRAWTO") == 0) return 40;
        if (runtime_strcasecmp(s, "FILL") == 0) return 41;
        if (runtime_strcasecmp(s, "TALK") == 0) return 130;
        if (runtime_strcasecmp(s, "LISTEN") == 0) return 131;
        if (runtime_strcasecmp(s, "UNLISTEN") == 0) return 132;
        if (runtime_strcasecmp(s, "UNTALK") == 0) return 133;
        if (runtime_strcasecmp(s, "CBM_STATUS") == 0) return 135;
        if (runtime_strcasecmp(s, "CBM_RESET") == 0) return 136;
        if (runtime_strcasecmp(s, "STEP_IN") == 0) return 140;
        if (runtime_strcasecmp(s, "STEP_OUT") == 0) return 141;
        if (runtime_strcasecmp(s, "READ_NIBBLE") == 0) return 143;
        if (runtime_strcasecmp(s, "WRITE_NIBBLE") == 0) return 144;
        if (runtime_strcasecmp(s, "SIO_SEND") == 0) return 160;
        if (runtime_strcasecmp(s, "SIO_STATUS") == 0) return 161;
        if (runtime_strcasecmp(s, "PBI") == 0) return 164;
        if (runtime_strcasecmp(s, "ECI") == 0) return 166;
        if (runtime_strcasecmp(s, "PIN_MODE") == 0) return 200;
        if (runtime_strcasecmp(s, "DIGITAL_WRITE") == 0) return 201;
        if (runtime_strcasecmp(s, "DIGITAL_READ") == 0) return 202;
        if (runtime_strcasecmp(s, "PWM") == 0 || runtime_strcasecmp(s, "ANALOG_WRITE") == 0) return 203;
        if (runtime_strcasecmp(s, "ADC") == 0 || runtime_strcasecmp(s, "ANALOG_READ") == 0) return 204;
        if (runtime_strcasecmp(s, "I2C") == 0) return 205;
        if (runtime_strcasecmp(s, "SPI") == 0) return 206;
        if (runtime_strcasecmp(s, "DAC") == 0) return 207;
        if (runtime_strcasecmp(s, "SERVO") == 0) return 208;
        if (runtime_strcasecmp(s, "NEOPIXEL") == 0) return 209;
        if (runtime_strcasecmp(s, "SENSOR") == 0) return 210;
        if (runtime_strcasecmp(s, "FORWARD") == 0 || runtime_strcasecmp(s, "UPNP_FORWARD") == 0) return 300;
        if (runtime_strcasecmp(s, "UNFORWARD") == 0 || runtime_strcasecmp(s, "UPNP_UNFORWARD") == 0) return 301;
        if (runtime_strcasecmp(s, "EXTERNAL_IP") == 0 || runtime_strcasecmp(s, "UPNP_EXTERNAL_IP") == 0) return 302;
        if (runtime_strcasecmp(s, "STATUS") == 0 || runtime_strcasecmp(s, "UPNP_STATUS") == 0) return 303;
    }
    return 0;
}

BValue func_devctl_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 2) {
        err->code = ERR_SYNTAX;
        err->message = "DEVCTL requires at least 2 arguments (dev$, cmd)";
        return res;
    }

    if (args[0].type != VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *dev_name = args[0].as.string ? str_data(args[0].as.string) : "";
    int cmd = parse_cmd_code_arg(&args[1]);
    int aux1 = (arg_count >= 3 && (args[2].type == VAL_NUMBER || args[2].type == VAL_INTEGER)) ? (int)args[2].as.number : 0;
    int aux2 = (arg_count >= 4 && (args[3].type == VAL_NUMBER || args[3].type == VAL_INTEGER)) ? (int)args[3].as.number : 0;

    VDevContext *vctx = vm_get_vdev(vm);
    VDev *dev = vctx ? vdev_get(vctx, dev_name) : NULL;
    if (!dev) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "Device not found or reserved";
        return res;
    }

    int iargs[4] = {aux1, aux2, 0, 0};
    int ret = vdev_ioctl(dev, cmd, iargs);
    res.as.number = (double)ret;
    return res;
}

BValue func_devctl_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (arg_count < 2) {
        err->code = ERR_SYNTAX;
        err->message = "DEVCTL$ requires at least 2 arguments (dev$, cmd)";
        return res;
    }

    if (args[0].type != VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        return res;
    }

    const char *dev_name = args[0].as.string ? str_data(args[0].as.string) : "";
    int cmd = parse_cmd_code_arg(&args[1]);
    int aux1 = (arg_count >= 3 && (args[2].type == VAL_NUMBER || args[2].type == VAL_INTEGER)) ? (int)args[2].as.number : 0;
    int aux2 = (arg_count >= 4 && (args[3].type == VAL_NUMBER || args[3].type == VAL_INTEGER)) ? (int)args[3].as.number : 0;

    VDevContext *vctx = vm_get_vdev(vm);
    VDev *dev = vctx ? vdev_get(vctx, dev_name) : NULL;
    if (!dev) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "Device not found or reserved";
        return res;
    }

    int iargs[4] = {aux1, aux2, 0, 0};
    int ret = vdev_ioctl(dev, cmd, iargs);
    char out_buf[64];
    runtime_snprintf(out_buf, sizeof(out_buf), "%d", ret);

    res.as.string = str_create(vm_get_str(vm), out_buf, runtime_strlen(out_buf));
    return res;
}

BValue func_mesg_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args; (void)err;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));

    bool is_str = (runtime_strcasecmp(uname, "MESG$") == 0);
    bool allowed = dev_user_get_mesg(dev_user_current_username());

    if (is_str) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), allowed ? "y" : "n", 1);
    } else {
        res.type = VAL_NUMBER;
        res.as.number = allowed ? 1.0 : 0.0;
    }
    return res;
}
