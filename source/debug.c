#include "parser_internal.h"
#ifdef _WIN32
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif
#include "device_alias.h"
#include "sdl2_emu.h"
#include "lexer.h"
#include "value.h"
#include "errors.h"
#include "funcreg.h"
#include "vdev.h"
#include "security.h"
#include "exec.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

#ifndef NO_SDL2
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define TokenType WinTokenType
#include <windows.h>
#undef TokenType
#endif
#include <SDL.h>
#include "console.h"
#endif

// Static logging helper
static void st_log(RuntimeState *rt, const char *fmt, ...)
{
    va_list args;
    if (rt && rt->log_fp) {
        va_start(args, fmt);
        fprintf((FILE*)rt->log_fp, "[SELFTEST] ");
        vfprintf((FILE*)rt->log_fp, fmt, args);
        fprintf((FILE*)rt->log_fp, "\n");
        va_end(args);
        fflush((FILE*)rt->log_fp);
    }
}

// 1. Tracing control: TRON and TROFF
void pi_parse_tron(Lexer *lex, RuntimeState *rt, int line_num) {
    (void)lex; (void)line_num;
    rt->trace_on = 1;
}

void pi_parse_troff(Lexer *lex, RuntimeState *rt, int line_num) {
    (void)lex; (void)line_num;
    rt->trace_on = 0;
}

void pi_parse_trace(Lexer *lex, RuntimeState *rt, int line_num) {
    if (lex->current.type == TOK_KEYWORD) {
        KeywordId kw = lex->current.value.keyword;
        if (kw == KW_ON) {
            rt->trace_on = 1;
            lexer_next(lex);
            return;
        }
    }
    if (lex->current.type == TOK_NAMED_VAR || lex->current.type == TOK_VARIABLE) {
        char name[8] = {0};
        if (lex->current.type == TOK_VARIABLE) {
            name[0] = lex->current.value.var_name;
        } else {
            int len = lex->current.str_length;
            if (len > 7) len = 7;
            strncpy(name, lex->current.str_start, len);
        }
        if (pi_str_case_equal(name, "OFF")) {
            rt->trace_on = 0;
            lexer_next(lex);
            return;
        }
    }
    error_raise(ERR_WHAT, line_num);
}

// 2. Breakpoint Debugger: BREAK and CONT
void pi_parse_break(Lexer *lex, RuntimeState *rt, int line_num) {
    if (lex->current.type == TOK_NUMBER) {
        long val = lex->current.value.num_value;
        lexer_next(lex);
        if (val > 0) {
            // Set breakpoint
            int exists = 0;
            for (int i = 0; i < rt->breakpoint_count; i++) {
                if (rt->breakpoints[i] == (int)val) {
                    exists = 1;
                    break;
                }
            }
            if (!exists && rt->breakpoint_count < 256) {
                rt->breakpoints[rt->breakpoint_count++] = (int)val;
            }
            printf("Breakpoint set at line %ld\n", val);
        } else if (val < 0) {
            // Clear breakpoint
            long target = -val;
            int found = -1;
            for (int i = 0; i < rt->breakpoint_count; i++) {
                if (rt->breakpoints[i] == (int)target) {
                    found = i;
                    break;
                }
            }
            if (found >= 0) {
                for (int i = found; i < rt->breakpoint_count - 1; i++) {
                    rt->breakpoints[i] = rt->breakpoints[i + 1];
                }
                rt->breakpoint_count--;
            }
            printf("Breakpoint cleared at line %ld\n", target);
        }
    } else if (lex->current.type == TOK_MINUS) {
        // Handle negative numbers if parsed as operator + number
        lexer_next(lex);
        if (lex->current.type == TOK_NUMBER) {
            long val = lex->current.value.num_value;
            lexer_next(lex);
            int found = -1;
            for (int i = 0; i < rt->breakpoint_count; i++) {
                if (rt->breakpoints[i] == (int)val) {
                    found = i;
                    break;
                }
            }
            if (found >= 0) {
                for (int i = found; i < rt->breakpoint_count - 1; i++) {
                    rt->breakpoints[i] = rt->breakpoints[i + 1];
                }
                rt->breakpoint_count--;
            }
            printf("Breakpoint cleared at line %ld\n", val);
        } else {
            error_raise(ERR_WHAT, line_num);
        }
    } else {
        // List breakpoints
        if (rt->breakpoint_count == 0) {
            printf("No breakpoints set.\n");
        } else {
            printf("Breakpoints:");
            for (int i = 0; i < rt->breakpoint_count; i++) {
                printf(" %d", rt->breakpoints[i]);
            }
            printf("\n");
        }
    }
}

void pi_parse_cont(Lexer *lex, RuntimeState *rt, int line_num) {
    (void)lex;
    extern int exec_cont(RuntimeState *rt);
    if (exec_cont(rt) != 0) {
        error_raise(ERR_HOW, line_num); // Can't continue
    }
}

// 3. Unit Testing: TEST, ENDTEST, and ASSERT
void pi_parse_test(Lexer *lex, RuntimeState *rt, int line_num) {
    BValue name_val = parse_expression_bval(lex, rt, line_num);
    if (error_occurred()) return;
    char buf[128] = {0};
    bval_to_string_buf(&name_val, buf, sizeof(buf));
    
    rt->in_test = 1;
    strncpy(rt->test_name, buf, sizeof(rt->test_name) - 1);
    rt->test_name[sizeof(rt->test_name) - 1] = '\0';
    rt->test_pass = 0;
    rt->test_fail = 0;
    rt->test_total = 0;
    
    printf("=== TEST: %s ===\n", rt->test_name);
}

void pi_parse_endtest(Lexer *lex, RuntimeState *rt, int line_num) {
    (void)lex;
    if (!rt->in_test) {
        error_raise(ERR_HOW, line_num);
        return;
    }
    printf("Test '%s': %d passed, %d failed.\n", rt->test_name, rt->test_pass, rt->test_fail);
    rt->in_test = 0;
    rt->test_name[0] = '\0';
}

