// FILENAME: sys_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel
// Provides core logic and interface definitions for sys_fn within BASIC++.
//
// ---- Includes ----

#include "eval/builtins_internal.h"
#include "eval/eval.h"
#include "eval/functions/bits/logic/and.h"
#include "eval/functions/bits/manipulation/bitcount.h"
#include "eval/functions/bits/logic/eqv.h"
#include "eval/functions/bits/logic/imp.h"
#include "eval/functions/bits/logic/not.h"
#include "eval/functions/bits/logic/or.h"
#include "eval/functions/bits/manipulation/readbit.h"
#include "eval/functions/bits/manipulation/resetbit.h"
#include "eval/functions/bits/manipulation/setbit.h"
#include "eval/functions/bits/shift/shl.h"
#include "eval/functions/bits/shift/shr.h"
#include "eval/functions/bits/shift/rol.h"
#include "eval/functions/bits/shift/ror.h"
#include "eval/functions/bits/manipulation/bitfield.h"
#include "eval/functions/bits/manipulation/togglebit.h"
#include "eval/functions/bits/logic/xor.h"
#include "eval/functions/filesystem/status/eof_fn.h"
#include "eval/functions/filesystem/descriptors/fid.h"
#include "eval/functions/filesystem/descriptors/freefile.h"
#include "eval/functions/filesystem/status/loc_fn.h"
#include "eval/functions/filesystem/status/lof.h"
#include "eval/functions/math/linear_algebra/lbound.h"
#include "eval/functions/math/linear_algebra/ubound.h"
#include "eval/functions/string/format/spc.h"
#include "eval/functions/string/format/tab.h"

#include "eval/functions/system/terminal/csrlin.h"
#include "eval/functions/system/error/erl.h"
#include "eval/functions/system/error/err_fn.h"
#include "eval/functions/system/environment/fre.h"
#include "eval/functions/system/terminal/inkey.h"
#include "eval/functions/system/hardware/inp.h"
#include "eval/functions/system/hardware/func_cpuspeed.h"
#include "eval/functions/system/hardware/func_clocks.h"
#include "eval/functions/system/ipc/func_receive.h"
#include "eval/functions/system/terminal/lpos.h"
#include "eval/functions/system/environment/dir_fn.h"
#include "eval/functions/system/environment/curdir.h"
#include "eval/functions/system/environment/setmem.h"
#include "eval/functions/system/environment/sseg.h"
#include "eval/functions/system/hardware/peek.h"
#include "eval/functions/system/terminal/pos.h"
#include "statements/io/gpib.h"
#include "eval/functions/filesystem/func_record_lock.h"
#include "eval/functions/filesystem/func_isam_fn.h"
#include "eval/functions/system/func_systems_types.h"
#include "statements/system/stmt_mutex.h"
#include "eval/functions/system/environment/sys_fn.h"
#include "eval/functions/system/environment/func_program.h"
#include "eval/functions/system/time/ticks.h"
#include "eval/functions/system/time/jiffies.h"
#include "eval/functions/system/time/ti.h"
#include "eval/functions/system/time/time.h"
#include "eval/functions/system/time/time_fn.h"
#include "eval/functions/system/time/func_tim.h"
#include "eval/functions/datetime/dateserial.h"
#include "eval/functions/datetime/timeserial.h"
#include "eval/functions/datetime/datevalue.h"
#include "eval/functions/datetime/timevalue.h"
#include "eval/functions/io/func_input_str.h"
#include "eval/functions/system/hardware/func_screen_fn.h"
#include "eval/functions/datetime/day.h"
#include "eval/functions/datetime/month.h"
#include "eval/functions/datetime/year.h"
#include "eval/functions/datetime/weekday.h"
#include "eval/functions/datetime/hour.h"
#include "eval/functions/datetime/minute.h"
#include "eval/functions/datetime/second.h"
#include "eval/functions/datetime/unixtime.h"
#include "eval/functions/datetime/epochdate.h"
#include "eval/functions/datetime/utc.h"
#include "eval/functions/datetime/time_part.h"
#include "statements/program/stmt_rename.h"
#include "statements/program/reformat.h"
#include "statements/program/renum.h"
#include "statements/program/stmt_revert.h"
#include "statements/debug/diagnostics/check.h"
#include "eval/functions/system/time/timer.h"
#include "eval/functions/ui/graphics/point_fn.h"
#include "functions/varptr.h"
#include "runtime/file.h"
#include "eval/functions/system/hardware/func_gemini_meta.h"
#include "eval/functions/system/hardware/func_tnfs.h"
#include "eval/functions/system/hardware/func_fujinet.h"
#include "eval/functions/string/manipulation/func_nil_compress.h"
#include "eval/functions/system/hardware/func_nil_bead.h"
#include "eval/functions/system/hardware/func_remote.h"
#include "eval/functions/system/hardware/func_sock.h"
#include "eval/functions/system/hardware/func_packet.h"
#include "eval/functions/func_devinfo.h"
#include "eval/functions/func_devctl.h"
#include "eval/functions/system/security/func_crypto.h"
#include "eval/functions/system/hardware/func_baud.h"
#include "eval/functions/system/hardware/stick.h"
#include "eval/functions/system/hardware/strig.h"
#include "eval/functions/system/hardware/paddle.h"
#include "eval/functions/system/hardware/ptrig.h"
#include "eval/functions/system/hardware/attr.h"
#include "runtime/string/strops.h"

