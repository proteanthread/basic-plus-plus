/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: uef.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Dynamic Unified Extension Framework (UEF) for static/dynamic C modules
 *    and IMPORT/INCLUDE statement routing.
 * ===================================================================== */

#include "lexer.h"
#include "runtime.h"
#include "errors.h"
#include "parser_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

void pi_parse_include(Lexer *lex, RuntimeState *rt, int line_num)
{
    if (lex->current.type != TOK_STRING) {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    char path[260];
    int len = lex->current.str_length;
    if (len > 259) len = 259;
    memcpy(path, lex->current.str_start, (size_t)len);
    path[len] = '\0';
    lexer_next(lex);
    
    FILE *f = fopen(path, "r");
    if (!f) {
        error_raise(ERR_HOW, line_num); // File not found
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        error_raise(ERR_SORRY, line_num);
        return;
    }
    
    size_t read_bytes = fread(buf, 1, (size_t)size, f);
    buf[read_bytes] = '\0';
    fclose(f);
    
    Lexer inc_lex;
    lexer_init(&inc_lex, buf);
    while (inc_lex.current.type != TOK_EOF && !error_occurred()) {
        parser_execute_line(&inc_lex, rt, line_num);
    }
    free(buf);
}

void pi_parse_import(Lexer *lex, RuntimeState *rt, int line_num)
{
    if (lex->current.type != TOK_STRING) {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    char path[260];
    int len = lex->current.str_length;
    if (len > 259) len = 259;
    memcpy(path, lex->current.str_start, (size_t)len);
    path[len] = '\0';
    lexer_next(lex);

#ifdef _WIN32
    HMODULE lib = LoadLibraryA(path);
    if (!lib) {
        char path_dll[280];
        snprintf(path_dll, sizeof(path_dll), "%s.dll", path);
        lib = LoadLibraryA(path_dll);
    }
    if (!lib) {
        error_raise(ERR_HOW, line_num);
        return;
    }
    typedef int (*ModInitFunc)(void *);
    ModInitFunc init_f = (ModInitFunc)GetProcAddress(lib, "bpp_module_init");
    if (!init_f) {
        FreeLibrary(lib);
        error_raise(ERR_HOW, line_num);
        return;
    }
#else
    void *lib = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (!lib) {
        char path_so[280];
        snprintf(path_so, sizeof(path_so), "./%s.so", path);
        lib = dlopen(path_so, RTLD_NOW | RTLD_GLOBAL);
    }
    if (!lib) {
        error_raise(ERR_HOW, line_num);
        return;
    }
    typedef int (*ModInitFunc)(void *);
    ModInitFunc init_f = (ModInitFunc)dlsym(lib, "bpp_module_init");
    if (!init_f) {
        dlclose(lib);
        error_raise(ERR_HOW, line_num);
        return;
    }
#endif

    if (init_f(rt) != 0) {
        error_raise(ERR_HOW, line_num);
    }
}