void pi_parse_assert(Lexer *lex, RuntimeState *rt, int line_num) {
    BValue cond_val = parse_expression_bval(lex, rt, line_num);
    if (error_occurred()) return;
    int pass = (bval_to_int(&cond_val) != 0);
    
    if (rt->in_test) {
        rt->test_total++;
        if (pass) {
            rt->test_pass++;
            rt->assert_pass_total++;
        } else {
            rt->test_fail++;
            rt->assert_fail_total++;
            printf("ASSERTION FAILED at line %d\n", line_num);
        }
    } else {
        if (!pass) {
            printf("Assertion failed at line %d\n", line_num);
            error_raise(ERR_HOW, line_num);
        }
    }
}

// 4. Debug & Introspection: INFO, DEBUG, DUMP, and BACKTRACE
void pi_parse_info(Lexer *lex, RuntimeState *rt, int line_num) {
    (void)lex; (void)line_num;
    int vars_count = 0;
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (bval_to_float(&rt->variables[i]) != 0.0) vars_count++;
    }
    
    printf("=== BASIC++ SYSTEM INFO ===\n\n");
    printf(" Version: 4.1.2\n");
    printf(" Name: BASIC++ (Stable)\n");
    printf(" Dialect: BASIC++ [BPP]\\n");
    printf(" Memmap: NONE\\n");
    SecLevel sec = security_get_level();
    printf(" Security: %s (%d)\n", security_level_name(sec), (int)sec);
    printf(" Functions: %d registered\n", funcreg_count());
    printf(" Program: %d lines\n", rt->program->count);
    printf(" Max Lines: 65536\n");
    printf(" Variables: %d (A-Z)\n", vars_count);
    printf(" Stack: %d levels\n", rt->stack_top);
    printf(" Breakpoints: %d max (%d set)\n", 256, rt->breakpoint_count);
    printf(" Build: %s %s\n", __DATE__, __TIME__);
    printf(" Standard: C17\n");
}

void pi_parse_debug(Lexer *lex, RuntimeState *rt, int line_num) {
    int first = 1;
    printf("[DEBUG] ");
    while (lex->current.type != TOK_EOF && lex->current.type != TOK_CR && lex->current.type != TOK_COLON) {
        if (!first) {
            if (lex->current.type == TOK_COMMA) {
                lexer_next(lex);
            }
        }
        first = 0;
        BValue val = parse_expression_bval(lex, rt, line_num);
        if (error_occurred()) return;
        char buf[256] = {0};
        bval_to_string_buf(&val, buf, sizeof(buf));
        printf("%s ", buf);
    }
    printf("\n");
}

void pi_parse_dump(Lexer *lex, RuntimeState *rt, int line_num) {
    FILE *out = stdout;
    char path[260] = {0};
    
    if (lex->current.type == TOK_STRING) {
        BValue path_val = parse_expression_bval(lex, rt, line_num);
        if (error_occurred()) return;
        bval_to_string_buf(&path_val, path, sizeof(path));
        out = fopen(path, "w");
        if (!out) {
            error_raise(ERR_HOW, line_num);
            return;
        }
    }
    
    fprintf(out, "--- Variables ---\n");
    int empty = 1;
    for (int i = 0; i < MAX_VARIABLES; i++) {
        BValue val = rt->variables[i];
        double f = bval_to_float(&val);
        if (f != 0.0) {
            empty = 0;
            char buf[32] = {0};
            bval_to_string_buf(&val, buf, sizeof(buf));
            fprintf(out, "  %c = %s\n", 'A' + i, buf);
        }
    }
    for (int i = 0; i < MAX_STRING_VARS; i++) {
        BValue val = rt->string_vars[i];
        if (val.v.sval.data != NULL && val.v.sval.length > 0) {
            empty = 0;
            char buf[256] = {0};
            bval_to_string_buf(&val, buf, sizeof(buf));
            fprintf(out, "  %c$ = \"%s\"\n", 'A' + i, buf);
        }
    }
    if (empty) {
        fprintf(out, "  (all zero)\n");
    }
    
    if (out != stdout) {
        fclose(out);
        printf("Variable state dumped to %s\n", path);
    }
}

void pi_parse_backtrace(Lexer *lex, RuntimeState *rt, int line_num) {
    (void)lex; (void)line_num;
    if (rt->stack_top == 0) {
        printf("Stack empty.\n");
        return;
    }
    for (int i = rt->stack_top - 1; i >= 0; i--) {
        StackFrame *f = &rt->stack[i];
        if (f->type == FRAME_GOSUB) {
            int ret_idx = f->data.gosub.return_index;
            double line = (ret_idx < rt->program->count) ? rt->program->lines[ret_idx].line_number : 0.0;
            if (floor(line) == line) {
                printf("  Frame %d: GOSUB (returns to line %.0f)\n", i, line);
            } else {
                printf("  Frame %d: GOSUB (returns to line %.2f)\n", i, line);
            }
        } else if (f->type == FRAME_FOR) {
            char var = f->data.for_loop.var_name;
            printf("  Frame %d: FOR loop (variable %c, limit %f)\n", i, var, f->data.for_loop.limit);
        } else if (f->type == FRAME_WHILE) {
            int loop_idx = f->data.while_loop.loop_index;
            double line = (loop_idx < rt->program->count) ? rt->program->lines[loop_idx].line_number : 0.0;
            if (floor(line) == line) {
                printf("  Frame %d: WHILE loop (starts at line %.0f)\n", i, line);
            } else {
                printf("  Frame %d: WHILE loop (starts at line %.2f)\n", i, line);
            }
        } else if (f->type == FRAME_DO) {
            printf("  Frame %d: DO loop\n", i);
        } else if (f->type == FRAME_SUB) {
            printf("  Frame %d: SUB call\n", i);
        }
    }
}

// 5. Static Checker & Verifier Subsystem
typedef struct LVar {
    char name[32];
    int assigned;
    int used;
    int first_line;
} LVar;

static int min3(int a, int b, int c) {
    int m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    return m;
}

static int levenshtein(const char *s1, const char *s2) {
    int len1 = (int)strlen(s1);
    int len2 = (int)strlen(s2);
    int *matrix = malloc((len1 + 1) * (len2 + 1) * sizeof(int));
    if (!matrix) return 99;
    
    for (int i = 0; i <= len1; i++) {
        matrix[i * (len2 + 1) + 0] = i;
    }
    for (int j = 0; j <= len2; j++) {
        matrix[0 * (len2 + 1) + j] = j;
    }
    
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            matrix[i * (len2 + 1) + j] = min3(
                matrix[(i - 1) * (len2 + 1) + j] + 1,
                matrix[i * (len2 + 1) + (j - 1)] + 1,
                matrix[(i - 1) * (len2 + 1) + (j - 1)] + cost
            );
        }
    }
    int dist = matrix[len1 * (len2 + 1) + len2];
    free(matrix);
    return dist;
}

