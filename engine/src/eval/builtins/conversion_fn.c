// FILENAME: conversion_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine
// Provides core logic and interface definitions for conversion_fn within BASIC++.
//
// ---- Includes ----

#include "eval/builtins_internal.h"
#include "eval/eval.h"
#include "eval/functions/datetime/pds_datetime.h"
#include "eval/functions/filesystem/descriptors/typ.h"
#include "eval/functions/math/financial/financial.h"
#include "eval/functions/string/conversion/bin.h"
#include "eval/functions/string/conversion/cvt.h"
#include "eval/functions/string/conversion/hex.h"
#include "eval/functions/string/conversion/mbf.h"
#include "eval/functions/string/conversion/oct.h"
#include "eval/functions/system/environment/category.h"
#include "eval/functions/system/time/clock_num.h"
#include "eval/functions/system/time/clock_str.h"
#include "eval/functions/system/environment/command_fn.h"
#include "eval/functions/system/time/date.h"
#include "eval/functions/system/environment/environ.h"
#include "eval/functions/system/environment/moddir.h"
#include "eval/functions/system/environment/pds_sys.h"
#include "eval/functions/system/time/time.h"
#include "eval/functions/ui/dialogs/vbdos_filebox.h"
#include "eval/functions/ui/dialogs/vbdos_fn.h"


//
// ---- Conversion & Financial Function Dispatcher ----

// evaluates radix representation, type conversion, date/time, and financial functions
bool eval_builtin_conversion(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err, BValue *out_res) {
    if (!uname || !out_res) return false;

    if (runtime_strcmp(uname, "HEX$") == 0 || runtime_strcmp(uname, "HEX") == 0) {
        *out_res = func_hex_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "OCT$") == 0 || runtime_strcmp(uname, "OCT") == 0) {
        *out_res = func_oct_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "BIN$") == 0 || runtime_strcmp(uname, "BIN") == 0) {
        *out_res = func_bin_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CVSMBF") == 0) {
        *out_res = func_cvsmbf_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CVDMBF") == 0) {
        *out_res = func_cvdmbf_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MKSMBF$") == 0) {
        *out_res = func_mksmbf_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MKDMBF$") == 0) {
        *out_res = func_mkdmbf_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DATE$") == 0 || runtime_strcmp(uname, "DATE") == 0 ||
        runtime_strcmp(uname, "DAY$") == 0 || runtime_strcmp(uname, "MONTH$") == 0) {
        *out_res = func_date_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PMT") == 0) {
        *out_res = func_pmt_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FV") == 0) {
        *out_res = func_fv_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PV") == 0) {
        *out_res = func_pv_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "NPER") == 0) {
        *out_res = func_nper_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RATE") == 0) {
        *out_res = func_rate_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "IPMT") == 0) {
        *out_res = func_ipmt_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PPMT") == 0) {
        *out_res = func_ppmt_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "NPV") == 0) {
        *out_res = func_npv_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "IRR") == 0) {
        *out_res = func_irr_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DATEVALUE") == 0) {
        *out_res = func_datevalue_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TIMEVALUE") == 0) {
        *out_res = func_timevalue_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DATESERIAL") == 0) {
        *out_res = func_dateserial_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TIMESERIAL") == 0) {
        *out_res = func_timeserial_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "YEAR") == 0) {
        *out_res = func_year_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MONTH") == 0) {
        *out_res = func_month_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DAY") == 0) {
        *out_res = func_day_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "HOUR") == 0) {
        *out_res = func_hour_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MINUTE") == 0) {
        *out_res = func_minute_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SECOND") == 0) {
        *out_res = func_second_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "WEEKDAY") == 0) {
        *out_res = func_weekday_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "COMMAND$") == 0 || runtime_strcmp(uname, "COMMAND") == 0) {
        *out_res = func_command_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CURDIR$") == 0 || runtime_strcmp(uname, "CURDIR") == 0) {
        *out_res = func_curdir_str_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DIR$") == 0 || runtime_strcmp(uname, "DIR") == 0) {
        *out_res = func_dir_str_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ENVIRON$") == 0 || runtime_strcmp(uname, "ENVIRON") == 0) {
        *out_res = func_environ_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CATEGORY$") == 0 || runtime_strcmp(uname, "CATEGORY") == 0) {
        *out_res = func_category_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "UNIXTIME") == 0) {
        *out_res = func_unixtime_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "EPOCHDATE") == 0) {
        *out_res = func_epochdate_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PREFIX$") == 0 || runtime_strcmp(uname, "PREFIX") == 0) {
        *out_res = func_curdir_str_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MODDIR$") == 0 || runtime_strcmp(uname, "MODDIR") == 0) {
        *out_res = func_moddir_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CLOCK$") == 0) {
        *out_res = func_clock_str_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CLOCK") == 0) {
        *out_res = func_clock_num_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FILEOPENBOX$") == 0 || runtime_strcmp(uname, "FILEOPENBOX") == 0) {
        *out_res = func_fileopenbox_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FILESAVEBOX$") == 0 || runtime_strcmp(uname, "FILESAVEBOX") == 0) {
        *out_res = func_filesavebox_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MSGBOX$") == 0 || runtime_strcmp(uname, "MSGBOX") == 0) {
        *out_res = func_msgbox_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "INPUTBOX$") == 0 || runtime_strcmp(uname, "INPUTBOX") == 0) {
        *out_res = func_inputbox_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DOEVENTS") == 0) {
        *out_res = func_doevents_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MKI$") == 0 || runtime_strcmp(uname, "CVI") == 0 ||
        runtime_strcmp(uname, "MKS$") == 0 || runtime_strcmp(uname, "CVS") == 0 ||
        runtime_strcmp(uname, "MKD$") == 0 || runtime_strcmp(uname, "CVD") == 0 ||
        runtime_strcmp(uname, "MKL$") == 0 || runtime_strcmp(uname, "CVL") == 0) {
        *out_res = func_cvt_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CINT") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13;
            err->message = "CINT expects numeric argument";
            return false;
        }
        out_res->type = VAL_NUMBER;
        out_res->as.number = (double)(int16_t)runtime_round(args[0].as.number);
        return true;
    }
    if (runtime_strcmp(uname, "CLNG") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13;
            err->message = "CLNG expects numeric argument";
            return false;
        }
        out_res->type = VAL_NUMBER;
        out_res->as.number = (double)(int32_t)runtime_round(args[0].as.number);
        return true;
    }
    if (runtime_strcmp(uname, "CSNG") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13;
            err->message = "CSNG expects numeric argument";
            return false;
        }
        out_res->type = VAL_NUMBER;
        out_res->as.number = (double)(float)args[0].as.number;
        return true;
    }
    if (runtime_strcmp(uname, "CDBL") == 0) {
        if (arg_count != 1 || args[0].type == VAL_STRING) {
            err->code = 13;
            err->message = "CDBL expects numeric argument";
            return false;
        }
        out_res->type = VAL_NUMBER;
        out_res->as.number = (double)args[0].as.number;
        return true;
    }
    if (runtime_strcmp(uname, "TYP") == 0) {
        *out_res = func_typ_eval(vm, uname, arg_count, args, err);
        return true;
    }

    return false;
}

