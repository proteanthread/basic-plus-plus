// FILENAME: sub_lookup.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (sub_internal.h)
// Provides runtime implementation for the SUB_LOOKUP statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/sub_internal.h"

//
// ---- Procedure Lookup ----

bool find_procedure_ex(VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text, bool *out_is_lib) {
    if (!vm || !name) return false;

    MemoryContext *mem = vm_get_mem(vm);
    char mod_prefix[128] = {0};
    char pure_name[128] = {0};
    const char *dot = strchr(name, '.');
    if (dot) {
        size_t mlen = (size_t)(dot - name);
        if (mlen < sizeof(mod_prefix)) {
            memcpy(mod_prefix, name, mlen);
            mod_prefix[mlen] = '\0';
        }
        strncpy(pure_name, dot + 1, sizeof(pure_name) - 1);
    } else {
        strncpy(pure_name, name, sizeof(pure_name) - 1);
    }

    for (int pass = 0; pass < 2; pass++) {
        size_t count = 0;
        BppProgramLine *lines = (pass == 0) ? mem_program_get_all(mem, &count) : mem_lib_program_get_all(mem, &count);
        if (!lines || count == 0) continue;

        char cur_mod[128] = {0};
        char cur_class[128] = {0};

        for (size_t i = 0; i < count; ++i) {
            LexerContext *scan_lex = lex_init(mem, lines[i].text);
            if (!scan_lex) continue;
            BppToken tok = lex_next(scan_lex);
            if (tok.type == TOK_NUMBER) {
                tok = lex_next(scan_lex);
            }

            if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_MODULE) {
                BppToken m_tok = lex_next(scan_lex);
                if (m_tok.type == TOK_IDENT || m_tok.type == TOK_KEYWORD) {
                    size_t len = (m_tok.length < sizeof(cur_mod) - 1) ? m_tok.length : sizeof(cur_mod) - 1;
                    memcpy(cur_mod, m_tok.start, len);
                    cur_mod[len] = '\0';
                }
                lex_shutdown(scan_lex);
                continue;
            } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_CLASS) {
                BppToken c_tok = lex_next(scan_lex);
                if (c_tok.type == TOK_IDENT || c_tok.type == TOK_KEYWORD) {
                    size_t len = (c_tok.length < sizeof(cur_class) - 1) ? c_tok.length : sizeof(cur_class) - 1;
                    memcpy(cur_class, c_tok.start, len);
                    cur_class[len] = '\0';
                }
                lex_shutdown(scan_lex);
                continue;
            } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
                BppToken ntok = lex_peek(scan_lex);
                if (ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_MODULE) {
                    cur_mod[0] = '\0';
                    lex_shutdown(scan_lex);
                    continue;
                } else if (ntok.type == TOK_KEYWORD && ntok.as.keyword == KW_CLASS) {
                    cur_class[0] = '\0';
                    lex_shutdown(scan_lex);
                    continue;
                }
            }

            if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_PUBLIC || tok.as.keyword == KW_PRIVATE ||
                                            tok.as.keyword == KW_STATIC || tok.as.keyword == KW_SHARED)) {
                tok = lex_next(scan_lex);
            }

            if (tok.type == TOK_KEYWORD &&
                (tok.as.keyword == proc_kw ||
                 (proc_kw == KW_SUB && (tok.as.keyword == KW_PROCEDURE || tok.as.keyword == KW_PROPERTY || tok.as.keyword == KW_CONSTRUCTOR)) ||
                 (proc_kw == KW_FUNCTION && tok.as.keyword == KW_DEF))) {

                BppToken name_tok = lex_next(scan_lex);
                char candidate[128] = {0};
                if ((name_tok.type == TOK_KEYWORD && name_tok.as.keyword == KW_OPERATOR) ||
                    (name_tok.type == TOK_IDENT && name_tok.length == 8 && strncasecmp(name_tok.start, "OPERATOR", 8) == 0)) {
                    BppToken op_tok = lex_next(scan_lex);
                    char op_str[32] = {0};
                    if (op_tok.type == TOK_KEYWORD || op_tok.type == TOK_IDENT) {
                        size_t olen = (op_tok.length < sizeof(op_str) - 1) ? op_tok.length : sizeof(op_str) - 1;
                        memcpy(op_str, op_tok.start, olen);
                        op_str[olen] = '\0';
                    } else if (op_tok.type == TOK_PLUS) {
                        strcpy(op_str, "+");
                    } else if (op_tok.type == TOK_MINUS) {
                        strcpy(op_str, "-");
                    } else if (op_tok.type == TOK_MUL) {
                        strcpy(op_str, "*");
                    } else if (op_tok.type == TOK_DIV) {
                        strcpy(op_str, "/");
                    } else if (op_tok.type == TOK_EQ) {
                        strcpy(op_str, "=");
                    } else if (op_tok.type == TOK_LT) {
                        if (lex_peek(scan_lex).type == TOK_GT) {
                            lex_next(scan_lex);
                            strcpy(op_str, "<>");
                        } else if (lex_peek(scan_lex).type == TOK_EQ) {
                            lex_next(scan_lex);
                            strcpy(op_str, "<=");
                        } else {
                            strcpy(op_str, "<");
                        }
                    } else if (op_tok.type == TOK_GT) {
                        if (lex_peek(scan_lex).type == TOK_EQ) {
                            lex_next(scan_lex);
                            strcpy(op_str, ">=");
                        } else {
                            strcpy(op_str, ">");
                        }
                    } else if (op_tok.start && op_tok.length > 0) {
                        size_t olen = (op_tok.length < sizeof(op_str) - 1) ? op_tok.length : sizeof(op_str) - 1;
                        memcpy(op_str, op_tok.start, olen);
                        op_str[olen] = '\0';
                    }
                    snprintf(candidate, sizeof(candidate), "OPERATOR_%s", op_str);
                } else if (name_tok.type == TOK_IDENT || name_tok.type == TOK_KEYWORD) {
                    size_t len = (name_tok.length < sizeof(candidate) - 1) ? name_tok.length : sizeof(candidate) - 1;
                    memcpy(candidate, name_tok.start, len);
                    candidate[len] = '\0';
                }

                if (candidate[0] != '\0') {
                    while (lex_peek(scan_lex).type == TOK_PERIOD) {
                        lex_next(scan_lex);
                        BppToken sub_tok = lex_next(scan_lex);
                        if (sub_tok.type != TOK_IDENT && sub_tok.type != TOK_KEYWORD) break;
                        char sub_part[64];
                        size_t slen = (sub_tok.length < sizeof(sub_part) - 1) ? sub_tok.length : sizeof(sub_part) - 1;
                        memcpy(sub_part, sub_tok.start, slen);
                        sub_part[slen] = '\0';

                        char combined[128];
                        snprintf(combined, sizeof(combined), "%s.%s", candidate, sub_part);
                        strncpy(candidate, combined, sizeof(candidate) - 1);
                        candidate[sizeof(candidate) - 1] = '\0';
                    }

                    bool matches = false;
                    if (strcasecmp(candidate, name) == 0) {
                        matches = true;
                    } else if (strncmp(candidate, "OPERATOR_", 9) == 0 && strcasecmp(candidate + 9, name) == 0) {
                        matches = true;
                    } else if (mod_prefix[0] != '\0' && cur_mod[0] != '\0' &&
                               strcasecmp(mod_prefix, cur_mod) == 0 &&
                               strcasecmp(pure_name, candidate) == 0) {
                        matches = true;
                    } else if (mod_prefix[0] != '\0' && cur_class[0] != '\0' &&
                               strcasecmp(mod_prefix, cur_class) == 0 &&
                               strcasecmp(pure_name, candidate) == 0) {
                        matches = true;
                    } else if (mod_prefix[0] == '\0' && cur_class[0] != '\0') {
                        char full_c_name[128];
                        snprintf(full_c_name, sizeof(full_c_name), "%s.%s", cur_class, candidate);
                        if (strcasecmp(full_c_name, name) == 0) {
                            matches = true;
                        }
                    }

                    if (matches) {
                        if (out_line) *out_line = lines[i].line_number;
                        if (out_text) *out_text = lines[i].text;
                        if (out_is_lib) *out_is_lib = (pass == 1);
                        lex_shutdown(scan_lex);
                        return true;
                    }
                }
            }
            lex_shutdown(scan_lex);
        }
    }
    return false;
}

