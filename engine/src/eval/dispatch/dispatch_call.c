// FILENAME: dispatch_call.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (dispatch_internal.h)
// Provides core logic and interface definitions for dispatch_call within BASIC++.
//
// ---- Includes ----

#include "eval/dispatch_internal.h"
#include "eval/functions/system/hardware/func_cpuspeed.h"
#include "eval/functions/system/hardware/func_clocks.h"
#include "eval/functions/bits/manipulation/bitfield.h"
#include "eval/functions/bits/shift/rol.h"
#include "eval/functions/bits/shift/ror.h"
#include "eval/functions/string/manipulation/pick.h"
#include "eval/functions/math/trig/pol_rec.h"
#include "eval/functions/string/manipulation/translate.h"
#include "eval/functions/filesystem/func_record_lock.h"
#include "eval/functions/filesystem/func_isam_fn.h"
#include "eval/functions/system/func_systems_types.h"
#include "eval/functions/system/hardware/peek.h"
#include "runtime/arrays.h"
#include "runtime/string/memops.h"
#include "runtime/math/math.h"

static uint64_t s_dispatch_rnd_seed = 123456789ULL;
static inline double dispatch_rand_float(void) {
    s_dispatch_rnd_seed = s_dispatch_rnd_seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return (double)(s_dispatch_rnd_seed >> 11) / (double)(1ULL << 53);
}

//
// ---- Function Evaluation & Call Dispatch ----
//

