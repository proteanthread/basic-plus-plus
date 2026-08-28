// FILENAME: eval_new.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (eval_expr_internal.h)
// Provides core logic and interface definitions for eval_new within BASIC++.
//
// ---- Includes ----

#include "eval/eval_expr_internal.h"

//
// ---- NEW Expression Parsing and Instantiation ----

bool eval_parse_new_expression(VMContext *vm, LexerContext *lex, BValue *out_val, BppError *out_err) {
    if (!vm || !lex || !out_val || !out_err) return false;

    BppToken class_tok = lex_next(lex);
    if (class_tok.type != TOK_IDENT && class_tok.type != TOK_KEYWORD) {
        out_err->code = 2;
        out_err->message = "Expected class name after NEW";
        return false;
    }

    char class_name[64] = {0};
    size_t c_len = (class_tok.length < sizeof(class_name) - 1) ? class_tok.length : sizeof(class_name) - 1;
    runtime_memcpy(class_name, class_tok.start, c_len);
    class_name[c_len] = '\0';

    char err_buf[128] = {0};
    BppMap *inst = struct_instantiate(vm, vm_get_types(vm), class_name, err_buf, sizeof(err_buf));
    if (!inst) {
        out_err->code = 5;
        out_err->message = "Unknown class or out of memory in NEW";
        return false;
    }

    // Parse optional constructor arguments: NEW ClassName(arg1, arg2)
    BValue constr_args[10];
    int constr_argc = 0;
    constr_args[constr_argc++] = (BValue){.type = VAL_MAP, .as.map = inst};

    BppToken paren_tok = lex_peek(lex);
    if (paren_tok.type == TOK_LPAREN) {
        lex_next(lex); // Consume '('
        while (lex_peek(lex).type != TOK_RPAREN && lex_peek(lex).type != TOK_EOF && lex_peek(lex).type != TOK_EOL) {
            if (constr_argc < 10) {
                BValue c_arg = eval_expression(vm, lex, out_err);
                if (out_err->code != 0) {
                    map_release(vm_get_str(vm), inst);
                    return false;
                }
                constr_args[constr_argc++] = c_arg;
            }
            BppToken sep = lex_peek(lex);
            if (sep.type == TOK_COMMA) lex_next(lex);
            else break;
        }
        if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
    }

    // If constructor exists, invoke it: ClassName.CONSTRUCTOR, ClassName.NEW, ClassName.INIT, or ClassName.Class_Initialize
    char constr_name[128];
    runtime_snprintf(constr_name, sizeof(constr_name), "%s.CONSTRUCTOR", class_name);
    BppError c_err;
    runtime_memset(&c_err, 0, sizeof(c_err));
    BppLineNumber target_line = 0;
    const char *target_text = NULL;
    bool is_lib = false;

    if (find_procedure_ex(vm, constr_name, KW_CONSTRUCTOR, &target_line, &target_text, &is_lib) ||
        find_procedure_ex(vm, constr_name, KW_SUB, &target_line, &target_text, &is_lib) ||
        find_procedure_ex(vm, constr_name, KW_FUNCTION, &target_line, &target_text, &is_lib)) {
        invoke_user_function(vm, constr_name, constr_args, constr_argc, &c_err);
    } else {
        runtime_snprintf(constr_name, sizeof(constr_name), "%s.NEW", class_name);
        if (find_procedure_ex(vm, constr_name, KW_SUB, &target_line, &target_text, &is_lib) ||
            find_procedure_ex(vm, constr_name, KW_FUNCTION, &target_line, &target_text, &is_lib)) {
            invoke_user_function(vm, constr_name, constr_args, constr_argc, &c_err);
        } else {
            runtime_snprintf(constr_name, sizeof(constr_name), "%s.Class_Initialize", class_name);
            if (find_procedure_ex(vm, constr_name, KW_SUB, &target_line, &target_text, &is_lib) ||
                find_procedure_ex(vm, constr_name, KW_FUNCTION, &target_line, &target_text, &is_lib)) {
                invoke_user_function(vm, constr_name, constr_args, constr_argc, &c_err);
            } else {
                runtime_snprintf(constr_name, sizeof(constr_name), "%s.INIT", class_name);
                if (find_procedure_ex(vm, constr_name, KW_SUB, &target_line, &target_text, &is_lib) ||
                    find_procedure_ex(vm, constr_name, KW_FUNCTION, &target_line, &target_text, &is_lib)) {
                    invoke_user_function(vm, constr_name, constr_args, constr_argc, &c_err);
                }
            }
        }
    }


    out_val->type = VAL_MAP;
    out_val->as.map = inst;
    return true;
}