bool find_procedure(VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text) {
    return find_procedure_ex(vm, name, proc_kw, out_line, out_text, NULL);
}

//
// ---- Formal Parameter Parsing ----

int parse_formal_params(MemoryContext *mem, const char *line_text, FormalParam *params, int max_params) {
    if (!mem || !line_text || !params || max_params <= 0) return 0;

    LexerContext *lex = lex_init(mem, line_text);
    if (!lex) return 0;

    BppToken tok = lex_next(lex);
    if (tok.type == TOK_NUMBER) tok = lex_next(lex);
    if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_PUBLIC || tok.as.keyword == KW_PRIVATE ||
                                    tok.as.keyword == KW_STATIC || tok.as.keyword == KW_SHARED)) {
        tok = lex_next(lex);
    }
    if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_SUB || tok.as.keyword == KW_FUNCTION ||
                                    tok.as.keyword == KW_PROCEDURE || tok.as.keyword == KW_PROPERTY ||
                                    tok.as.keyword == KW_CONSTRUCTOR || tok.as.keyword == KW_DEF)) {
        tok = lex_next(lex);
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_OPERATOR) ||
            (tok.type == TOK_IDENT && tok.length == 8 && strncasecmp(tok.start, "OPERATOR", 8) == 0)) {
            tok = lex_next(lex);
        }
        while (lex_peek(lex).type == TOK_PERIOD) {
            lex_next(lex);
            lex_next(lex);
        }
    }

    tok = lex_next(lex);
    if (tok.type != TOK_LPAREN) {
        lex_shutdown(lex);
        return 0;
    }

    int count = 0;
    while (count < max_params) {
        tok = lex_peek(lex);
        if (tok.type == TOK_RPAREN || tok.type == TOK_EOF || tok.type == TOK_EOL) break;

        bool is_byref = true;
        bool is_optional = false;
        if (tok.type == TOK_KEYWORD) {
            if (tok.as.keyword == KW_BYVAL) {
                is_byref = false;
                lex_next(lex);
            } else if (tok.as.keyword == KW_BYREF) {
                is_byref = true;
                lex_next(lex);
            } else if (tok.as.keyword == KW_OPTIONAL) {
                is_optional = true;
                lex_next(lex);
                BppToken n = lex_peek(lex);
                if (n.type == TOK_KEYWORD && n.as.keyword == KW_BYVAL) {
                    is_byref = false;
                    lex_next(lex);
                } else if (n.type == TOK_KEYWORD && n.as.keyword == KW_BYREF) {
                    is_byref = true;
                    lex_next(lex);
                }
            }
        }

        tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) break;

        size_t len = (tok.length < sizeof(params[count].name) - 1) ? tok.length : sizeof(params[count].name) - 1;
        memcpy(params[count].name, tok.start, len);
        params[count].name[len] = '\0';
        params[count].is_byref = is_byref;
        params[count].is_optional = is_optional;
        params[count].is_array = false;
        params[count].type_name[0] = '\0';
        params[count].default_expr[0] = '\0';

        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex);
            if (lex_peek(lex).type == TOK_RPAREN) {
                lex_next(lex);
                params[count].is_array = true;
            }
        }

        if (lex_peek(lex).type == TOK_KEYWORD && lex_peek(lex).as.keyword == KW_AS) {
            lex_next(lex);
            BppToken type_tok = lex_next(lex);
            if (type_tok.type == TOK_IDENT || type_tok.type == TOK_KEYWORD) {
                size_t tlen = (type_tok.length < sizeof(params[count].type_name) - 1) ? type_tok.length : sizeof(params[count].type_name) - 1;
                memcpy(params[count].type_name, type_tok.start, tlen);
                params[count].type_name[tlen] = '\0';
            }
        }

        if (lex_peek(lex).type == TOK_EQ) {
            lex_next(lex);
            const char *expr_start = lex_get_pos(lex);
            int paren_depth = 0;
            while (true) {
                BppToken dtok = lex_peek(lex);
                if (dtok.type == TOK_EOF || dtok.type == TOK_EOL) break;
                if (dtok.type == TOK_LPAREN) paren_depth++;
                else if (dtok.type == TOK_RPAREN) {
                    if (paren_depth == 0) break;
                    paren_depth--;
                } else if (dtok.type == TOK_COMMA && paren_depth == 0) {
                    break;
                }
                lex_next(lex);
            }
            const char *expr_end = lex_get_pos(lex);
            size_t elen = (size_t)(expr_end - expr_start);
            if (elen > sizeof(params[count].default_expr) - 1) elen = sizeof(params[count].default_expr) - 1;
            memcpy(params[count].default_expr, expr_start, elen);
            params[count].default_expr[elen] = '\0';
            params[count].is_optional = true;
        }

        count++;
        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    lex_shutdown(lex);
    return count;
}