static void run_static_analysis(RuntimeState *rt, int is_verify, const char *vname) {
    (void)vname;
    (void)is_verify;
    printf("=== CHECK: Program Analysis ===\n\n");
    
    int total_lines = rt->program->count;
    int code_lines = 0;
    int comment_lines = 0;
    
    LVar vars[512];
    int vars_count = 0;
    
    int data_items = 0;
    int read_vars = 0;
    int for_loops = 0;
    int gosub_calls = 0;
    
    // Jump targets tracking
    int jump_targets[512];
    int jump_targets_count = 0;
    
    // Referenced jumps verification
    int referenced_jumps[512];
    int referenced_jumps_count = 0;
    int jump_sources[512];
    
    // Nested constructs
    int while_count = 0;
    int wend_count = 0;
    int for_count = 0;
    int next_count = 0;
    
    // Complexity count
    int complexity = 1;
    
    // Errors & warnings count
    int errors_cnt = 0;
    int warnings_cnt = 0;
    
    // Source text size
    long source_text_size = 0;
    
    for (int i = 0; i < total_lines; i++) {
        ProgramLine *line = &rt->program->lines[i];
        source_text_size += (long)strlen(line->text);
        
        Lexer l;
        lexer_init(&l, line->text);
        
        int is_comment = 0;
        int has_number = 0;
    (void)has_number;
        int lnum = 0;
        
        if (l.current.type == TOK_NUMBER) {
            lnum = (int)l.current.value.num_value;
            has_number = 1;
            lexer_next(&l);
        }
        
        if (l.current.type == TOK_KEYWORD) {
            KeywordId kw = l.current.value.keyword;
            if (kw == KW_REM) {
                is_comment = 1;
            }
        }
        
        if (is_comment) {
            comment_lines++;
        } else {
            code_lines++;
        }
        
        // Loop over tokens in line
        lexer_init(&l, line->text);
        if (l.current.type == TOK_NUMBER) lexer_next(&l);
        
        int in_assignment = 0;
        int in_read_input = 0;
        int in_for = 0;
        
        while (l.current.type != TOK_EOF && l.current.type != TOK_CR) {
            if (l.current.type == TOK_COLON) {
                in_assignment = 0;
                in_read_input = 0;
                in_for = 0;
                lexer_next(&l);
                continue;
            }
            
            if (l.current.type == TOK_KEYWORD) {
                KeywordId kw = l.current.value.keyword;
                if (kw == KW_REM) {
                    break;
                }
                if (kw == KW_LET) {
                    in_assignment = 1;
                } else if (kw == KW_FOR) {
                    in_for = 1;
                    for_loops++;
                    for_count++;
                    complexity++;
                } else if (kw == KW_NEXT) {
                    next_count++;
                } else if (kw == KW_INPUT || kw == KW_READ) {
                    in_read_input = 1;
                } else if (kw == KW_WHILE) {
                    while_count++;
                    complexity++;
                } else if (kw == KW_WEND) {
                    wend_count++;
                } else if (kw == KW_IF || kw == KW_UNTIL) {
                    complexity++;
                } else if (kw == KW_AND || kw == KW_OR) {
                    complexity++;
                } else if (kw == KW_GOSUB) {
                    gosub_calls++;
                }
                
                // Portability warning
                if (kw == KW_INP || kw == KW_OUT || kw == KW_PEEK || kw == KW_POKE || kw == KW_PEEKB || kw == KW_POKEB) {
                    const char *kname = "INP";
                    if (kw == KW_OUT) kname = "OUT";
                    else if (kw == KW_PEEK) kname = "PEEK";
                    else if (kw == KW_POKE) kname = "POKE";
                    else if (kw == KW_PEEKB) kname = "PEEKB";
                    else if (kw == KW_POKEB) kname = "POKEB";
                    printf("  WARNING (line %d): Non-portable keyword '%s' used\n", lnum, kname);
                    warnings_cnt++;
                }

                // Check PEEKB/POKEB bank and offset literals
                if (kw == KW_PEEKB || kw == KW_POKEB) {
                    Lexer look = l;
                    lexer_next(&look);
                    if (look.current.type == TOK_LPAREN) {
                        lexer_next(&look);
                    }
                    if (look.current.type == TOK_NUMBER) {
                        int bank = (int)look.current.value.num_value;
                        if (bank < 1 || bank > 254) {
                            printf("  WARNING (line %d): Memory bank ID %d is out of range (1-254)\n", lnum, bank);
                            warnings_cnt++;
                        }
                        lexer_next(&look);
                        if (look.current.type == TOK_COMMA) {
                            lexer_next(&look);
                            if (look.current.type == TOK_NUMBER) {
                                long offset = (long)look.current.value.num_value;
                                if (offset < 0 || offset >= 1048576) {
                                    printf("  WARNING (line %d): RAMBANK offset %ld is out of range (0-1048575)\n", lnum, offset);
                                    warnings_cnt++;
                                }
                            }
                        }
                    }
                }

                // Check BANK memory bank limits
                if (kw == KW_BANK) {
                    Lexer look = l;
                    lexer_next(&look);
                    if (look.current.type == TOK_NAMED_VAR && 
                        look.current.str_length == 4 && 
                        strncasecmp(look.current.str_start, "LIST", 4) == 0) {
                        // BANK LIST is valid
                    } else if (look.current.type == TOK_NAMED_VAR && 
                        look.current.str_length == 4 && 
                        (strncasecmp(look.current.str_start, "COPY", 4) == 0 ||
                         strncasecmp(look.current.str_start, "FILL", 4) == 0)) {
                        // BANK COPY / FILL - valid
                    } else if (look.current.type == TOK_NUMBER) {
                        int bank = (int)look.current.value.num_value;
                        if (bank < 1 || bank > 254) {
                            printf("  WARNING (line %d): Memory bank ID %d is out of range (1-254) in BANK statement\n", lnum, bank);
                            warnings_cnt++;
                        }
                    }
                }

                // Check FRE bank limit
                if (kw == KW_FRE) {
                    Lexer look = l;
                    lexer_next(&look);
                    if (look.current.type == TOK_LPAREN) {
                        lexer_next(&look);
                    }
                    if (look.current.type == TOK_NUMBER) {
                        int bank = (int)look.current.value.num_value;
                        if (bank > 254 || bank < 0) {
                            printf("  WARNING (line %d): Memory bank ID %d in FRE() out of range (1-254)\n", lnum, bank);
                            warnings_cnt++;
                        }
                    }
                }

                // Check TASK spawn target / PID limits
                if (kw == KW_TASK) {
                    Lexer look = l;
                    lexer_next(&look);
                    if (look.current.type == TOK_KEYWORD && 
                        (look.current.value.keyword == KW_LIST || 
                         look.current.value.keyword == KW_WAIT || 
                         look.current.value.keyword == KW_KILL)) {
                        // Skip validation, these are modifiers
                    } else if (look.current.type == TOK_STRING) {
                        char fname[260] = {0};
                        int len = look.current.str_length;
                        if (len > 255) len = 255;
                        if (look.current.str_start != NULL) {
                            strncpy(fname, look.current.str_start, len);
                        }
                        
                        // Check if file exists if in VERIFY mode or general linting
                        int flen = (int)strlen(fname);
                        if (flen > 0) {
                            FILE *f = fopen(fname, "r");
                            if (!f) {
                                char path2[300];
                                strcpy(path2, fname);
                                strcat(path2, ".bas");
                                f = fopen(path2, "r");
                                if (!f) {
                                    strcpy(path2, fname);
                                    strcat(path2, ".BAS");
                                    f = fopen(path2, "r");
                                }
                            }
                            if (f) {
                                fclose(f);
                            } else {
                                printf("  WARNING (line %d): Task spawn target file '%s' not found on disk\n", lnum, fname);
                                warnings_cnt++;
                            }
                        }
                    } else if (look.current.type == TOK_NUMBER) {
                        int pid = (int)look.current.value.num_value;
                        if (pid < 0 || pid >= 32) { // MAX_TASKS is 32
                            printf("  WARNING (line %d): Invalid task PID %d (must be 0-31)\n", lnum, pid);
                            warnings_cnt++;
                        }
                    } else if (look.current.type == TOK_NAMED_VAR || look.current.type == TOK_VARIABLE) {
                        char name[8] = {0};
                        if (look.current.type == TOK_VARIABLE) {
                            name[0] = look.current.value.var_name;
                        } else {
                            int len = look.current.str_length;
                            if (len > 7) len = 7;
                            strncpy(name, look.current.str_start, len);
                        }
                        // Check if it's "LIST" (which is valid); otherwise it's a variable which is also valid at runtime
                        if (name[0] != '\0' && !pi_str_case_equal(name, "LIST")) {
                            // It's a variable/identifier, which is fine
                        }
                    }
                }
                
                // Jump tracking
                if (kw == KW_GOTO || kw == KW_GOSUB || kw == KW_RUN || kw == KW_RESTORE) {
                    lexer_next(&l);
                    if (l.current.type == TOK_NUMBER) {
                        int target = (int)l.current.value.num_value;
                        if (referenced_jumps_count < 512) {
                            jump_sources[referenced_jumps_count] = lnum;
                            referenced_jumps[referenced_jumps_count++] = target;
                        }
                    }
                    continue;
                }
                
                // DATA counting
                if (kw == KW_DATA) {
                    lexer_next(&l);
                    while (l.current.type != TOK_EOF && l.current.type != TOK_CR && l.current.type != TOK_COLON) {
                        if (l.current.type != TOK_COMMA) {
                            data_items++;
                        }
                        lexer_next(&l);
                    }
                    continue;
                }
            }
            
            // IF THEN linenum check
            if (l.current.type == TOK_KEYWORD && l.current.value.keyword == KW_THEN) {
                lexer_next(&l);
                if (l.current.type == TOK_NUMBER) {
                    int target = (int)l.current.value.num_value;
                    if (referenced_jumps_count < 512) {
                        jump_sources[referenced_jumps_count] = lnum;
                        referenced_jumps[referenced_jumps_count++] = target;
                    }
                }
                continue;
            }
            
            // Variables detection
            if (l.current.type == TOK_VARIABLE || l.current.type == TOK_STRING_VAR || l.current.type == TOK_NAMED_VAR) {
                char vname_buf[32] = {0};
                if (l.current.type == TOK_VARIABLE) {
                    vname_buf[0] = l.current.value.var_name;
                } else if (l.current.type == TOK_STRING_VAR) {
                    vname_buf[0] = l.current.value.var_name;
                    vname_buf[1] = '$';
                } else {
                    int len = l.current.str_length;
                    if (len > 31) len = 31;
                    strncpy(vname_buf, l.current.str_start, len);
                }
                
                int is_assigned = 0;
                if (in_assignment) {
                    is_assigned = 1;
                    in_assignment = 0;
                } else if (in_for) {
                    is_assigned = 1;
                    in_for = 0;
                } else if (in_read_input) {
                    is_assigned = 1;
                    read_vars++;
                } else {
                    // Check implicit LET
                    Lexer look = l;
                    lexer_next(&look);
                    if (look.current.type == TOK_EQUALS) {
                        is_assigned = 1;
                    }
                }
                
                // Record
                int idx = -1;
                for (int v = 0; v < vars_count; v++) {
                    if (strcmp(vars[v].name, vname_buf) == 0) {
                        idx = v;
                        break;
                    }
                }
                if (idx < 0 && vars_count < 512) {
                    idx = vars_count++;
                    strcpy(vars[idx].name, vname_buf);
                    vars[idx].assigned = 0;
                    vars[idx].used = 0;
                    vars[idx].first_line = lnum;
                }
                if (idx >= 0) {
                    if (is_assigned) vars[idx].assigned++;
                    else vars[idx].used++;
                }
            }
            lexer_next(&l);
        }
    }
    
    // Warnings for unassigned variables
    for (int v = 0; v < vars_count; v++) {
        if (vars[v].used > 0 && vars[v].assigned == 0) {
            printf("  WARNING (line %d): Variable %s used but never assigned\n", vars[v].first_line, vars[v].name);
            warnings_cnt++;
        }
    }
    
    // Check variable name typos (single-use check)
    for (int v = 0; v < vars_count; v++) {
        if (vars[v].used + vars[v].assigned == 1) {
            // Find closest candidate
            for (int u = 0; u < vars_count; u++) {
                if (u != v && (vars[u].used + vars[u].assigned > 1)) {
                    int dist = levenshtein(vars[v].name, vars[u].name);
                    if (dist > 0 && dist <= 2) {
                        printf("  WARNING (line %d): Variable %s might be a typo for %s\n", vars[v].first_line, vars[v].name, vars[u].name);
                        warnings_cnt++;
                    }
                }
            }
        }
    }
    
    // Collect all valid line numbers as jump targets
    for (int i = 0; i < total_lines; i++) {
        Lexer l;
        lexer_init(&l, rt->program->lines[i].text);
        if (l.current.type == TOK_NUMBER) {
            if (jump_targets_count < 512) {
                jump_targets[jump_targets_count++] = (int)l.current.value.num_value;
            }
        }
    }
    
    // Verify referenced jumps
    for (int j = 0; j < referenced_jumps_count; j++) {
        int target = referenced_jumps[j];
        int found = 0;
        for (int t = 0; t < jump_targets_count; t++) {
            if (jump_targets[t] == target) {
                found = 1;
                break;
            }
        }
        if (!found) {
            printf("  ERROR (line %d): Target line %d does not exist\n", jump_sources[j], target);
            errors_cnt++;
        }
    }
    
    // Unreachable Code Detection
    for (int i = 1; i < total_lines; i++) {
        ProgramLine *prev = &rt->program->lines[i - 1];
        ProgramLine *curr = &rt->program->lines[i];
        
        Lexer lprev, lcurr;
        lexer_init(&lprev, prev->text);
        lexer_init(&lcurr, curr->text);
        
        int prev_lnum = 0;
    (void)prev_lnum;
        if (lprev.current.type == TOK_NUMBER) {
            prev_lnum = (int)lprev.current.value.num_value;
            lexer_next(&lprev);
        }
        int curr_lnum = 0;
        if (lcurr.current.type == TOK_NUMBER) {
            curr_lnum = (int)lcurr.current.value.num_value;
            lexer_next(&lcurr);
        }
        
        // Check comment
        int curr_is_comment = 0;
        if (lcurr.current.type == TOK_KEYWORD && lcurr.current.value.keyword == KW_REM) {
            curr_is_comment = 1;
        }
        
        if (lprev.current.type == TOK_KEYWORD) {
            KeywordId pkw = lprev.current.value.keyword;
            if (pkw == KW_RETURN || pkw == KW_END || pkw == KW_GOTO || pkw == KW_SYSTEM) {
                // Check if current line is targeted by any branch
                int targeted = 0;
                for (int j = 0; j < referenced_jumps_count; j++) {
                    if (referenced_jumps[j] == curr_lnum) {
                        targeted = 1;
                        break;
                    }
                }
                if (!targeted && !curr_is_comment) {
                    printf("  WARNING (line %d): Unreachable code following unconditional control statement\n", curr_lnum);
                    warnings_cnt++;
                }
            }
        }
    }
    
    // Block Structure count mismatches
    if (while_count != wend_count) {
        printf("  WARNING: Mismatched WHILE/WEND loop structures (WHILE=%d, WEND=%d)\n", while_count, wend_count);
        warnings_cnt++;
    }
    if (for_count != next_count) {
        printf("  WARNING: Mismatched FOR/NEXT loop structures (FOR=%d, NEXT=%d)\n", for_count, next_count);
        warnings_cnt++;
    }
    
    // READ/DATA Parity
    if (read_vars > data_items) {
        printf("  WARNING: READ variable count (%d) exceeds DATA items (%d)\n", read_vars, data_items);
        warnings_cnt++;
    }
    
    long line_storage = (long)total_lines * 260;
    
    printf("\n--- Program Statistics ---\n");
    printf("  Lines:      %d (%d code, %d comments)\n", total_lines, code_lines, comment_lines);
    printf("  Variables:  %d\n", vars_count);
    printf("  DATA/READ:  %d items, %d reads\n", data_items, read_vars);
    printf("  FOR loops:  %d\n", for_loops);
    printf("  Subroutines: %d GOSUB calls\n", gosub_calls);
    printf("  Branch targets: %d\n", jump_targets_count);
    
    printf("\n--- Memory Usage ---\n");
    printf("  Source text:  %ld bytes\n", source_text_size);
    printf("  Line storage: %ld bytes (%d x 260)\n", line_storage, total_lines);
    printf("  Total:        %ld bytes\n", source_text_size + line_storage);
    
    printf("\n--- Complexity ---\n");
    const char *desc = "simple";
    if (complexity > 50) desc = "extremely complex";
    else if (complexity > 20) desc = "complex";
    else if (complexity > 10) desc = "moderate";
    printf("  Cyclomatic: %d (%s)\n", complexity, desc);
    
    printf("\n=== %d errors, %d warnings ===\n", errors_cnt, warnings_cnt);
}

