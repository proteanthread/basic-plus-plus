// FILENAME: eval_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements component functionality for eval_internal.h.
//
// ---- Includes ----

// FILENAME: eval_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (string.h)
// NEEDED BY: libcore (microplex.c)
// NEEDED BY: libengine (abs.h, acos.h, and.h, asin.h, ast_internal.h, atan2.h)
// NEEDED BY: libengine (atn.h, bin.h, bitcount.h, category.h, chr.h, clamp.h)
// NEEDED BY: libengine (clock_num.h, clock_str.h, comp.h, complex_fn.h, cos.h)
// NEEDED BY: libengine (cosh.h, cot.h, csc.h, csrlin.h, date.h)
// NEEDED BY: libengine (dispatch_internal.h, environ.h, eof_fn.h, eqv.h, erl.h)
// NEEDED BY: libengine (err_fn.h, eval_expr_internal.h, exec_internal.h, exp.h)
// NEEDED BY: libengine (fix.h, floor.h, fre.h, freefile.h, hex.h, hypot.h)
// NEEDED BY: libengine (imp.h, inkey.h, inp.h, instr.h, int.h, lbound.h)
// NEEDED BY: libengine (lcase.h, left.h, len.h, lerp.h, loc_fn.h, lof.h, log.h)
// NEEDED BY: libengine (log10.h, log2.h, lpos.h, ltrim.h, max.h, mid.h, min.h)
// NEEDED BY: libengine (mod.h, not.h, oct.h, or.h, pdif.h, peek.h, pi.h)
// NEEDED BY: libengine (point_fn.h, pos.h, readbit.h, resetbit.h, right.h)
// NEEDED BY: libengine (rnd.h, round.h, rtrim.h, sec.h, setbit.h, sgn.h, shl.h)
// NEEDED BY: libengine (shr.h, shuffle.h, sin.h, sinh.h, space.h, spc.h, sqr.h)
// NEEDED BY: libengine (str.h, tab.h, tan.h, tanh.h, ticks.h, time.h, timer.h)
// NEEDED BY: libengine (togglebit.h, trim.h, ubound.h, ucase.h, val.h)
// NEEDED BY: libengine (verify_fn.h, xor.h)
// NEEDED BY: libengine (ast_eval_expr.c, ast_parse_block.c, ast_parse_expr.c)
// NEEDED BY: libengine (ast_parse_stmt.c, category.c, eval_builtins.c, help.c)
// NEEDED BY: libengine (helpers.c, ops.c, rpn.c)
// NEEDS: libcore (algebra.h, alloc.h, basic.h, calendar.h, ctype.h)
// NEEDS: libcore (float_parse.h, hal.h, memops.h, num_parse.h, snprintf.h)
// NEEDS: libcore (strings.h, strops.h, trig.h)
// NEEDS: libcore (algebra.c, alloc.c, basic.c, calendar.c, ctype.c)
// NEEDS: libcore (float_parse.c, memops.c, num_parse.c, snprintf.c, strings.c)
// NEEDS: libcore (strops.c, trig.c)
// NEEDS: libengine (eval.h, lexer.h, vm.h)
// NEEDS: libengine (eval.c, lexer.c)
// NEEDS: libkernel (types.h)
// NEEDS: libplatform (platform.h)
// Provides core logic and interface definitions for eval_internal within BASIC++.
//
// ---- Includes ----

#ifndef EVAL_INTERNAL_H
#define EVAL_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/ctype/ctype.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/conv/num_parse.h"
#include "runtime/conv/float_parse.h"
#include "runtime/format/snprintf.h"
#include "runtime/math/basic.h"
#include "runtime/math/trig.h"
#include "runtime/math/algebra.h"
#include "runtime/memory/alloc.h"
#include "runtime/time/calendar.h"
#include "hal/hal.h"

#include "eval/eval.h"
#include "platform/platform.h"
#include "types/types.h"
#include "lexer/lexer.h"
#include "vm/vm.h"
#include "runtime/strings.h"

// ===================================================================
// Constants
// ===================================================================

#ifndef MAX_EVAL_DEPTH
#define MAX_EVAL_DEPTH 128
#endif

// ===================================================================
// Shared Helper Functions (eval_helpers.c)
// ===================================================================