//
// ---- Call Argument Parsing ----

int parse_call_args(VMContext *vm, LexerContext *lex, FormalParam *formal_params, int param_count,
                    BValue *out_args, char out_arg_names[][64], bool *out_is_byref, int max_args, BppError *out_err) {
    if (!vm || !lex || !out_args || max_args <= 0 || !out_err) return 0;

    int count = 0;
    bool has_parens = false;

    if (lex_peek(lex).type == TOK_LPAREN) {
        has_parens = true;
        lex_next(lex);
    }

    while (count < max_args) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOF || tok.type == TOK_EOL) break;
        if (has_parens && tok.type == TOK_RPAREN) break;

        bool byref_target = (count < param_count) ? formal_params[count].is_byref : true;
        bool is_array_param = (count < param_count) ? formal_params[count].is_array : false;

        out_arg_names[count][0] = '\0';
        out_is_byref[count] = false;

        if (tok.type == TOK_IDENT) {
            size_t nlen = (tok.length < 63) ? tok.length : 63;
            memcpy(out_arg_names[count], tok.start, nlen);
            out_arg_names[count][nlen] = '\0';

            if (is_array_param) {
                lex_next(lex);
                if (lex_peek(lex).type == TOK_LPAREN) {
                    lex_next(lex);
                    if (lex_peek(lex).type == TOK_RPAREN) {
                        lex_next(lex);
                    }
                }
                out_args[count].type = VAL_ARRAY_REF;
                out_args[count].as.string = str_create(vm_get_str(vm), out_arg_names[count], strlen(out_arg_names[count]));
                out_is_byref[count] = true;
                count++;
                if (lex_peek(lex).type == TOK_COMMA) { lex_next(lex); continue; }
                else break;
            }
        }

        BValue val = eval_expression(vm, lex, out_err);
        if (out_err->code != 0) return count;

        if (byref_target && out_arg_names[count][0] != '\0') {
            out_is_byref[count] = true;
        }

        out_args[count] = val;
        count++;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    if (has_parens) {
        if (lex_peek(lex).type == TOK_RPAREN) {
            lex_next(lex);
        } else {
            out_err->code = 2;
            out_err->message = "Expected ')' after procedure argument list";
        }
    }

    return count;
}