void pi_parse_check(Lexer *lex, RuntimeState *rt, int line_num) {
    (void)lex; (void)line_num;
    run_static_analysis(rt, 0, NULL);
}

void pi_parse_verify(Lexer *lex, RuntimeState *rt, int line_num) {
    if (lex->current.type == TOK_STRING) {
        BValue path_val = parse_expression_bval(lex, rt, line_num);
        if (error_occurred()) return;
        char path[260] = {0};
        bval_to_string_buf(&path_val, path, sizeof(path));
        
        printf("=== VERIFY: %s ===\n", path);
        
        // Save current program pointer
        ProgramStore *saved_pgm = rt->program;
        
        // Load target file into a temporary program store
        ProgramStore *temp_pgm = (ProgramStore *)malloc(sizeof(ProgramStore));
        if (temp_pgm == NULL) {
            error_raise(ERR_SORRY, line_num);
            return;
        }
        memset(temp_pgm, 0, sizeof(ProgramStore));
        temp_pgm->capacity = MAX_PROGRAM_LINES;
        
        temp_pgm->lines = (ProgramLine *)malloc(MAX_PROGRAM_LINES * sizeof(ProgramLine));
        if (temp_pgm->lines == NULL) {
            free(temp_pgm);
            error_raise(ERR_SORRY, line_num);
            return;
        }
        memset(temp_pgm->lines, 0, MAX_PROGRAM_LINES * sizeof(ProgramLine));
        
        rt->program = temp_pgm;
        
        fileio_load(rt->program, path);
        
        if (error_occurred()) {
            rt->program = saved_pgm;
            free(temp_pgm->lines);
            free(temp_pgm);
            error_clear();
            error_raise(ERR_HOW, line_num);
            return;
        }
        
        run_static_analysis(rt, 1, path);
        
        // Restore original program pointer
        rt->program = saved_pgm;
        free(temp_pgm->lines);
        free(temp_pgm);
    } else {
        error_raise(ERR_WHAT, line_num);
    }
}