static inline bool eval_is_zero_arg_builtin_function(const char *name) {
    if (!name) return false;
    char uname[64];
    size_t len = runtime_strlen(name);
    if (len >= sizeof(uname)) len = sizeof(uname) - 1;
    for (size_t i = 0; i < len; i++) uname[i] = (char)runtime_toupper((unsigned char)name[i]);
    uname[len] = '\0';
    return (runtime_strcmp(uname, "INKEY$") == 0 || runtime_strcmp(uname, "INKEY") == 0 ||
            runtime_strcmp(uname, "DATE$") == 0 || runtime_strcmp(uname, "TIME$") == 0 ||
            runtime_strcmp(uname, "TIMER") == 0 || runtime_strcmp(uname, "CSRLIN") == 0 ||
            runtime_strcmp(uname, "RND") == 0 || runtime_strcmp(uname, "ERR") == 0 ||
            runtime_strcmp(uname, "ERL") == 0 || runtime_strcmp(uname, "FREEFILE") == 0 ||
            runtime_strcmp(uname, "COMMAND$") == 0 || runtime_strcmp(uname, "COMMAND") == 0 ||
            runtime_strcmp(uname, "DOEVENTS") == 0 ||
            runtime_strcmp(uname, "PI") == 0 || runtime_strcmp(uname, "_PI") == 0 || runtime_strcmp(uname, "MATH.PI") == 0 ||
            runtime_strcmp(uname, "INF") == 0 || runtime_strcmp(uname, "_INF") == 0 || runtime_strcmp(uname, "MATH.INF") == 0 || runtime_strcmp(uname, "INFINITY") == 0 ||
            runtime_strcmp(uname, "MAXNUM") == 0 || runtime_strcmp(uname, "_MAXNUM") == 0 || runtime_strcmp(uname, "MATH.MAXNUM") == 0 ||
            runtime_strcmp(uname, "EPS") == 0 || runtime_strcmp(uname, "_EPS") == 0 || runtime_strcmp(uname, "MATH.EPS") == 0 ||
            runtime_strcmp(uname, "TRUE") == 0 || runtime_strcmp(uname, "FALSE") == 0 ||
            runtime_strcmp(uname, "TICKS") == 0 || runtime_strcmp(uname, "TI") == 0 || runtime_strcmp(uname, "TI$") == 0 ||
            runtime_strcmp(uname, "DATE") == 0 || runtime_strcmp(uname, "TIME") == 0 || runtime_strcmp(uname, "CLOCK") == 0 || runtime_strcmp(uname, "CLOCK$") == 0 ||
            runtime_strcmp(uname, "TZ") == 0 || runtime_strcmp(uname, "TZ$") == 0 || runtime_strcmp(uname, "TIMEZONE$") == 0 || runtime_strcmp(uname, "UTC") == 0 || runtime_strcmp(uname, "JIFFIES") == 0 ||
            runtime_strcmp(uname, "HOSTNAME$") == 0 || runtime_strcmp(uname, "USERNAME$") == 0 ||
            runtime_strcmp(uname, "BASEDIR$") == 0 || runtime_strcmp(uname, "BASEPATH$") == 0 || runtime_strcmp(uname, "BASENAME$") == 0 ||
            runtime_strcmp(uname, "PATH$") == 0 || runtime_strcmp(uname, "ERR$") == 0 ||
            runtime_strcmp(uname, "RECOUNT") == 0 || runtime_strcmp(uname, "STATUS") == 0 ||
            runtime_strcmp(uname, "TIM") == 0 || runtime_strcmp(uname, "HRS") == 0 || runtime_strcmp(uname, "HOURS") == 0 ||
            runtime_strcmp(uname, "MIN") == 0 || runtime_strcmp(uname, "MINUTES") == 0 ||
            runtime_strcmp(uname, "SEC") == 0 || runtime_strcmp(uname, "SECONDS") == 0 ||
            runtime_strcmp(uname, "JULIAN") == 0 || runtime_strcmp(uname, "JULIAN$") == 0 || runtime_strcmp(uname, "DAT") == 0 ||
            runtime_strcmp(uname, "DAY") == 0 || runtime_strcmp(uname, "MONTH") == 0 || runtime_strcmp(uname, "YEAR") == 0 ||
            runtime_strcmp(uname, "DAY$") == 0 || runtime_strcmp(uname, "MONTH$") == 0 ||
            runtime_strcmp(uname, "CURDIR$") == 0 || runtime_strcmp(uname, "CURDIR") == 0 ||
            runtime_strcmp(uname, "PREFIX$") == 0 || runtime_strcmp(uname, "PREFIX") == 0 ||
            runtime_strcmp(uname, "MODDIR$") == 0 || runtime_strcmp(uname, "MODDIR") == 0 ||
            runtime_strcmp(uname, "DIR$") == 0 || runtime_strcmp(uname, "DIR") == 0 ||
            runtime_strcmp(uname, "FRE") == 0 || runtime_strcmp(uname, "UNIXTIME") == 0 ||
            runtime_strcmp(uname, "POS") == 0 || runtime_strcmp(uname, "LPOS") == 0);
}


