/* =====================================================================
 * BASIC++ Interpreter — PCode Cache Manager
 * ===================================================================== */

#ifndef BPP_LITE_BUILD

#include <stdlib.h>
#include <stdio.h>
#include "runtime.h"
#include "ast.h"
#include "pcode.h"

void pcode_cache_invalidate(RuntimeState *rt)
{
    if (rt->has_loaded_pcode && rt->loaded_pcode != NULL) {
        PCodeProgram *pcode = (PCodeProgram *)rt->loaded_pcode;
        pcode_free(pcode);
        free(pcode);
        rt->loaded_pcode = NULL;
        rt->has_loaded_pcode = 0;
    }

    if (rt->line_asts != NULL) {
        int i;
        for (i = 0; i < rt->line_asts_count; i++) {
            if (rt->line_asts[i] != NULL) {
                ast_free_line(rt->line_asts[i]);
            }
        }
        free(rt->line_asts);
        rt->line_asts = NULL;
    }
    rt->line_asts_count = 0;
}

#endif
