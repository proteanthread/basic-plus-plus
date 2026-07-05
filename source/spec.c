/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: spec.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Utility or helper code for BASIC++ interpreter.
 *
 * 2. WHAT TO EXPECT:
 *    Executes with low overhead, relying on fixed compile-time limits and memory pools.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Internal helper functions, optimization passes, or local naming adjustments.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Public API structures, parameter contracts, or global type definitions.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check memory pool margins, look for segmentation faults, and trace parameter values via a debugger.
 * ===================================================================== */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "spec.h"
#include "runtime.h"
#include "console.h"

static SpecObject spec_registry[MAX_SPECS];
static int spec_count = 0;

void spec_registry_init(void) {
    spec_count = 0;
}

SpecObject* spec_find(const char *name) {
    int i;
    for (i = 0; i < spec_count; i++) {
        if (strcmp(spec_registry[i].name, name) == 0) {
            return &spec_registry[i];
        }
    }
    return NULL;
}

void spec_list_all(void) {
    int i;
    gw_printf("--- Loaded Specifications ---\n");
    for (i = 0; i < spec_count; i++) {
        gw_printf("SPEC: %s (v%s) - Category %d, LIB: %s\n", 
            spec_registry[i].name, 
            spec_registry[i].version, 
            spec_registry[i].category,
            spec_registry[i].lib_path[0] ? spec_registry[i].lib_path : "None");
    }
}

 // Basic parser for DEFINE SPECIFICATION "NAME"
 // It expects a very simple line-based declarative syntax.
int spec_load_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    char line[256];
    SpecObject current_spec;
    int in_spec = 0;

    if (!fp) return -1;

    memset(&current_spec, 0, sizeof(SpecObject));

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\r' || *p == '\n' || *p == '\0' || *p == '#') continue;

        if (strncmp(p, "DEFINE SPECIFICATION", 20) == 0) {
            char *quote = strchr(p, '"');
            if (quote) {
                char *endquote = strchr(quote + 1, '"');
                if (endquote) {
                    *endquote = '\0';
                    strncpy(current_spec.name, quote + 1, 63);
                    in_spec = 1;
                }
            }
        } else if (in_spec && strncmp(p, "END SPECIFICATION", 17) == 0) {
            if (spec_count < MAX_SPECS) {
                // Register the new keyword dynamically!
                if (current_spec.category == SPEC_CAT_STATEMENT) {
                    current_spec.kw_id = keyword_register_custom(current_spec.name, DFLAG_ALL);
                }
                spec_registry[spec_count++] = current_spec;
            }
            in_spec = 0;
            memset(&current_spec, 0, sizeof(SpecObject));
        } else if (in_spec) {
            if (strncmp(p, "CATEGORY", 8) == 0) {
                char *quote = strchr(p, '"');
                if (quote) {
                    char *endquote = strchr(quote + 1, '"');
                    if (endquote) {
                        *endquote = '\0';
                        if (strcmp(quote + 1, "STATEMENT") == 0) current_spec.category = SPEC_CAT_STATEMENT;
                        else if (strcmp(quote + 1, "FUNCTION") == 0) current_spec.category = SPEC_CAT_FUNCTION;
                    }
                }
            } else if (strncmp(p, "VERSION", 7) == 0) {
                char *quote = strchr(p, '"');
                if (quote) {
                    char *endquote = strchr(quote + 1, '"');
                    if (endquote) {
                        *endquote = '\0';
                        strncpy(current_spec.version, quote + 1, 15);
                    }
                }
            } else if (strncmp(p, "LIB", 3) == 0) {
                char *quote = strchr(p, '"');
                if (quote) {
                    char *endquote = strchr(quote + 1, '"');
                    if (endquote) {
                        *endquote = '\0';
                        strncpy(current_spec.lib_path, quote + 1, 255);
                    }
                }
            }
        }
    }

    fclose(fp);
    return 0;
}

void pi_parse_custom_statement(Lexer *lex, RuntimeState *rt, int line_num, int kw_id) {
    (void)rt;
    (void)line_num;
    int i;
    for (i = 0; i < spec_count; i++) {
        if (spec_registry[i].kw_id == kw_id) {
            gw_printf("[SPEC-LIB LOADER] Executing dynamically defined statement: %s\n", spec_registry[i].name);
            gw_printf("[SPEC-LIB LOADER] Loading logic from external cross-platform .LIB archive: %s\n", 
                spec_registry[i].lib_path[0] ? spec_registry[i].lib_path : "(unspecified)");
             // In a full implementation, the VM would now load the custom 
             // BASIC++ .LIB bytecode and GOSUB into the routine here.
            lexer_next(lex); // Consume keyword
            return;
        }
    }
}