// @brief Parse a Sinclair/Atari string slice construct from the token stream.
BValue eval_parse_string_slice(VMContext *vm, LexerContext *lex,
                               const char *var_name, BppTokenType open_tok,
                               BppError *out_err);

// @brief Split a dotted identifier into base variable and member chain.
void eval_split_member_chain(const char *start, size_t len,
                             char *var_name, size_t var_name_max,
                             char member_chain[8][64], int *member_count);

// @brief Get the precedence of an operator token type.
int eval_get_precedence(BppTokenType type);

// @brief Check if top-of-stack operator has precedence over new operator.
bool eval_has_precedence(VMContext *vm, BppTokenType top, BppTokenType op);

// @brief Check if a token type is an operator.
bool eval_is_operator(BppTokenType type);

// @brief Round a double to a specified number of significant digits.
double eval_round_to_decimal(double val, int precision);

// @brief Format a double to clean string representation.
void eval_format_double_clean(char *buf, size_t buf_size, double val,
                              bool leading_space, bool trailing_space);

// ===================================================================
// Operator Execution (eval_ops.c)
// ===================================================================

// @brief Execute a binary or unary operator on the value stack.
// @return true on success, false on error (err populated).
bool eval_execute_op(VMContext *vm, BppTokenType op,
                     BValue *val_stack, size_t *val_ptr, BppError *err);

// @brief Resolve member access chain on a value (dot notation).
BValue eval_resolve_member_access(VMContext *vm, LexerContext *lex,
                                  BValue val, BppError *out_err);

// ===================================================================
// Builtin Dispatch (eval_dispatch.c)
// ===================================================================

// @brief Check if a name is a known builtin function.
bool eval_is_builtin_function(const char *name);

// @brief Check if a token is a keyword representing a built-in function or operand.
bool eval_is_builtin_function_tok(BppToken tok);

// @brief Read a file's entire contents into a malloc'd string.
// @return Heap-allocated NUL-terminated string, or NULL on failure. Caller frees.
char *eval_read_file_to_string(const char *path);

// @brief Parse arguments from token stream and dispatch to builtin implementation.
BValue eval_builtin_function(VMContext *vm, const char *name,
                             LexerContext *lex, bool has_parens,
                             BppError *err);

// ===================================================================
// Builtin Implementation (eval_builtins.c)
// ===================================================================

// @brief Master dispatch for builtin function evaluation by uppercase name.
BValue eval_builtin_function_impl(VMContext *vm, const char *uname,
                                  int arg_count, BValue *args, BppError *err);

// @brief CRC-16 computation helper for BIOCHECKSUM.
uint16_t eval_compute_crc16(const unsigned char *data, size_t len);

// @brief RPN expression evaluator (reverse Polish notation).
BValue eval_expression_rpn(VMContext *vm, LexerContext *lex, BppError *out_err);

// ===================================================================
// Shared Global State
// ===================================================================

extern BppDirSearch *g_eval_dir_search;

// ===================================================================
// External Platform/VM Functions used across eval modules
// ===================================================================

extern bool find_procedure(struct VMContext *vm, const char *name,
                           BppKeywordId proc_kw, double *out_line,
                           const char **out_text);
extern int platform_inkey_char(void);
extern int platform_mouse_x(void);
extern int platform_mouse_y(void);
extern int platform_mouse_btn(void);
extern struct tm *platform_localtime(const time_t *timep, struct tm *result);
extern double vm_get_last_rnd(VMContext *vm);
extern void vm_set_last_rnd(VMContext *vm, double val);
extern double platform_get_timer(void);
extern double platform_get_uptime(void);
extern double vm_get_ti_offset(VMContext *vm);
extern void vm_set_ti_offset(VMContext *vm, double val);
extern void hash_string(const char *algo, const char *data,
                            char *out_buf, size_t out_size);

#endif // EVAL_INTERNAL_H
