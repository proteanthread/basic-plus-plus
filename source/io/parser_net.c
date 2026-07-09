#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lexer.h"
#include "../value.h"
#include "../errors.h"
#include "parser_internal.h"
#include "vdev_net.h"

// pi_parse_gemini - Execute GEMINI statement
// Syntax: GEMINI "gemini://url", var$
void pi_parse_gemini(Lexer *lex, RuntimeState *rt, int line_num)
{
    BValue url_val = parse_expression_bval(lex, rt, line_num);
    if (error_occurred()) return;

    if (!bval_is_string(&url_val)) {
        error_raise(ERR_HOW, line_num);
        return;
    }

    if (lex->current.type != TOK_COMMA) {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    lexer_next(lex); // consume comma

    if (lex->current.type != TOK_NAMED_VAR && lex->current.type != TOK_STRING_VAR) {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    
    char name[256];
    int nlen = lex->current.str_length;
    if (nlen > 255) nlen = 255;
    memcpy(name, lex->current.str_start, (size_t)nlen);
    name[nlen] = '\0';
    lexer_next(lex);

    // Call the network virtual device to fetch (which handles TLS if available)
    char url_str[512];
    // We use bval_string_length, but we didn't declare it. Let's use stringpool for now... Wait! 
    // Wait, BValue holds .v.sval.length and .v.sval.data for strings. Let's just access those!
    int ulen = url_val.v.sval.length;
    if (ulen > 511) ulen = 511;
    memcpy(url_str, url_val.v.sval.data, (size_t)ulen);
    url_str[ulen] = '\0';

    char *result = net_gemini_fetch(url_str);
    if (result == NULL) {
        error_raise(ERR_HOW, line_num);
        return;
    }

    runtime_set_named_var_bval(rt, name, nlen, bval_string(result, (int)strlen(result)));
    free(result);
}

// pi_parse_gopher - Execute GOPHER statement
// Syntax: GOPHER "gopher://url", var$
void pi_parse_gopher(Lexer *lex, RuntimeState *rt, int line_num)
{
    BValue url_val = parse_expression_bval(lex, rt, line_num);
    if (error_occurred()) return;

    if (!bval_is_string(&url_val)) {
        error_raise(ERR_HOW, line_num);
        return;
    }

    if (lex->current.type != TOK_COMMA) {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    lexer_next(lex); // consume comma

    if (lex->current.type != TOK_NAMED_VAR && lex->current.type != TOK_STRING_VAR) {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    char name[256];
    int nlen = lex->current.str_length;
    if (nlen > 255) nlen = 255;
    memcpy(name, lex->current.str_start, (size_t)nlen);
    name[nlen] = '\0';
    lexer_next(lex);

    char url_str[512];
    int ulen = url_val.v.sval.length;
    if (ulen > 511) ulen = 511;
    memcpy(url_str, url_val.v.sval.data, (size_t)ulen);
    url_str[ulen] = '\0';

    // Call the network virtual device to fetch via gopher
    char *result = net_gopher_fetch(url_str);
    if (result == NULL) {
        error_raise(ERR_HOW, line_num);
        return;
    }

    runtime_set_named_var_bval(rt, name, nlen, bval_string(result, (int)strlen(result)));
    free(result);
}
