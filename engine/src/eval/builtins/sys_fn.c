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
#include "eval/functions/system/terminal/lpos.h"
#include "eval/functions/system/environment/pds_sys.h"
#include "eval/functions/system/hardware/peek.h"
#include "eval/functions/system/terminal/pos.h"
#include "eval/functions/system/environment/sys_fn.h"
#include "eval/functions/system/time/ticks.h"
#include "eval/functions/system/time/time.h"
#include "eval/functions/system/time/time_fn.h"
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
#include "eval/functions/system/security/func_crypto.h"

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

//
// ---- System & Bitwise Function Dispatcher ----

// evaluates system, I/O, bitwise, memory pointer, and device functions
bool eval_builtin_sys(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err, BValue *out_res) {
    if (!uname || !out_res) return false;

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
    if (runtime_strcmp(uname, "PEEK") == 0) {
        *out_res = func_peek_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "INP") == 0 || runtime_strcmp(uname, "INPM") == 0) {
        *out_res = func_inp_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TICKS") == 0) {
        *out_res = func_ticks_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TIME$") == 0) {
        *out_res = func_time_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TIME") == 0) {
        *out_res = func_time_fn_eval(vm, uname, arg_count, args, err);
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
    if (runtime_strcmp(uname, "SHL") == 0) {
        *out_res = func_shl_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SHR") == 0) {
        *out_res = func_shr_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "READBIT") == 0) {
        *out_res = func_readbit_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "SETBIT") == 0) {
        *out_res = func_setbit_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "RESETBIT") == 0) {
        *out_res = func_resetbit_eval(vm, uname, arg_count, args, err);
        return true;
    }
    if (runtime_strcmp(uname, "TOGGLEBIT") == 0) {
        *out_res = func_togglebit_eval(vm, uname, arg_count, args, err);
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

    return false;
}