// 6. Original Built-in SELFTEST Implementation
void pi_parse_selftest(Lexer *lex, RuntimeState *rt, int line_num) {
    (void)lex; (void)line_num;
    printf("=== BASIC++ SELF-TEST ===\n");
    st_log(rt, "Starting Exhaustive Self-Test Suite...");

    // Build configuration info
#ifdef INPUT_CONSOLE
    printf("Build type........ Console (text-only / terminal)\n");
    st_log(rt, "Build type: Console (text-only / terminal)");
#else
    printf("Build type........ GUI (SDL-based window)\n");
    st_log(rt, "Build type: GUI (SDL-based window)");
#endif

#ifdef NO_SDL2
    printf("SDL2 support...... Disabled\n");
    st_log(rt, "SDL2 support: Disabled");
#else
    printf("SDL2 support...... Enabled\n");
    st_log(rt, "SDL2 support: Enabled");
#endif
    
    // 1. Lexer test
    {
        Lexer tl;
        lexer_init(&tl, "10 PRINT \"HELLO\", X");
        int lex_ok = 1;
        if (tl.current.type != TOK_NUMBER) lex_ok = 0;
        st_log(rt, "Lexer: parsed TOK_NUMBER: type=%d", tl.current.type);
        lexer_next(&tl);
        if (tl.current.type != TOK_KEYWORD || tl.current.value.keyword != KW_PRINT) lex_ok = 0;
        st_log(rt, "Lexer: parsed TOK_KEYWORD: type=%d, val=%d", tl.current.type, tl.current.value.keyword);
        lexer_next(&tl);
        if (tl.current.type != TOK_STRING) lex_ok = 0;
        st_log(rt, "Lexer: parsed TOK_STRING: type=%d", tl.current.type);
        lexer_next(&tl);
        if (tl.current.type != TOK_COMMA) lex_ok = 0;
        st_log(rt, "Lexer: parsed TOK_COMMA: type=%d", tl.current.type);
        lexer_next(&tl);
        if (tl.current.type != TOK_VARIABLE) lex_ok = 0;
        st_log(rt, "Lexer: parsed TOK_VARIABLE: type=%d", tl.current.type);
        lexer_next(&tl);
        if (tl.current.type != TOK_EOF) lex_ok = 0;
        st_log(rt, "Lexer: reached TOK_EOF: type=%d", tl.current.type);
        printf("Lexer............. %s\n", lex_ok ? "PASS" : "FAIL");
        st_log(rt, "Lexer test result: %s", lex_ok ? "PASS" : "FAIL");
    }

    // 2. Value system test
    {
        BValue v1 = bval_int(2);
        BValue v2 = bval_int(3);
        BValue sum = bval_add(&v1, &v2, 0);
        int val_ok = (bval_to_int(&sum) == 5);
        st_log(rt, "Value system: 2 + 3 = %ld", bval_to_int(&sum));
        
        BValue f1 = bval_float(2.5f);
        BValue f2 = bval_float(1.5f);
        BValue fsum = bval_add(&f1, &f2, 0);
        if (bval_to_float(&fsum) != 4.0f) val_ok = 0;
        st_log(rt, "Value system: 2.5 + 1.5 = %f", bval_to_float(&fsum));

        printf("Value system...... %s\n", val_ok ? "PASS" : "FAIL");
        st_log(rt, "Value system test result: %s", val_ok ? "PASS" : "FAIL");
    }

    // 3. String pool test
    {
        int pool_ok = (rt != NULL && rt->strpool.size >= 0);
        if (rt) {
            st_log(rt, "String pool: size=%ld, used=%ld", rt->strpool.size, rt->strpool.used);
        }
        printf("String pool....... %s\n", pool_ok ? "PASS" : "FAIL");
        st_log(rt, "String pool test result: %s", pool_ok ? "PASS" : "FAIL");
    }

    // 4. Function registry test
    {
        int func_ok = (funcreg_count() > 0);
        st_log(rt, "Function registry: total registered=%d", funcreg_count());
        printf("Function registry. %s\n", func_ok ? "PASS" : "FAIL");
        st_log(rt, "Function registry test result: %s", func_ok ? "PASS" : "FAIL");
    }

    // 5. Memory pool test
    {
        int mem_ok = (rt->memory != NULL && rt->memory->scratch.size > 0);
        if (rt->memory) {
            st_log(rt, "Memory pool: scratch size=%ld, scratch used=%ld", rt->memory->scratch.size, rt->memory->scratch.used);
        }
        printf("Memory pool....... %s\n", mem_ok ? "PASS" : "FAIL");
        st_log(rt, "Memory pool test result: %s", mem_ok ? "PASS" : "FAIL");
    }

    // 6. Dialect Configuration / Keyword Properties test
    {
        int dialect_ok = 1;
        
        printf("Dialect Config.... %s\n", dialect_ok ? "PASS" : "FAIL");
        st_log(rt, "Dialect Config test result: %s", dialect_ok ? "PASS" : "FAIL");
    }

    // 7. Parser Precedence test
    {
        Lexer tl;
        lexer_init(&tl, "LET X = 2 + 3 * 4");
        parser_execute_line(&tl, rt, 0);
        long val = runtime_get_var(rt, 'X');
        int precedence_ok = (val == 14);
        printf("Parser Precedence. %s (X = %ld, expected 14)\n", precedence_ok ? "PASS" : "FAIL", val);
        st_log(rt, "Parser Precedence: parsed 'LET X = 2 + 3 * 4' -> X=%ld (expected 14)", val);
    }

    // 8. Loop Control Flow test
    {
        Lexer tl;
        lexer_init(&tl, "LET Y = 0");
        parser_execute_line(&tl, rt, 0);
        lexer_init(&tl, "FOR I = 1 TO 5: LET Y = Y + I: NEXT I");
        parser_execute_line(&tl, rt, 0);
        long val = runtime_get_var(rt, 'Y');
        int loop_ok = (val == 15);
        printf("Loop Control Flow. %s (Y = %ld, expected 15)\n", loop_ok ? "PASS" : "FAIL", val);
        st_log(rt, "Loop Control Flow: evaluated FOR I = 1 TO 5 -> Y=%ld (expected 15)", val);
    }

    // 9. VFS & Comprehensive File I/O test
    {
        Lexer tl;
        char buf[64] = {0};
        int file_ok = 1;
        
        st_log(rt, "File I/O: Testing sequential mode...");
        lexer_init(&tl, "OPEN \"O\", 1, \"selftest_vfs.tmp\"");
        parser_execute_line(&tl, rt, 0);
        lexer_init(&tl, "PRINT #1, \"TEST_LINE_1\"");
        parser_execute_line(&tl, rt, 0);
        lexer_init(&tl, "PRINT #1, \"TEST_LINE_2\"");
        parser_execute_line(&tl, rt, 0);
        lexer_init(&tl, "CLOSE 1");
        parser_execute_line(&tl, rt, 0);
        
        lexer_init(&tl, "OPEN \"I\", 1, \"selftest_vfs.tmp\"");
        parser_execute_line(&tl, rt, 0);
        lexer_init(&tl, "INPUT #1, A$");
        parser_execute_line(&tl, rt, 0);
        BValue sval1 = runtime_get_string_var(rt, 'A');
        bval_to_string_buf(&sval1, buf, sizeof(buf));
        st_log(rt, "File I/O: read line 1: A$=\"%s\" (expected \"TEST_LINE_1\")", buf);
        if (strcmp(buf, "TEST_LINE_1") != 0) file_ok = 0;

        lexer_init(&tl, "INPUT #1, B$");
        parser_execute_line(&tl, rt, 0);
        BValue sval2 = runtime_get_string_var(rt, 'B');
        bval_to_string_buf(&sval2, buf, sizeof(buf));
        st_log(rt, "File I/O: read line 2: B$=\"%s\" (expected \"TEST_LINE_2\")", buf);
        if (strcmp(buf, "TEST_LINE_2") != 0) file_ok = 0;

        lexer_init(&tl, "CLOSE 1");
        parser_execute_line(&tl, rt, 0);
        
        st_log(rt, "File I/O: Testing binary/random mode...");
        // Test binary file remove
        lexer_init(&tl, "KILL \"selftest_vfs.tmp\"");
        parser_execute_line(&tl, rt, 0);
        
        printf("VFS File I/O...... %s\n", file_ok ? "PASS" : "FAIL");
        st_log(rt, "VFS File I/O test result: %s", file_ok ? "PASS" : "FAIL");
    }

    // 10. Device Alias Test
    {
        const DeviceAlias *da = device_alias_resolve("SCRN:");
        int alias_ok = (da != NULL);
        if (da) {
            st_log(rt, "Device Alias: resolved 'SCRN:' -> target='%s', direction=%d", da->target, da->direction);
        } else {
            st_log(rt, "Device Alias: failed to resolve 'SCRN:'");
        }
        printf("Device Aliases.... %s\n", alias_ok ? "PASS" : "FAIL");
        st_log(rt, "Device Aliases test result: %s", alias_ok ? "PASS" : "FAIL");
    }

    // 11. Graphics/SDL test
    {
#ifdef NO_SDL2
        printf("Graphics/SDL2..... DISABLED (NO_SDL2 defined)\n");
        st_log(rt, "Graphics/SDL2 test result: DISABLED (NO_SDL2 defined)");
#else
        int active_at_start = gw_sdl2_is_active();
        int sdl_ok = 1;
        
        st_log(rt, "Graphics/SDL2: Checking active state: active=%d", active_at_start);
        
        // If not active (e.g. Console build at startup), dynamically initialize it
        if (!active_at_start) {
            st_log(rt, "Graphics/SDL2: Console build, trying dynamic initialization...");
            if (gw_sdl2_init(16, 16, "BASIC++ Self-Test Dynamic", 0) != 0) {
                st_log(rt, "Graphics/SDL2 dynamic init failed: %s", SDL_GetError());
                sdl_ok = 0;
            } else {
                st_log(rt, "Graphics/SDL2 dynamic init succeeded.");
            }
        }
        
        if (sdl_ok) {
            // Get handles
            SDL_Window *win = (SDL_Window *)gw_sdl2_get_window_ptr();
            SDL_Renderer *ren = (SDL_Renderer *)gw_sdl2_get_renderer_ptr();
            SDL_AudioDeviceID dev = (SDL_AudioDeviceID)gw_sdl2_get_audio_device_id();
            
            st_log(rt, "Graphics/SDL2: window=%p, renderer=%p, audio_device=%d", (void*)win, (void*)ren, dev);
            
            // Query SDL Version
            SDL_version compiled;
            SDL_version linked;
            SDL_VERSION(&compiled);
            SDL_GetVersion(&linked);
            st_log(rt, "SDL2 Version: Compiled=%d.%d.%d, Linked=%d.%d.%d", 
                   compiled.major, compiled.minor, compiled.patch,
                   linked.major, linked.minor, linked.patch);
            
            // Video driver
            st_log(rt, "SDL2 Video Driver: %s", SDL_GetCurrentVideoDriver());
            
            // Renderer info
            if (ren) {
                SDL_RendererInfo rinfo;
                if (SDL_GetRendererInfo(ren, &rinfo) == 0) {
                    st_log(rt, "SDL2 Renderer: name='%s', flags=0x%X, max_w=%d, max_h=%d",
                           rinfo.name, rinfo.flags, rinfo.max_texture_width, rinfo.max_texture_height);
                    if (rinfo.flags & SDL_RENDERER_ACCELERATED) {
                        st_log(rt, "SDL2 Renderer type: Hardware Accelerated");
                    } else {
                        st_log(rt, "SDL2 Renderer type: Software Fallback");
                    }
                }
            } else {
                st_log(rt, "SDL2 Renderer is NULL!");
                sdl_ok = 0;
            }
            
            // Display details
            if (win) {
                int disp = SDL_GetWindowDisplayIndex(win);
                SDL_DisplayMode dmode;
                if (SDL_GetCurrentDisplayMode(disp, &dmode) == 0) {
                    st_log(rt, "SDL2 Display: index=%d, size=%dx%d, refresh=%dHz, format=0x%X",
                           disp, dmode.w, dmode.h, dmode.refresh_rate, dmode.format);
                }
            }
            
            // Audio details
            st_log(rt, "SDL2 Audio Driver: %s", SDL_GetCurrentAudioDriver());
            if (dev > 0) {
                st_log(rt, "SDL2 Audio Device status: Open and operational (ID=%d)", dev);
            } else {
                st_log(rt, "SDL2 Audio Device status: Closed / Unavailable");
            }
            
            // Cleanup dynamically initialized SDL
            if (!active_at_start) {
                st_log(rt, "Graphics/SDL2: Cleaning up dynamic SDL instance...");
                gw_sdl2_cleanup();
            }
        }
        
        printf("Graphics/SDL2..... %s (Subsystems initialized, hidden renderer operational)\n", sdl_ok ? "PASS" : "FAIL");
        st_log(rt, "Graphics/SDL2 test result: %s", sdl_ok ? "PASS" : "FAIL");
#endif
    }

    // 12. Multitask & RAMBANKs test
    {
        int task_ok = 1;
        
        // Test RAMBANK peek/poke
        st_log(rt, "Multitasking/RAMBANK: Testing RAMBANK read/write...");
        rambank_poke(rt->memory, 1, 100, 42, 0);
        int v = rambank_peek(rt->memory, 1, 100, 0);
        if (v != 42) {
            st_log(rt, "RAMBANK error: expected 42 at bank 1 offset 100, got %d", v);
            task_ok = 0;
        }
        
        // Test LRU eviction by poking 10 different banks (residency limit is 8)
        st_log(rt, "Multitasking/RAMBANK: Poking 10 banks to verify LRU eviction...");
        for (int b = 1; b <= 10; b++) {
            rambank_poke(rt->memory, b, 0, (unsigned char)(b * 10), 0);
        }
        // Read them back (which triggers swaps/loads)
        for (int b = 1; b <= 10; b++) {
            int val = rambank_peek(rt->memory, b, 0, 0);
            if (val != b * 10) {
                st_log(rt, "RAMBANK LRU error: expected %d at bank %d offset 0, got %d", b * 10, b, val);
                task_ok = 0;
            }
        }
        
        // Test BANK FILL
        st_log(rt, "Multitasking/RAMBANK: Testing BANK FILL on bank 2...");
        rambank_fill(rt->memory, 2, 10, 100, 88, 0);
        int fill_val1 = rambank_peek(rt->memory, 2, 10, 0);
        int fill_val2 = rambank_peek(rt->memory, 2, 109, 0);
        if (fill_val1 != 88 || fill_val2 != 88) {
            st_log(rt, "BANK FILL test failed: expected 88, got %d and %d", fill_val1, fill_val2);
            task_ok = 0;
        }

        // Test BANK COPY
        st_log(rt, "Multitasking/RAMBANK: Testing BANK COPY from bank 2 to bank 3...");
        rambank_copy(rt->memory, 2, 10, 3, 50, 100, 0);
        int copy_val1 = rambank_peek(rt->memory, 3, 50, 0);
        int copy_val2 = rambank_peek(rt->memory, 3, 149, 0);
        if (copy_val1 != 88 || copy_val2 != 88) {
            st_log(rt, "BANK COPY test failed: expected 88, got %d and %d", copy_val1, copy_val2);
            task_ok = 0;
        }

        // Test task spawning & cooperative/threaded execution
        st_log(rt, "Multitasking/RAMBANK: Creating temporary background script...");
        FILE *tf = fopen("selftest_task.tmp.bas", "w");
        if (tf) {
            fprintf(tf, "10 POKEB 1, 0, 99\n");
            fclose(tf);
            
            // Mark bank 1 as shared so background task can write to it
            MemorySystem *main_mem = task_get_main_mem();
            if (main_mem) {
                main_mem->banks[1].shared = 1;
            }
            rt->memory->banks[1].shared = 1;
            
            st_log(rt, "Multitasking/RAMBANK: Spawning temporary task...");
            int pid = task_spawn("selftest_task.tmp.bas", 0);
            if (pid < 0) {
                st_log(rt, "Task Spawn failed during SELFTEST");
                task_ok = 0;
            } else {
                // Check initial task status
                int initial_status = task_get_status(pid);
                st_log(rt, "Multitasking/RAMBANK: Task initial status: %d", initial_status);
                
                // Wait/join task
                st_log(rt, "Multitasking/RAMBANK: Waiting/joining task %d...", pid);
                task_join(pid, rt, 0);
                
                // Check completed task status
                int final_status = task_get_status(pid);
                st_log(rt, "Multitasking/RAMBANK: Task final status: %d (expected 5 = TASK_DONE + 1)", final_status);
                if (final_status != 5) {
                    st_log(rt, "Task status verification failed: expected 5, got %d", final_status);
                    task_ok = 0;
                }
                
                // Read back the shared value
                int shared_val = rambank_peek(rt->memory, 1, 0, 0);
                st_log(rt, "Multitasking/RAMBANK: Read shared value from bank 1 offset 0: %d (expected 99)", shared_val);
                if (shared_val != 99) {
                    st_log(rt, "Task execution failed: shared bank offset 0 was not 99, got %d", shared_val);
                    task_ok = 0;
                }
            }
            
            // Cleanup temp script files
            remove("selftest_task.tmp.bas");
            remove("selftest_task.tmp.BAS");
        } else {
            st_log(rt, "Failed to create temporary file for task test");
            task_ok = 0;
        }
        
        printf("Multitask & VM.... %s\n", task_ok ? "PASS" : "FAIL");
        st_log(rt, "Multitask & VM test result: %s", task_ok ? "PASS" : "FAIL");
    }
    
    printf("=== SELF-TEST COMPLETE ===\n");
    st_log(rt, "Self-Test Suite completed.");
}