BValue eval_builtin_function_delim(VMContext *vm, const char *name, LexerContext *lex, BppTokenType open_delim, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    char uname[64];
    normalize_func_name(name, uname, sizeof(uname));

    bool has_delim = (open_delim == TOK_LPAREN || open_delim == TOK_LBRACKET || open_delim == TOK_LBRACE);
    if (!has_delim) {
        return eval_builtin_function_impl(vm, uname, 0, NULL, err);
    }

    if (open_delim == TOK_LBRACKET) {
        if (runtime_strcasecmp(uname, "CPUSPEED") == 0 || runtime_strcasecmp(uname, "SYS.CPUSPEED") == 0) {
            return func_cpuspeed_eval_bracket(vm, lex, err);
        }
        if (runtime_strcasecmp(uname, "CLOCKS") == 0 || runtime_strcasecmp(uname, "SYS.CLOCKS") == 0) {
            return func_clocks_eval_bracket(vm, lex, err);
        }
        if (runtime_strcasecmp(uname, "MAXLEN") == 0 || runtime_strcasecmp(uname, "MAXLEN$") == 0) {
            BppToken name_tok = lex_next(lex);
            if (name_tok.type != TOK_IDENT && name_tok.type != TOK_KEYWORD) {
                err->code = 2;
                err->message = "Expected variable name in MAXLEN";
                return res;
            }
            char var_name[256];
            size_t clen = (name_tok.length < sizeof(var_name) - 1) ? name_tok.length : sizeof(var_name) - 1;
            runtime_memcpy(var_name, name_tok.start, clen);
            var_name[clen] = '\0';
            if (lex_peek(lex).type == TOK_RBRACKET) {
                lex_next(lex);
            }
            size_t mlen = var_get_max_len(vm_get_var(vm), var_name);
            res.type = VAL_INTEGER;
            res.as.number = (double)mlen;
            return res;
        }
        if (runtime_strcasecmp(uname, "SUM") == 0 || runtime_strcasecmp(uname, "AVG") == 0 ||
            runtime_strcasecmp(uname, "MEAN") == 0 || runtime_strcasecmp(uname, "MIN") == 0 ||
            runtime_strcasecmp(uname, "MAX") == 0) {
            const char *save_pos = lex_get_pos(lex);
            BppToken name_tok = lex_peek(lex);
            if (name_tok.type == TOK_IDENT) {
                char arr_name[256];
                size_t clen = (name_tok.length < sizeof(arr_name) - 1) ? name_tok.length : sizeof(arr_name) - 1;
                runtime_memcpy(arr_name, name_tok.start, clen);
                arr_name[clen] = '\0';
                if (arr_exists(vm_get_arr(vm), arr_name)) {
                    lex_next(lex);
                    if (lex_peek(lex).type == TOK_RBRACKET) {
                        lex_next(lex);
                        int total_size = 0;
                        BValue *elems = arr_get_flat_elements(vm_get_arr(vm), arr_name, &total_size);
                        res.type = VAL_NUMBER;
                        if (runtime_strcasecmp(uname, "SUM") == 0) {
                            double s = 0.0;
                            for (int i = 0; i < total_size; i++) {
                                if (elems && (elems[i].type == VAL_NUMBER || elems[i].type == VAL_INTEGER)) s += elems[i].as.number;
                            }
                            res.as.number = s;
                        } else if (runtime_strcasecmp(uname, "AVG") == 0 || runtime_strcasecmp(uname, "MEAN") == 0) {
                            double s = 0.0; int cnt = 0;
                            for (int i = 0; i < total_size; i++) {
                                if (elems && (elems[i].type == VAL_NUMBER || elems[i].type == VAL_INTEGER)) { s += elems[i].as.number; cnt++; }
                            }
                            res.as.number = (cnt > 0) ? (s / (double)cnt) : 0.0;
                        } else if (runtime_strcasecmp(uname, "MIN") == 0) {
                            double mv = 0.0; bool first = true;
                            for (int i = 0; i < total_size; i++) {
                                if (elems && (elems[i].type == VAL_NUMBER || elems[i].type == VAL_INTEGER)) {
                                    if (first || elems[i].as.number < mv) { mv = elems[i].as.number; first = false; }
                                }
                            }
                            res.as.number = mv;
                        } else if (runtime_strcasecmp(uname, "MAX") == 0) {
                            double mv = 0.0; bool first = true;
                            for (int i = 0; i < total_size; i++) {
                                if (elems && (elems[i].type == VAL_NUMBER || elems[i].type == VAL_INTEGER)) {
                                    if (first || elems[i].as.number > mv) { mv = elems[i].as.number; first = false; }
                                }
                            }
                            res.as.number = mv;
                        }
                        return res;
                    }
                }
            }
            lex_set_pos(lex, save_pos);
        }
        if (runtime_strcasecmp(uname, "COUNT") == 0 || runtime_strcasecmp(uname, "COUNT$") == 0) {
            const char *save_pos = lex_get_pos(lex);
            BppToken name_tok = lex_peek(lex);
            if (name_tok.type == TOK_IDENT) {
                char arr_name[256];
                size_t clen = (name_tok.length < sizeof(arr_name) - 1) ? name_tok.length : sizeof(arr_name) - 1;
                runtime_memcpy(arr_name, name_tok.start, clen);
                arr_name[clen] = '\0';
                if (arr_exists(vm_get_arr(vm), arr_name)) {
                    lex_next(lex);
                    if (lex_peek(lex).type == TOK_RBRACKET) {
                        lex_next(lex);
                        int total_size = 0;
                        arr_get_flat_elements(vm_get_arr(vm), arr_name, &total_size);
                        res.type = VAL_NUMBER;
                        res.as.number = (double)total_size;
                        return res;
                    }
                }
            }
            lex_set_pos(lex, save_pos);
        }
    }
    if (open_delim == TOK_LBRACE) {
        if (runtime_strcasecmp(uname, "CPUSPEED") == 0 || runtime_strcasecmp(uname, "SYS.CPUSPEED") == 0) {
            return func_cpuspeed_eval_brace(vm, lex, err);
        }
        if (runtime_strcasecmp(uname, "CLOCKS") == 0 || runtime_strcasecmp(uname, "SYS.CLOCKS") == 0) {
            return func_clocks_eval_brace(vm, lex, err);
        }
    }

    if (open_delim == TOK_LPAREN && dispatch_handle_special(vm, uname, lex, true, &res, err)) {
        return res;
    }

    BppTokenType close_delim = (open_delim == TOK_LPAREN) ? TOK_RPAREN : ((open_delim == TOK_LBRACKET) ? TOK_RBRACKET : TOK_RBRACE);

    BValue args[32];
    int arg_count = 0;
    runtime_memset(args, 0, sizeof(args));

    BppToken tok = lex_peek(lex);
    if (tok.type != close_delim) {
        while (true) {
            if (arg_count >= 32) {
                err->code = 2;
                err->message = "Too many arguments for function";
                break;
            }
            args[arg_count] = eval_expression(vm, lex, err);
            if (err->code != 0) break;
            arg_count++;

            tok = lex_peek(lex);
            if (tok.type == TOK_COMMA) {
                lex_next(lex);
            } else if (tok.type == close_delim) {
                break;
            } else {
                err->code = 2;
                err->message = "Expected delimiter or comma in function call";
                break;
            }
        }
    }

    if (err->code == 0) {
        tok = lex_next(lex);
        if (tok.type != close_delim) {
            err->code = 2;
            err->message = "Mismatched closing delimiter in function call";
        }
    }

    if (err->code == 0) {
        if (open_delim == TOK_LBRACKET) {
            if (runtime_strcasecmp(uname, "POL") == 0) {
                return func_pol_eval(vm, uname, arg_count, args, err);
            }
            if (runtime_strcasecmp(uname, "REC") == 0) {
                return func_rec_eval(vm, uname, arg_count, args, err);
            }
            if (runtime_strcasecmp(uname, "TRANSLATE$") == 0 || runtime_strcasecmp(uname, "TRANSLATE") == 0) {
                if (lex_peek(lex).type == TOK_LPAREN) {
                    lex_next(lex); // consume (
                    BValue src_arg = eval_expression(vm, lex, err);
                    if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
                    if (err->code != 0) return res;
                    BValue call_args[3];
                    call_args[0] = src_arg;
                    call_args[1] = args[0];
                    if (arg_count >= 2) {
                        call_args[2] = args[1];
                        return func_translate_eval(vm, uname, 3, call_args, err);
                    } else {
                        return func_translate_eval(vm, uname, 2, call_args, err);
                    }
                }
                return func_translate_eval(vm, uname, arg_count, args, err);
            }
            if (runtime_strcasecmp(uname, "RND") == 0 && arg_count == 2) {
                double min_v = (args[0].type == VAL_NUMBER) ? args[0].as.number : 0.0;
                double max_v = (args[1].type == VAL_NUMBER) ? args[1].as.number : 1.0;
                if (min_v > max_v) { double t = min_v; min_v = max_v; max_v = t; }
                double r = dispatch_rand_float();
                res.type = VAL_NUMBER;
                res.as.number = min_v + r * (max_v - min_v);
                return res;
            }
            if (runtime_strcasecmp(uname, "CLAMP") == 0 && arg_count == 3) {
                double val = (args[0].type == VAL_NUMBER) ? args[0].as.number : 0.0;
                double lo = (args[1].type == VAL_NUMBER) ? args[1].as.number : 0.0;
                double hi = (args[2].type == VAL_NUMBER) ? args[2].as.number : 0.0;
                res.type = VAL_NUMBER;
                res.as.number = runtime_clamp(val, lo, hi);
                return res;
            }
            if (runtime_strcasecmp(uname, "ROUND") == 0 && arg_count == 2) {
                double val = (args[0].type == VAL_NUMBER) ? args[0].as.number : 0.0;
                int dec = (int)((args[1].type == VAL_NUMBER) ? args[1].as.number : 0.0);
                double factor = runtime_pow(10.0, (double)dec);
                res.type = VAL_NUMBER;
                res.as.number = runtime_round(val * factor) / factor;
                return res;
            }
            if (runtime_strcasecmp(uname, "LERP") == 0 && arg_count == 3) {
                double a = (args[0].type == VAL_NUMBER) ? args[0].as.number : 0.0;
                double b = (args[1].type == VAL_NUMBER) ? args[1].as.number : 0.0;
                double t = (args[2].type == VAL_NUMBER) ? args[2].as.number : 0.0;
                res.type = VAL_NUMBER;
                res.as.number = runtime_lerp(a, b, t);
                return res;
            }
            if ((runtime_strcasecmp(uname, "MID$") == 0 || runtime_strcasecmp(uname, "MID") == 0) && arg_count == 3) {
                if (args[0].type == VAL_STRING && args[0].as.string) {
                    const char *src = str_data(args[0].as.string);
                    size_t slen = str_len(args[0].as.string);
                    int start = (int)((args[1].type == VAL_NUMBER) ? args[1].as.number : 1.0);
                    int end = (int)((args[2].type == VAL_NUMBER) ? args[2].as.number : (double)slen);
                    if (start < 1) start = 1;
                    if (end > (int)slen) end = (int)slen;
                    if (start <= end && (size_t)(start - 1) < slen) {
                        size_t sub_len = (size_t)(end - start + 1);
                        res.type = VAL_STRING;
                        res.as.string = str_create(vm_get_str(vm), src + (start - 1), sub_len);
                        return res;
                    } else {
                        res.type = VAL_STRING;
                        res.as.string = str_create(vm_get_str(vm), "", 0);
                        return res;
                    }
                }
            }
            if (runtime_strcasecmp(uname, "BITFIELD") == 0 || runtime_strcasecmp(uname, "_BITFIELD") == 0) {
                if (lex_peek(lex).type == TOK_LPAREN) {
                    lex_next(lex); // consume (
                    BValue val_arg = eval_expression(vm, lex, err);
                    if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
                    if (err->code != 0) return res;
                    BValue call_args[3];
                    call_args[0] = val_arg;
                    call_args[1] = (arg_count >= 1) ? args[0] : (BValue){ .type = VAL_NUMBER, .as.number = 0.0 };
                    call_args[2] = (arg_count >= 2) ? args[1] : (BValue){ .type = VAL_NUMBER, .as.number = 0.0 };
                    return func_bitfield_eval(vm, uname, 3, call_args, err);
                }
                if (arg_count == 3) {
                    return func_bitfield_eval(vm, uname, arg_count, args, err);
                }
            }
            if ((runtime_strcasecmp(uname, "ROL") == 0 || runtime_strcasecmp(uname, "_ROL") == 0) && (arg_count == 2 || arg_count == 3)) {
                return func_rol_eval(vm, uname, arg_count, args, err);
            }
            if ((runtime_strcasecmp(uname, "ROR") == 0 || runtime_strcasecmp(uname, "_ROR") == 0) && (arg_count == 2 || arg_count == 3)) {
                return func_ror_eval(vm, uname, arg_count, args, err);
            }
            if ((runtime_strcasecmp(uname, "BYTE") == 0 || runtime_strcasecmp(uname, "CBYTE") == 0) && arg_count >= 1) {
                return func_peek_eval(vm, "PEEK", 1, args, err);
            }
            if ((runtime_strcasecmp(uname, "WORD") == 0 || runtime_strcasecmp(uname, "CWORD") == 0) && arg_count >= 1) {
                BValue width_val = { .type = VAL_NUMBER, .as.number = 2.0 };
                BValue peek_args[2] = { args[0], width_val };
                return func_peek_eval(vm, "PEEK", 2, peek_args, err);
            }
            if ((runtime_strcasecmp(uname, "DWORD") == 0 || runtime_strcasecmp(uname, "CDWORD") == 0) && arg_count >= 1) {
                BValue width_val = { .type = VAL_NUMBER, .as.number = 4.0 };
                BValue peek_args[2] = { args[0], width_val };
                return func_peek_eval(vm, "PEEK", 2, peek_args, err);
            }
            if (runtime_strcasecmp(uname, "PTR") == 0 && arg_count >= 1) {
                return func_ptr_eval(vm, uname, arg_count, args, err);
            }
            if (runtime_strcasecmp(uname, "MASK") == 0) {
                if (lex_peek(lex).type == TOK_LPAREN) {
                    lex_next(lex); // consume (
                    BValue val_arg = eval_expression(vm, lex, err);
                    if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
                    if (err->code != 0) return res;
                    BValue call_args[2];
                    call_args[0] = val_arg;
                    call_args[1] = (arg_count >= 1) ? args[0] : (BValue){ .type = VAL_NUMBER, .as.number = 0.0 };
                    return func_mask_eval(vm, uname, 2, call_args, err);
                }
                if (arg_count == 2) {
                    return func_mask_eval(vm, uname, arg_count, args, err);
                }
            }
            if (runtime_strcasecmp(uname, "KEY$") == 0 && arg_count >= 1) {
                return func_key_str_eval(vm, uname, arg_count, args, err);
            }
            if (runtime_strcasecmp(uname, "KEYCOUNT") == 0 && arg_count >= 1) {
                return func_keycount_eval(vm, uname, arg_count, args, err);
            }
            if (runtime_strcasecmp(uname, "LOCKED") == 0 && arg_count >= 2) {
                return func_locked_eval(vm, uname, arg_count, args, err);
            }
        }


        if (open_delim == TOK_LBRACE) {
            if (runtime_strcasecmp(uname, "COUNT") == 0 || runtime_strcasecmp(uname, "COUNT$") == 0) {
                res.type = VAL_NUMBER;
                res.as.number = (double)arg_count;
                return res;
            }
            if (runtime_strcasecmp(uname, "MIN") == 0 && arg_count > 0) {
                double min_val = (args[0].type == VAL_NUMBER) ? args[0].as.number : 0.0;
                for (int i = 1; i < arg_count; i++) {
                    double v = (args[i].type == VAL_NUMBER) ? args[i].as.number : 0.0;
                    if (v < min_val) min_val = v;
                }
                res.type = VAL_NUMBER;
                res.as.number = min_val;
                return res;
            }
            if (runtime_strcasecmp(uname, "MAX") == 0 && arg_count > 0) {
                double max_val = (args[0].type == VAL_NUMBER) ? args[0].as.number : 0.0;
                for (int i = 1; i < arg_count; i++) {
                    double v = (args[i].type == VAL_NUMBER) ? args[i].as.number : 0.0;
                    if (v > max_val) max_val = v;
                }
                res.type = VAL_NUMBER;
                res.as.number = max_val;
                return res;
            }
            if ((runtime_strcasecmp(uname, "SUM") == 0 || runtime_strcasecmp(uname, "MATH.SUM") == 0) && arg_count > 0) {
                double total = 0.0;
                for (int i = 0; i < arg_count; i++) {
                    total += (args[i].type == VAL_NUMBER) ? args[i].as.number : 0.0;
                }
                res.type = VAL_NUMBER;
                res.as.number = total;
                return res;
            }
            if ((runtime_strcasecmp(uname, "AVG") == 0 || runtime_strcasecmp(uname, "MATH.AVG") == 0 ||
                 runtime_strcasecmp(uname, "MEAN") == 0 || runtime_strcasecmp(uname, "MATH.MEAN") == 0) && arg_count > 0) {
                double total = 0.0;
                for (int i = 0; i < arg_count; i++) {
                    total += (args[i].type == VAL_NUMBER) ? args[i].as.number : 0.0;
                }
                res.type = VAL_NUMBER;
                res.as.number = total / (double)arg_count;
                return res;
            }
            if (runtime_strcasecmp(uname, "ABS") == 0 && arg_count == 2) {
                double x = (args[0].type == VAL_NUMBER) ? args[0].as.number : 0.0;
                double y = (args[1].type == VAL_NUMBER) ? args[1].as.number : 0.0;
                res.type = VAL_NUMBER;
                res.as.number = runtime_hypot(x, y);
                return res;
            }
            if (runtime_strcasecmp(uname, "SQR") == 0 && arg_count == 2) {
                double r = (args[0].type == VAL_NUMBER) ? args[0].as.number : 0.0;
                double im = (args[1].type == VAL_NUMBER) ? args[1].as.number : 0.0;
                res.type = VAL_NUMBER;
                res.as.number = runtime_hypot(r, im);
                return res;
            }
        }
        res = eval_builtin_function_impl(vm, uname, arg_count, args, err);
    }

    if (err->code != 0) {
        for (int j = 0; j < arg_count; j++) {
            if (args[j].type == VAL_STRING && args[j].as.string) {
                str_release(vm_get_str(vm), args[j].as.string);
                args[j].as.string = NULL;
            } else if (args[j].type == VAL_MAP && args[j].as.map) {
                map_release(vm_get_str(vm), args[j].as.map);
                args[j].as.map = NULL;
            }
        }
    }

    return res;
}

BValue eval_builtin_function(VMContext *vm, const char *name, LexerContext *lex, bool has_parens, BppError *err) {
    return eval_builtin_function_delim(vm, name, lex, has_parens ? TOK_LPAREN : TOK_UNKNOWN, err);
}
