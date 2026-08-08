/* =====================================================================
 * What it does: Header declaration for the MICROPLEX$ builtin function evaluator.
 * Why it exists: Provides explicit header surface for func_microplex.c micro-library target.
 * Why it works this way: Declares func_microplex_eval function prototype.
 * What can be changed: Additional helper prototypes if expanded.
 * What cannot be changed: C17 header guards, self-sufficiency includes.
 * What to expect: Clean compilation when included independently.
 * What to do if something breaks: Check included header paths relative to engine/include/.
 * Assumptions: VMContext, BValue, and BppError types are defined.
 * Portability concerns: Strict C17 compliant.
 * Future expansions: Multi-argument microplex function declarations.
 * ===================================================================== */

#ifndef EVAL_FUNC_MICROPLEX_H
#define EVAL_FUNC_MICROPLEX_H

#include "vm/vm.h"
#include "eval/eval.h"

BValue func_microplex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif /* EVAL_FUNC_MICROPLEX_H */