// IoT & Microcontroller Built-in Functions
BValue func_dread_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_aread_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_touch_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_hall_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_ticks_ms_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_ticks_us_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_ticks_diff_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_mem_free_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_http_get_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_gemini_get_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_gopher_get_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_python_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_upnp_status_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

//
// ---- System & Bitwise Function Dispatcher ----

// evaluates system, I/O, bitwise, memory pointer, and device functions
bool eval_builtin_sys(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err, BValue *out_res) {
    if (!uname || !out_res) return false;

    if (runtime_strcmp(uname, "UPNP.STATUS") == 0 || runtime_strcmp(uname, "UPNP_STATUS") == 0 ||
        runtime_strcmp(uname, "UPNP.STATUS%") == 0) {
        *out_res = func_upnp_status_eval(vm, uname, arg_count, args, err);
        return true;
    }

    if (runtime_strcmp(uname, "TAB") == 0) {
        *out_res = func_tab_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SPC") == 0) {
        *out_res = func_spc_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "POS") == 0) {
        *out_res = func_pos_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "WBYTE") == 0) {
        *out_res = func_wbyte_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RBYTE") == 0) {
        *out_res = func_rbyte_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CSRLIN") == 0) {
        *out_res = func_csrlin_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LPOS") == 0) {
        *out_res = func_lpos_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ERL") == 0) {
        *out_res = func_erl_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ERR") == 0) {
        *out_res = func_err_fn_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "EOF") == 0) {
        *out_res = func_eof_fn_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LOF") == 0) {
        *out_res = func_lof_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LOC") == 0) {
        *out_res = func_loc_fn_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FREEFILE") == 0) {
        *out_res = func_freefile_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "POINT") == 0) {
        *out_res = func_point_fn_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LBOUND") == 0) {
        *out_res = func_lbound_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "UBOUND") == 0) {
        *out_res = func_ubound_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FRE") == 0) {
        *out_res = func_fre_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PEEK") == 0 || runtime_strcmp(uname, "EXAM") == 0 ||
        runtime_strcmp(uname, "DPEEK") == 0 ||
        runtime_strcmp(uname, "DEEK") == 0 || runtime_strcmp(uname, "LPEEK") == 0 ||
        runtime_strcmp(uname, "QPEEK") == 0 || runtime_strcmp(uname, "PEEK$") == 0 ||
        runtime_strcmp(uname, "PEEK2") == 0 || runtime_strcmp(uname, "PEEK4") == 0 ||
        runtime_strcmp(uname, "PEEK8") == 0) {
        *out_res = func_peek_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "INP") == 0 || runtime_strcmp(uname, "INPM") == 0) {
        *out_res = func_inp_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TICKS") == 0 || runtime_strcmp(uname, "TICKS_MS") == 0 || runtime_strcmp(uname, "_TICKS") == 0) {
        *out_res = func_ticks_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "JIFFIES") == 0) {
        *out_res = func_jiffies_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TI") == 0 || runtime_strcmp(uname, "TI$") == 0 || runtime_strcmp(uname, "TIMER$") == 0) {
        *out_res = func_ti_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TIME$") == 0) {
        *out_res = func_time_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PROGRAM$") == 0 || runtime_strcmp(uname, "PROGRAM") == 0) {
        *out_res = func_program_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DEVINFO$") == 0 || runtime_strcmp(uname, "DEVINFO") == 0) {
        *out_res = func_devinfo_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DEVCAPS") == 0 || runtime_strcmp(uname, "DEVCAPS%") == 0) {
        *out_res = func_devcaps_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DEVCTL") == 0 || runtime_strcmp(uname, "DEVCTL%") == 0) {
        *out_res = func_devctl_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DEVCTL$") == 0) {
        *out_res = func_devctl_str_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MESG") == 0 || runtime_strcmp(uname, "MESG$") == 0) {
        *out_res = func_mesg_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RECEIVE$") == 0 || runtime_strcmp(uname, "RECEIVE") == 0 ||
        runtime_strcmp(uname, "MSGRECV$") == 0 || runtime_strcmp(uname, "MSGRECV") == 0) {
        *out_res = func_receive_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TIME") == 0) {
        *out_res = func_time_fn_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TIM") == 0) {
        *out_res = func_tim_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TIME_PART") == 0 || runtime_strcmp(uname, "TIMEPART") == 0) {
        *out_res = func_time_part_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TIMER") == 0) {
        *out_res = func_timer_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SYS") == 0) {
        *out_res = func_sys_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SETMEM") == 0) {
        *out_res = func_setmem_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SSEG") == 0 || runtime_strcmp(uname, "SSEGADD") == 0) {
        *out_res = func_sseg_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FID") == 0) {
        *out_res = func_fid_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FIN") == 0) {
        *out_res = func_fin_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RECOUNT") == 0 || runtime_strcmp(uname, "_RECOUNT") == 0) {
        out_res->type = VAL_NUMBER;
        out_res->as.number = (double)file_get_recount(vm_get_file(vm));
        return true;
    }
    if (runtime_strcmp(uname, "STATUS") == 0 || runtime_strcmp(uname, "_STATUS") == 0) {
        out_res->type = VAL_NUMBER;
        out_res->as.number = 0.0;
        return true;
    }
    if (runtime_strcmp(uname, "INKEY$") == 0 || runtime_strcmp(uname, "INKEY") == 0) {
        *out_res = func_inkey_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "BAUD") == 0) {
        *out_res = func_baud_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CPUSPEED") == 0 || runtime_strcmp(uname, "CPUSPEED$") == 0 ||
        runtime_strcmp(uname, "SYS.CPUSPEED") == 0 || runtime_strcmp(uname, "SYS.CPUSPEED$") == 0) {
        *out_res = func_cpuspeed_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CLOCKS") == 0 || runtime_strcmp(uname, "CLOCKS$") == 0 ||
        runtime_strcmp(uname, "SYS.CLOCKS") == 0 || runtime_strcmp(uname, "SYS.CLOCKS$") == 0) {
        *out_res = func_clocks_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "STICK") == 0) {
        *out_res = func_stick_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "STRIG") == 0) {
        *out_res = func_strig_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PADDLE") == 0) {
        *out_res = func_paddle_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PTRIG") == 0) {
        *out_res = func_ptrig_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ATTR") == 0) {
        *out_res = func_attr_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SHL") == 0) {
        *out_res = func_shl_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SHR") == 0) {
        *out_res = func_shr_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ROL") == 0 || runtime_strcmp(uname, "_ROL") == 0 || runtime_strcmp(uname, "ROTL") == 0) {
        *out_res = func_rol_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ROR") == 0 || runtime_strcmp(uname, "_ROR") == 0 || runtime_strcmp(uname, "ROTR") == 0) {
        *out_res = func_ror_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "BIT") == 0 || runtime_strcmp(uname, "READBIT") == 0) {
        *out_res = func_readbit_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SETBIT") == 0) {
        *out_res = func_setbit_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CLRBIT") == 0 || runtime_strcmp(uname, "RESETBIT") == 0) {
        *out_res = func_resetbit_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TOGGLEBIT") == 0) {
        *out_res = func_togglebit_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "BITFIELD") == 0 || runtime_strcmp(uname, "_BITFIELD") == 0) {
        *out_res = func_bitfield_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "BITCOUNT") == 0) {
        *out_res = func_bitcount_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "AND") == 0) {
        *out_res = func_and_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "OR") == 0) {
        *out_res = func_or_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "XOR") == 0) {
        *out_res = func_xor_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "IMP") == 0) {
        *out_res = func_imp_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "EQV") == 0) {
        *out_res = func_eqv_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "NOT") == 0) {
        *out_res = func_not_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "VARPTR") == 0) {
        *out_res = func_varptr_eval(args, arg_count, vm);
        return true;
    }
    if (runtime_strcmp(uname, "VARPTR$") == 0) {
        *out_res = func_varptr_str_eval(args, arg_count, vm);
        return true;
    }

    if (runtime_strcmp(uname, "DREAD") == 0) {
        *out_res = func_dread_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "AREAD") == 0) {
        *out_res = func_aread_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TOUCH") == 0 || runtime_strcmp(uname, "TOUCH.READ") == 0) {
        *out_res = func_touch_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "HALL") == 0 || runtime_strcmp(uname, "HALL.READ") == 0) {
        *out_res = func_hall_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TICKS_MS") == 0) {
        *out_res = func_ticks_ms_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TICKS_US") == 0) {
        *out_res = func_ticks_us_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TICKS_DIFF") == 0) {
        *out_res = func_ticks_diff_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MEM.FREE") == 0 || runtime_strcmp(uname, "MEM.ALLOC") == 0) {
        *out_res = func_mem_free_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "HTTP.GET$") == 0) {
        *out_res = func_http_get_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "GEMINI.GET$") == 0) {
        *out_res = func_gemini_get_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "GEMINI.STATUS%") == 0 || runtime_strcmp(uname, "GEMINI.STATUS") == 0) {
        *out_res = func_gemini_status(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "GEMINI.META$") == 0) {
        *out_res = func_gemini_meta(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "GOPHER.GET$") == 0) {
        *out_res = func_gopher_get_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TNFS.DIR$") == 0) {
        *out_res = func_tnfs_dir(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FUJI.STATUS$") == 0) {
        *out_res = func_fuji_status(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FUJI.SSID$") == 0) {
        *out_res = func_fuji_ssid(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FUJI.IP$") == 0) {
        *out_res = func_fuji_ip(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "FUJI.JSON.GET$") == 0) {
        *out_res = func_fuji_json_get(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PYTHON$") == 0) {
        *out_res = func_python_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "COMSTR$") == 0 || runtime_strcmp(uname, "NIL.COMPRESS$") == 0) {
        *out_res = func_comstr_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DECOMSTR$") == 0 || runtime_strcmp(uname, "NIL.DECOMPRESS$") == 0) {
        *out_res = func_decomstr_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "NET.PACK$") == 0 || runtime_strcmp(uname, "NIL.PACK$") == 0) {
        *out_res = func_net_pack(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "REMOTE.EVAL$") == 0) {
        *out_res = func_remote_eval(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "IOT.RPC$") == 0) {
        *out_res = func_iot_rpc(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SOCK.OPEN%") == 0 || runtime_strcmp(uname, "SOCK.OPEN") == 0) {
        *out_res = func_sock_open(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SOCK.ACCEPT%") == 0 || runtime_strcmp(uname, "SOCK.ACCEPT") == 0) {
        *out_res = func_sock_accept(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SOCK.RECV$") == 0) {
        *out_res = func_sock_recv(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SOCK.POLL%") == 0 || runtime_strcmp(uname, "SOCK.POLL") == 0) {
        *out_res = func_sock_poll(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SOCK.STATUS%") == 0 || runtime_strcmp(uname, "SOCK.STATUS") == 0) {
        *out_res = func_sock_status(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PACKET.MAC$") == 0) {
        *out_res = func_packet_mac(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PACKET.RSSI%") == 0 || runtime_strcmp(uname, "PACKET.RSSI") == 0) {
        *out_res = func_packet_rssi(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PACKET.PAYLOAD$") == 0) {
        *out_res = func_packet_payload(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PACKET.LEN%") == 0 || runtime_strcmp(uname, "PACKET.LEN") == 0) {
        *out_res = func_packet_len(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PACKET.SRC$") == 0) {
        *out_res = func_packet_src(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PACKET.PORT%") == 0 || runtime_strcmp(uname, "PACKET.PORT") == 0) {
        *out_res = func_packet_port(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PACKET.TYPE%") == 0 || runtime_strcmp(uname, "PACKET.TYPE") == 0) {
        *out_res = func_packet_type(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CRYPTO.ENCRYPT$") == 0) {
        *out_res = func_crypto_encrypt(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CRYPTO.DECRYPT$") == 0) {
        *out_res = func_crypto_decrypt(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CRYPTO.HASH$") == 0) {
        *out_res = func_crypto_hash(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CRYPTO.HMAC$") == 0) {
        *out_res = func_crypto_hmac(vm, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CRYPTO.KEY$") == 0) {
        *out_res = func_crypto_key(vm, arg_count, args, err);
        return true;
    }

    if (runtime_strcmp(uname, "DEVINFO$") == 0 || runtime_strcmp(uname, "DEVINFO") == 0) {
        *out_res = func_devinfo_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DEVCAPS%") == 0 || runtime_strcmp(uname, "DEVCAPS") == 0) {
        *out_res = func_devcaps_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RECEIVE$") == 0 || runtime_strcmp(uname, "RECEIVE") == 0 ||
        runtime_strcmp(uname, "MSGRECV$") == 0 || runtime_strcmp(uname, "MSGRECV") == 0) {
        *out_res = func_receive_eval(vm, uname, arg_count, args, err);
        return true;
    }

    if (runtime_strcmp(uname, "RENAME") == 0) {
        *out_res = func_rename_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "REFORMAT") == 0) {
        *out_res = func_reformat_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CHECK") == 0) {
        *out_res = func_check_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RENUM") == 0) {
        *out_res = func_renum_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "REVERT") == 0) {
        *out_res = func_revert_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "INPUT$") == 0) {
        *out_res = func_input_str_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SCREEN") == 0) {
        *out_res = func_screen_fn_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "READU") == 0 || runtime_strcmp(uname, "READU$") == 0) {
        *out_res = func_readu_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "WRITEU") == 0) {
        *out_res = func_writeu_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RELEASE") == 0) {
        *out_res = func_release_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "LOCKED") == 0) {
        *out_res = func_locked_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "KEY$") == 0) {
        *out_res = func_key_str_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "KEYCOUNT") == 0) {
        *out_res = func_keycount_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "ISAM") == 0 || runtime_strcmp(uname, "KEYED") == 0) {
        *out_res = func_isam_check_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CBYTE") == 0 || runtime_strcmp(uname, "BYTE") == 0) {
        *out_res = func_byte_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CWORD") == 0 || runtime_strcmp(uname, "WORD") == 0) {
        *out_res = func_word_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "CDWORD") == 0 || runtime_strcmp(uname, "DWORD") == 0) {
        *out_res = func_dword_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "PTR") == 0 || runtime_strcmp(uname, "POINTER") == 0) {
        *out_res = func_ptr_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "DEREF") == 0) {
        *out_res = func_deref_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MASK") == 0) {
        *out_res = func_mask_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SET_BITFIELD") == 0) {
        *out_res = func_set_bitfield_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MUTEX") == 0) {
        *out_res = func_mutex_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MUTEX_LOCK") == 0) {
        *out_res = func_mutex_lock_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "MUTEX_UNLOCK") == 0) {
        *out_res = func_mutex_unlock_eval(vm, uname, arg_count, args, err);
        return true;
    }

    if (arg_count == 0 && eval_try_resolve_builtin_constant_or_system_var(vm, uname, out_res)) {
        return true;
    }

    return false;
}

