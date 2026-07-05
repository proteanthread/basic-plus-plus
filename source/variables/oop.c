/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: oop.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Object-Oriented Programming (OOP) class parsing and structures.
 * ===================================================================== */

#include "oop.h"
#include "errors.h"
#include <string.h>

void pi_parse_class(Lexer *lex, RuntimeState *rt, int line_num)
{
    char cname[MAX_VAR_NAME_LEN + 1];
    int len = 0;
    
    // Parse the class name
    if (lex->current.type == TOK_NAMED_VAR) {
        len = lex->current.str_length;
        if (len > MAX_VAR_NAME_LEN) len = MAX_VAR_NAME_LEN;
        memcpy(cname, lex->current.str_start, (size_t)len);
    } else if (lex->current.type == TOK_VARIABLE) {
        cname[0] = lex->current.value.var_name;
        len = 1;
    } else if (lex->current.type == TOK_KEYWORD) {
        const char *kn = lexer_keyword_name(lex->current.value.keyword);
        len = kn ? (int)strlen(kn) : 0;
        if (len > MAX_VAR_NAME_LEN) len = MAX_VAR_NAME_LEN;
        if (kn) memcpy(cname, kn, (size_t)len);
    } else {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    cname[len] = '\0';
    lexer_next(lex); // consume name
    
    // Uppercase for canonical name matching
    for (int i = 0; i < len; i++) {
        if (cname[i] >= 'a' && cname[i] <= 'z') {
            cname[i] = (char)(cname[i] - 32);
        }
    }
    
    // Check if already exists
    UserTypeDef *existing = runtime_find_type(rt, cname, len);
    if (existing != NULL) {
        // If it already exists (e.g. during execution loop), skip to ENDCLASS
        int depth = 1;
        while (lex->current.type != TOK_EOF) {
            if (lex->current.type == TOK_KEYWORD) {
                if (lex->current.value.keyword == KW_CLASS) {
                    depth++;
                } else if (lex->current.value.keyword == KW_ENDCLASS) {
                    depth--;
                    if (depth == 0) {
                        lexer_next(lex); // consume ENDCLASS
                        break;
                    }
                }
            }
            lexer_next(lex);
        }
        return;
    }
    
    if (rt->type_count >= MAX_USER_TYPES) {
        error_raise(ERR_SORRY, line_num);
        return;
    }
    
    UserTypeDef *utd = &rt->user_types[rt->type_count++];
    memset(utd, 0, sizeof(UserTypeDef));
    memcpy(utd->name, cname, (size_t)len);
    utd->name[len] = '\0';
    utd->is_class = 1;
    
    // Set class context
    strncpy(rt->class_context, cname, MAX_VAR_NAME_LEN);
    rt->class_context[MAX_VAR_NAME_LEN] = '\0';
}

void pi_parse_endclass(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex;
    (void)line_num;
    rt->class_context[0] = '\0';
}
