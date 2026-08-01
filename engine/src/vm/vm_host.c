/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ — a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * What it does: Implements the canonical BASIC++ VM binding for BppHostInterface.
 * Why it exists: Connects the host-agnostic BppHostInterface to VMContext operations.
 * Why it works this way: Wraps VM variable, I/O, and error APIs in static function callbacks.
 * What can be changed: Internal callback error formatting and device lookup dispatch.
 * What cannot be changed: BppHostInterface function pointer types.
 * What to expect: Seamless execution of host-decoupled subsystems backed by the full VM.
 * What to do if something breaks: Check callback VMContext pointer validity.
 * Assumptions: VMContext is initialized.
 * Portability concerns: Strict C17 compliant, pure 7-bit ASCII.
 * Future expansions: Add capability flags negotiation to vm_create_host_interface.
 * External extension hooks: Exposed via bpp_vm.h.
 */

#include "vm/host.h"
#include "vm/vm.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "device/vdev.h"
#include <stdio.h>
#include <string.h>

static void host_vm_output(void *user_data, const char *text) {
    VMContext *vm = (VMContext *)user_data;
    if (vm && text) {
        vdev_puts(vm_get_vdev(vm), text);
    }
}

static void host_vm_output_char(void *user_data, char c) {
    VMContext *vm = (VMContext *)user_data;
    if (vm) {
        vdev_putc(vm_get_vdev(vm), (int)c);
    }
}

static int host_vm_input_line(void *user_data, char *buf, int max_len) {
    (void)user_data; (void)buf; (void)max_len;
    return 0;
}

static int host_vm_input_char(void *user_data) {
    (void)user_data;
    return -1;
}

static double host_vm_get_numeric_var(void *user_data, const char *name) {
    VMContext *vm = (VMContext *)user_data;
    if (!vm || !name) return 0.0;
    BValue *val = var_lookup(vm_get_var(vm), name, false);
    return val ? val->as.number : 0.0;
}

static void host_vm_set_numeric_var(void *user_data, const char *name, double val) {
    VMContext *vm = (VMContext *)user_data;
    if (vm && name) {
        BValue *v = var_lookup(vm_get_var(vm), name, true);
        if (v) {
            v->type = VAL_NUMBER;
            v->as.number = val;
        }
    }
}

static const char *host_vm_get_string_var(void *user_data, const char *name) {
    VMContext *vm = (VMContext *)user_data;
    if (!vm || !name) return "";
    BValue *val = var_lookup(vm_get_var(vm), name, false);
    if (val && val->type == VAL_STRING) {
        const char *cstr = str_data(val->as.string);
        return cstr ? cstr : "";
    }
    return "";
}

static void host_vm_set_string_var(void *user_data, const char *name, const char *val) {
    VMContext *vm = (VMContext *)user_data;
    if (vm && name && val) {
        BValue *v = var_lookup(vm_get_var(vm), name, true);
        if (v) {
            v->type = VAL_STRING;
            v->as.string = str_create(vm_get_str(vm), val, strlen(val));
        }
    }
}

static void host_vm_report_error(void *user_data, BppErrorCode code, const char *msg) {
    VMContext *vm = (VMContext *)user_data;
    if (vm) {
        vm_set_error(vm, (int)code, msg);
    }
}

static void *host_vm_get_device(void *user_data, const char *device_name) {
    VMContext *vm = (VMContext *)user_data;
    if (!vm || !device_name) return NULL;
    return (void *)vdev_get(vm_get_vdev(vm), device_name);
}

BppHostInterface vm_create_host_interface(VMContext *vm) {
    BppHostInterface host;
    host.user_data = (void *)vm;
    host.output = host_vm_output;
    host.output_char = host_vm_output_char;
    host.input_line = host_vm_input_line;
    host.input_char = host_vm_input_char;
    host.get_numeric_var = host_vm_get_numeric_var;
    host.set_numeric_var = host_vm_set_numeric_var;
    host.get_string_var = host_vm_get_string_var;
    host.set_string_var = host_vm_set_string_var;
    host.report_error = host_vm_report_error;
    host.get_device = host_vm_get_device;
    return host;
}
