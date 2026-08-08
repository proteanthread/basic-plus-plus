/**
 * @file def.c
 * @brief DEFINT, DEFSNG, DEFDBL, DEFSTR implicit variable type default statement handlers for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements DEFINT, DEFSNG, DEFDBL, and DEFSTR letter-range statement handlers (e.g. DEFINT A-Z, DEFSTR S).
 *
 * 2. WHY IT EXISTS:
 * Sets default implicit variable data types based on variable name starting letters per GW-BASIC and QBASIC standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Parses letter ranges (e.g., A-D, F, H-Z) and updates the letter-to-type map in the active VM scope context.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_def'. Includes "stmt/stmt.h",
 * "lexer/lexer.h", "vm/vm.h", "security/security.h", "device/vdev.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (Core Included).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support DEFOBJ or DEFBYTE if adding novel implicit primitive data types.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Scope letter map array bounds (26 slots for 'A'..'Z'): Letter range parsing MUST update 0-25 indexing correctly.
 *
 * 8. WHAT TO EXPECT:
 * Updates VM implicit type table for scope and returns ERR_NONE.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check case insensitivity ('a'-'z' normalized to 'A'-'Z') and range upper bounds.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Standard ASCII letter code arithmetic ('A' + offset).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/vm_context.c
 * - engine/src/lexer/lexer.c
 * Prerequisite Header Files:
 * - engine/include/statements/oop/def.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "stmt/stmt.h"
#include "lexer/lexer.h"
#include "vm/vm.h"
#include "runtime/variables.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <ctype.h>

static BppError parse_def_range(VMContext *vm, LexerContext *lex, ValueType type) {
    BppError err;
    memset(&err, 0, sizeof(err));
    VariableContext *var = vm_get_var(vm);
    const char *scope = vm_get_active_proc(vm);

    char var_names[16][64];
    int num_vars = 0;

    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected variable name or single letter (A-Z) in DEF";
            return err;
        }

        char name[64];
        size_t clen = (tok.length < 63) ? tok.length : 63;
        memcpy(name, tok.start, clen);
        name[clen] = '\0';
        
        for (size_t i = 0; i < clen; i++) {
            name[i] = (char)toupper((unsigned char)name[i]);
        }
        
        /* Append type suffix if not already present to force explicit declaration */
        char last = name[clen - 1];
        if (last != '$' && last != '%' && last != '&' && last != '!' && last != '#') {
            if (type == VAL_STRING) name[clen] = '$';
            else if (type == VAL_INTEGER) name[clen] = '%';
            else name[clen] = '!';
            name[clen + 1] = '\0';
        }

        if (num_vars < 16) {
            snprintf(var_names[num_vars], sizeof(var_names[num_vars]), "%s", name);
            num_vars++;
        }

        BppToken next = lex_peek(lex);
        if (next.type == TOK_MINUS) {
            if (strlen(name) != 2 || !isalpha((unsigned char)name[0])) {
                err.code = 2; err.message = "Expected single letter (A-Z) before '-' in DEF range";
                return err;
            }
            lex_next(lex); /* Consume '-' */
            BppToken end_tok = lex_next(lex);
            if (end_tok.type != TOK_IDENT || end_tok.length != 1 || !isalpha((unsigned char)end_tok.start[0])) {
                err.code = 2; err.message = "Expected single letter (A-Z) after '-' in DEF range";
                return err;
            }
            char end_letter = (char)toupper((unsigned char)end_tok.start[0]);
            var_set_def_type(var, scope, name[0], end_letter, type);
        } else {
            /* If length 2 (1 char + 1 suffix), also define implicit type for that letter for legacy support */
            if (clen == 1 && isalpha((unsigned char)name[0])) {
                var_set_def_type(var, scope, name[0], name[0], type);
            }
            /* Explicitly declare the variable so it has the correct type regardless of implicit scope */
            BValue *v = var_declare(var, name);
            if (v) {
                if (v->type == VAL_STRING) str_release(vm_get_str(vm), v->as.string);
                v->type = type;
                if (type == VAL_STRING) v->as.string = NULL;
                else v->as.number = 0.0;
            }
        }

        next = lex_peek(lex);
        if (next.type == TOK_COMMA) {
            lex_next(lex); /* Consume ',' */
        } else {
            break;
        }
    }
    
    /* Check for assignment (e.g. DEFINT A, B, C = 10) */
    BppToken next = lex_peek(lex);
    if (next.type == TOK_EQ) {
        lex_next(lex); /* Consume '=' */
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        
        for (int i = 0; i < num_vars; i++) {
            /* Create a copy if string */
            BValue assign_val = val;
            if (assign_val.type == VAL_STRING && assign_val.as.string) {
                str_add_ref(assign_val.as.string);
            }
            
            bool ok = var_assign(var, var_names[i], assign_val);
            if (!ok) {
                err.code = 13; err.message = "Type mismatch in DEF variable assignment";
                /* We don't break early to ensure string refs are eventually managed, but we return error */
            }
        }
        
        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }
    }

    return err;
}

BppError stmt_defint_handler(VMContext *vm, LexerContext *lex) {
    return parse_def_range(vm, lex, VAL_INTEGER);
}

BppError stmt_defsng_handler(VMContext *vm, LexerContext *lex) {
    return parse_def_range(vm, lex, VAL_NUMBER);
}

BppError stmt_defdbl_handler(VMContext *vm, LexerContext *lex) {
    return parse_def_range(vm, lex, VAL_NUMBER);
}

BppError stmt_defstr_handler(VMContext *vm, LexerContext *lex) {
    return parse_def_range(vm, lex, VAL_STRING);
}

BppError stmt_defusr_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int usr_idx = 0;
    BppToken tok = lex_peek(lex);

    if (tok.type == TOK_NUMBER) {
        lex_next(lex);
        usr_idx = (int)tok.as.number;
    } else if (tok.type == TOK_IDENT) {
        if (tok.length >= 4 && (tok.start[0] == 'U' || tok.start[0] == 'u') &&
            (tok.start[1] == 'S' || tok.start[1] == 's') &&
            (tok.start[2] == 'R' || tok.start[2] == 'r') &&
            isdigit((unsigned char)tok.start[3])) {
            usr_idx = tok.start[3] - '0';
            lex_next(lex);
        }
    }

    if (usr_idx < 0 || usr_idx > 9) {
        err.code = 5; err.message = "Illegal function call: USR index must be 0 to 9";
        return err;
    }

    BppToken eq = lex_peek(lex);
    if (eq.type == TOK_EQ) {
        lex_next(lex); /* Consume '=' */
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "DEF USR address must be numeric";
        return err;
    }

    vm_set_usr_ptr(vm, usr_idx, (uintptr_t)val.as.number);
    return err;
}

void stmt_def_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DEFINT",
        .category = "Variables & Memory",
        .syntax = "DEFINT | DEFSNG | DEFDBL | DEFSTR letter_range [, letter_range...]",
        .help_text = "Sets the default implicit data type for variables starting with specified letters.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

