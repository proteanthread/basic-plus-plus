/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: trans.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Standalone transpiler command line tool for BASIC++ (trans).
 *    Transpiles BASIC source files to C17, C89, Python 3, Free Pascal, or Fortran.
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../config.h"
#include "../platform.h"
#include "../ast.h"
#include "../lexer.h"
#include "../memory.h"
#include "../errors.h"
#include "../codegen/trans.h"

#include "../boot.h"
#include "../runtime.h"
#include "../codegen/target.h"

// Simple file loader that reads lines and stores them in ProgramStore
static int load_program_file(const char *filename, ProgramStore *store) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    
    store->count = 0;
    store->capacity = 1024;
    store->lines = malloc(sizeof(ProgramLine) * store->capacity);
    if (!store->lines) {
        fclose(f);
        return -2;
    }
    
    char line_buf[2048];
    while (fgets(line_buf, sizeof(line_buf), f)) {
        // Strip trailing newline characters
        size_t len = strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\r' || line_buf[len - 1] == '\n')) {
            line_buf[len - 1] = '\0';
            len--;
        }
        if (len == 0) continue;
        
        // Find line number
        char *p = line_buf;
        while (*p && (*p == ' ' || *p == '\t')) p++;
        if (!*p) continue;
        
        double line_num = atof(p);
        if (line_num <= 0) {
            // Skip unnumbered comments or declarations for classic programs
            continue;
        }
        
        if (store->count >= store->capacity) {
            store->capacity *= 2;
            ProgramLine *new_lines = realloc(store->lines, sizeof(ProgramLine) * store->capacity);
            if (!new_lines) {
                fclose(f);
                return -2;
            }
            store->lines = new_lines;
        }
        
        store->lines[store->count].line_number = line_num;
        store->lines[store->count].text = plat_strdup(line_buf);
        store->count++;
    }
    
    fclose(f);
    return 0;
}

// Global variable required by the parser is already defined in errors.c

int main(int argc, char **argv)
{
    TransTarget target_lang = TRANS_TARGET_C17;
    const char *infile = NULL;
    const char *outfile = "output.out";
    int i;

    printf("BASIC++ Transpiler (trans) v" BASICPP_VERSION "\n");

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--target") == 0 && i + 1 < argc) {
            const char *t = argv[++i];
            if (strcmp(t, "c17") == 0) target_lang = TRANS_TARGET_C17;
            else if (strcmp(t, "c89") == 0) target_lang = TRANS_TARGET_C89;
            else if (strcmp(t, "py3") == 0) target_lang = TRANS_TARGET_PY3;
            else if (strcmp(t, "pas") == 0) target_lang = TRANS_TARGET_PAS;
            else if (strcmp(t, "f90") == 0) target_lang = TRANS_TARGET_F90;
            else {
                fprintf(stderr, "Unknown target language: %s\n", t);
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outfile = argv[++i];
        } else if (argv[i][0] != '-') {
            infile = argv[i];
        }
    }

    if (!infile) {
        printf("Usage: trans [--target c17|c89|py3|pas|f90] <input.bas> [-o <output_file>]\n");
        return 1;
    }

    printf("Transpiling %s -> %s...\n", infile, outfile);

    BootConfig boot_cfg;
    memset(&boot_cfg, 0, sizeof(boot_cfg));
    boot_cfg.verbosity = BOOT_SILENT;

    MemorySystem memory;
    static RuntimeState runtime;
    if (boot_execute(&boot_cfg, &memory, &runtime) == BOOT_CRITICAL) {
        fprintf(stderr, "Failed to initialize bootstrap engine.\n");
        return 1;
    }

    keyword_registry_init();

    ProgramStore program = {0};
    if (load_program_file(infile, &program) != 0) {
        fprintf(stderr, "Failed to load input file '%s'\n", infile);
        return 1;
    }

    if (program.count == 0) {
        fprintf(stderr, "Input file '%s' contains no numbered lines.\n", infile);
        return 1;
    }

    AstLine *ast_lines = malloc(sizeof(AstLine) * program.count);
    if (!ast_lines) {
        fprintf(stderr, "Out of memory allocating AST lines.\n");
        return 1;
    }
    memset(ast_lines, 0, sizeof(AstLine) * program.count);

    int success = 1;
    for (i = 0; i < program.count; i++) {
        Lexer lex;
        ProgramLine *pl = &program.lines[i];

        ast_lines[i].line_number = pl->line_number;

        lexer_init(&lex, pl->text);

        // Skip the line number token
        if (lex.current.type == TOK_NUMBER || lex.current.type == TOK_FLOAT_LIT) {
            lexer_next(&lex);
        }

        error_clear();
        g_current_executing_line = pl->line_number;

        ast_lines[i].stmts = ast_build_line(&lex, (int)pl->line_number);

        if (error_occurred()) {
            printf("Compile error at line %.0f\n", pl->line_number);
            success = 0;
            error_clear();
            break;
        }
    }

    int ret = -1;
    if (success) {
        FILE *out = fopen(outfile, "w");
        if (!out) {
            perror("Failed to open output file");
            return 1;
        }

        const TargetConfig *config = NULL;
#ifdef _WIN32
        config = target_find("windows");
#else
        config = target_find("linux");
#endif
        if (!config) {
            config = target_get_default();
        }
        ret = trans_emit(out, ast_lines, program.count, &program, config, target_lang);
        fclose(out);
    }

    // Cleanup
    g_current_executing_line = 0.0;
    for (i = 0; i < program.count; i++) {
        if (ast_lines[i].stmts) {
            ast_free_line(ast_lines[i].stmts);
        }
        free(program.lines[i].text);
    }
    free(ast_lines);
    free(program.lines);

    if (ret == 0) {
        printf("Transpilation completed successfully.\n");
    } else {
        printf("Transpilation failed.\n");
    }

    return ret;
}
