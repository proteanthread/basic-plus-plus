/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_meta.c
 * =====================================================================
 * Subsystem: QBASIC Metacommand Subsystem
 *
 * PURPOSE:
 * Parses and processes QBASIC metacommands ($STATIC, $DYNAMIC, $INCLUDE)
 * which are embedded inside comments (REM or ').
 * ===================================================================== */

#include "../parser_internal.h"
#include "../runtime.h"
#include "../errors.h"
#include "../platform.h"
#include "../standalone/vfs/vfs_core.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Handle $STATIC metacommand
// In QBASIC, $STATIC means arrays are statically allocated.
void pi_parse_meta_static(Lexer *lex, RuntimeState *rt, int line_num)
{
    lexer_next(lex); // consume TOK_META_STATIC
    rt->parser_meta_dynamic = 0; // 0 = STATIC
}

// Handle $DYNAMIC metacommand
// In QBASIC, $DYNAMIC means arrays are dynamically allocated.
void pi_parse_meta_dynamic(Lexer *lex, RuntimeState *rt, int line_num)
{
    lexer_next(lex); // consume TOK_META_DYNAMIC
    rt->parser_meta_dynamic = 1; // 1 = DYNAMIC
}

// Handle $INCLUDE metacommand
// Syntax: '$INCLUDE: 'filename.bas'
void pi_parse_meta_include(Lexer *lex, RuntimeState *rt, int line_num)
{
    // The string value of the metacommand is captured in the token's string.
    // e.g. " 'filename.bas'"
    char inc_file[256];
    inc_file[0] = '\0';
    
    // Parse the filename from lex->current.str_start
    if (lex->current.str_start != NULL && lex->current.str_length > 0) {
        int i = 0, j = 0;
        int in_quotes = 0;
        while (i < lex->current.str_length && j < 255) {
            char c = lex->current.str_start[i++];
            if (c == '\'' || c == '"') {
                in_quotes = !in_quotes;
                continue;
            }
            if (in_quotes || (c != ' ' && c != '\t')) {
                inc_file[j++] = c;
            }
        }
        inc_file[j] = '\0';
    }
    
    lexer_next(lex); // consume TOK_META_INCLUDE
    
    if (inc_file[0] == '\0') {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    
    // Load and parse the included file inline.
    char resolved[512];
    if (vfs_core_resolve(inc_file, resolved, sizeof(resolved), 0) != 0) {
        // Fallback to original if not mounted
        strncpy(resolved, inc_file, sizeof(resolved) - 1);
        resolved[sizeof(resolved) - 1] = '\0';
    }
    
    FILE *f = fopen(resolved, "rb");
    if (!f) {
        error_raise(ERR_HOW, line_num);
        return;
    }
    
    // Read file contents
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (sz <= 0) {
        fclose(f);
        return; // Empty file
    }
    
    char *inc_src = (char *)malloc((size_t)(sz + 1));
    if (!inc_src) {
        fclose(f);
        error_raise(ERR_SORRY, line_num);
        return;
    }
    
    size_t read_bytes = fread(inc_src, 1, (size_t)sz, f);
    inc_src[read_bytes] = '\0';
    fclose(f);
    
    // Preserve old lexer state
    Lexer old_lex = *lex;
    Lexer new_lex;
    
    char *line_start = inc_src;
    while (line_start && *line_start) {
        char *line_end = strchr(line_start, '\n');
        if (line_end) {
            *line_end = '\0';
        }
        
        // Remove trailing \r if present
        int len = (int)strlen(line_start);
        if (len > 0 && line_start[len - 1] == '\r') {
            line_start[len - 1] = '\0';
        }
        
        if (line_start[0] != '\0') {
            lexer_init(&new_lex, line_start);
            while (new_lex.current.type != TOK_EOF) {
                if (new_lex.current.type == TOK_CR || new_lex.current.type == TOK_COLON) {
                    lexer_next(&new_lex);
                    continue;
                }
                pi_parse_statement(&new_lex, rt, line_num);
                if (error_occurred()) break;
                
                if (new_lex.current.type == TOK_COLON) {
                    lexer_next(&new_lex);
                } else if (new_lex.current.type == TOK_CR) {
                    lexer_next(&new_lex);
                } else if (new_lex.current.type != TOK_EOF && 
                           !(new_lex.current.type == TOK_KEYWORD && new_lex.current.value.keyword == KW_ELSE) && 
                           !(new_lex.current.type == TOK_KEYWORD && new_lex.current.value.keyword == KW_ELSEIF)) {
                    error_raise(ERR_WHAT, line_num);
                    break;
                }
            }
        }
        
        if (error_occurred()) break;
        
        if (line_end) {
            line_start = line_end + 1;
        } else {
            break;
        }
    }
    
    free(inc_src);
    
    // Restore original lexer state
    *lex = old_lex;
}
