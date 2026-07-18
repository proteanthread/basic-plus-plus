/**
 * @file stmt_compat.c
 * @brief Legacy dialect compatibility statement handlers.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements SYS, GET, PAUSE, and ONERR statements to ensure unified BASIC++
 *   union mode runs legacy programs from C64, Sinclair, Apple II, and Atari BASIC.
 * - Why it exists: Enables transparent cross-dialect software execution on our unified VM
 *   without code fragmentation or explicit mode toggles.
 * - Why it works this way:
 *   - PAUSE: Delays execution for ticks or blocks indefinitely until keyboard input.
 *   - GET: Non-blocking single key press read mapped to target string or numeric variables.
 *   - SYS: Emulates C64 register states at memory locations 780-783 (A, X, Y, P) and routes
 *     to common Kernal routines (CHROUT, GETIN, CHRIN, PLOT, and RESET).
 *   - ONERR GOTO: Maps to the standard VM error trapping line configurations.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Mock Kernal addresses, delay precision timings.
 * - What cannot be changed: Virtual machine execution state modifications, sandboxed safety checks.
 * - What to expect: Blocking or non-blocking I/O dependent on simulated routine behaviors.
 * - What to do if something breaks: Trace variable assignments and check the virtual device streams.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Terminal input and keyboard query methods are portable across Windows/FreeDOS/Linux.
 * - Portability concerns: ANSI escape codes for cursor positioning assume terminal compatibility.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional BIOS/ROM mock vectors in the SYS dispatcher.
 */

#include "bpp_vm.h"
#include "bpp_lexer.h"
#include "bpp_vdev.h"
#include "bpp_eval.h"
#include "bpp_strings.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include <stdbool.h>

static bool parse_line_number(const char *str, double *out_line, const char **out_text) {
    while (*str && isspace((unsigned char)*str)) {
        str++;
    }
    if (!isdigit((unsigned char)*str)) {
        return false;
    }
    char *endptr;
    double line = strtod(str, &endptr);
    *out_line = line;
    *out_text = endptr;
    return true;
}

/* NOTE: Platform-specific headers (conio.h, unistd.h) are NOT included here.
 * All platform I/O is accessed through extern declarations below, which are
 * implemented in platform.c. This keeps statement handlers platform-independent. */

extern uint8_t vdev_bus_peek(unsigned long addr, bool *intercepted);
extern void vdev_bus_poke(unsigned long addr, uint8_t value, bool *intercepted);
extern void platform_sleep_ms(uint32_t ms);
extern bool platform_kbhit(void);
extern int  platform_getch(void);

extern int g_cursor_x;
extern int g_cursor_y;
extern int g_fg_color_idx;
extern int g_bg_color_idx;
extern int g_mouse_x;
extern int g_mouse_y;
extern int g_mouse_btn;

extern BppError stmt_cls_handler(struct VMContext *vm, struct LexerContext *lex);


BppError stmt_pause_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue dur_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (dur_val.type == VAL_STRING) {
        err.code = 13; err.message = "PAUSE duration must be numeric";
        return err;
    }

    double duration_ticks = dur_val.as.number;
    double start_time = (double)clock() / (double)CLOCKS_PER_SEC;
    /* 50 Hz clock (Spectrum standard): 1 tick = 20ms */
    double timeout = (duration_ticks > 0.0) ? (duration_ticks * 0.02) : 9999999.0;

    while (vm_is_running(vm)) {
        if (platform_kbhit()) {
            platform_getch(); /* Consume character */
            break;
        }
        double elapsed = ((double)clock() / (double)CLOCKS_PER_SEC) - start_time;
        if (duration_ticks > 0.0 && elapsed >= timeout) {
            break;
        }
        platform_sleep_ms(10);
    }

    return err;
}

BppError stmt_kbd_get_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected variable name in GET";
        return err;
    }

    char var_name[256];
    size_t copy_len = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
    memcpy(var_name, tok.as.string, copy_len);
    var_name[copy_len] = '\0';

    VariableContext *var_ctx = vm_get_var(vm);
    BValue *var = var_lookup(var_ctx, var_name, true);
    if (!var) {
        err.code = 2; err.message = "Undeclared variable in GET";
        return err;
    }

    int key = 0;
    if (platform_kbhit()) {
        key = platform_getch();
    }

    BValue val;
    memset(&val, 0, sizeof(val));

    if (var_name[strlen(var_name) - 1] == '$') {
        val.type = VAL_STRING;
        char buf[2] = "";
        if (key != 0) {
            buf[0] = (char)key;
        }
        val.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    } else {
        val.type = VAL_NUMBER;
        val.as.number = (double)key;
    }

    var_assign(var_ctx, var_name, val);
    if (val.type == VAL_STRING && val.as.string) {
        str_release(vm_get_str(vm), val.as.string);
    }
    return err;
}

BppError stmt_sys_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue addr_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (addr_val.type == VAL_STRING) {
        err.code = 13; err.message = "SYS address must be numeric";
        return err;
    }

    unsigned long addr = (unsigned long)addr_val.as.number;

    /* Read registers from virtual memory locations 780-783 */
    bool dummy;
    uint8_t a = vdev_bus_peek(780, &dummy);
    uint8_t x = vdev_bus_peek(781, &dummy);
    uint8_t y = vdev_bus_peek(782, &dummy);
    uint8_t p = vdev_bus_peek(783, &dummy);

    /* Dispatch mock C64 Kernal functions */
    switch (addr) {
        case 65490: /* $FFD2 - CHROUT */
            vdev_putc(vm_get_vdev(vm), (char)a);
            break;

        case 65508: /* $FFE4 - GETIN */
            {
                int ch = platform_kbhit() ? platform_getch() : 0;
                a = (uint8_t)ch;
            }
            break;

        case 65487: /* $FFCF - CHRIN */
            {
                int ch = platform_getch();
                a = (uint8_t)ch;
            }
            break;

        case 65520: /* $FFF0 - PLOT */
            if ((p & 0x01) == 0) {
                /* Clear carry flag: set cursor coordinates (row X, col Y) */
                vdev_printf(vm_get_vdev(vm), "\033[%d;%dH", x + 1, y + 1);
            } else {
                /* Set carry flag: read cursor coordinates (return in X and Y) */
                /* Cursor query is mocked to return current registry states */
            }
            break;

        case 64738: /* $FCE2 - RESET */
            /* Clear variables and run NEW */
            var_clear_all(vm_get_var(vm));
            arr_clear_all(vm_get_arr(vm));
            vm_jump(vm, 0, NULL);
            break;

        default:
            /* Log unhandled mock execution */
            vdev_printf(vm_get_vdev(vm), "[SYS CALL] Executed address %lu with A=%d X=%d Y=%d P=%d\n", addr, a, x, y, p);
            break;
    }

    /* Write register values back to RAM */
    vdev_bus_poke(780, a, &dummy);
    vdev_bus_poke(781, x, &dummy);
    vdev_bus_poke(782, y, &dummy);
    vdev_bus_poke(783, p, &dummy);

    return err;
}

BppError stmt_onerr_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD || tok.as.keyword != KW_GOTO) {
        err.code = 2; err.message = "Expected 'GOTO' after ONERR";
        return err;
    }

    BValue line_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (line_val.type == VAL_STRING) {
        err.code = 13; err.message = "ONERR destination line must be numeric";
        return err;
    }

    /* Set standard error trap line on the VM */
    vm_set_error_trap(vm, (BppLineNumber)line_val.as.number);
    return err;
}

BppError stmt_auto_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex);
        (void)eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        tok = lex_peek(lex);
        if (tok.type == TOK_RPAREN) {
            lex_next(lex);
        }
    } else if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_COMMA) {
        (void)eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
    }
    
    return err;
}

BppError stmt_bcolor_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex);
    }
    
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "BCOLOR expects a numeric color value";
        return err;
    }
    
    tok = lex_peek(lex);
    if (tok.type == TOK_RPAREN) {
        lex_next(lex);
    }
    
    int color = (int)val.as.number;
    if (color >= 0 && color < 256) {
        g_bg_color_idx = color;
        VDevContext *vdev_ctx = vm_get_vdev(vm);
        if (vdev_ctx) {
            static const int ansi_bg_map[16] = {40,44,42,46,41,45,43,47,100,104,102,106,101,105,103,107};
            int bg_ansi = (color < 16) ? ansi_bg_map[color] : color;
            char esc[64];
            if (color < 16) {
                snprintf(esc, sizeof(esc), "\033[%dm", bg_ansi);
            } else {
                snprintf(esc, sizeof(esc), "\033[48;5;%dm", color);
            }
            vdev_puts(vdev_ctx, esc);
        }
    }
    return err;
}

BppError stmt_fcolor_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex);
    }
    
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "FCOLOR expects a numeric color value";
        return err;
    }
    
    tok = lex_peek(lex);
    if (tok.type == TOK_RPAREN) {
        lex_next(lex);
    }
    
    int color = (int)val.as.number;
    if (color >= 0 && color < 256) {
        g_fg_color_idx = color;
        VDevContext *vdev_ctx = vm_get_vdev(vm);
        if (vdev_ctx) {
            static const int ansi_fg_map[16] = {30,34,32,36,31,35,33,37,90,94,92,96,91,95,93,97};
            int fg_ansi = (color < 16) ? ansi_fg_map[color] : color;
            char esc[64];
            if (color < 16) {
                snprintf(esc, sizeof(esc), "\033[%dm", fg_ansi);
            } else {
                snprintf(esc, sizeof(esc), "\033[38;5;%dm", color);
            }
            vdev_puts(vdev_ctx, esc);
        }
    }
    return err;
}

BppError stmt_clear_handler(VMContext *vm, LexerContext *lex) {
#ifndef BPP_LITE_BUILD
    return stmt_cls_handler(vm, lex);
#else
    BppError err;
    memset(&err, 0, sizeof(err));
    VDevContext *vdev_ctx = vm_get_vdev(vm);
    if (vdev_ctx) {
        vdev_puts(vdev_ctx, "\033[2J\033[H");
    }
    return err;
#endif
}

BppError stmt_locate_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int row = -1;
    int col = -1;

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_COMMA && tok.type != TOK_EOL && tok.type != TOK_EOF) {
        BValue row_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        row = (int)row_val.as.number;
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type != TOK_COMMA && tok.type != TOK_EOL && tok.type != TOK_EOF) {
            BValue col_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            col = (int)col_val.as.number;
        }
    }

    if (row > 0) g_cursor_y = row - 1;
    if (col > 0) g_cursor_x = col - 1;

    VDevContext *vdev_ctx = vm_get_vdev(vm);
    if (vdev_ctx) {
        char esc[64];
        snprintf(esc, sizeof(esc), "\033[%d;%dH", g_cursor_y + 1, g_cursor_x + 1);
        vdev_puts(vdev_ctx, esc);
    }

    return err;
}

BppError stmt_cursor_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex);
    }

    BValue col_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int col = (int)col_val.as.number;

    int row = -1;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue row_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        row = (int)row_val.as.number;
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_RPAREN) {
        lex_next(lex);
    }

    g_cursor_x = col;
    if (row >= 0) g_cursor_y = row;

    VDevContext *vdev_ctx = vm_get_vdev(vm);
    if (vdev_ctx) {
        char esc[64];
        snprintf(esc, sizeof(esc), "\033[%d;%dH", g_cursor_y + 1, g_cursor_x + 1);
        vdev_puts(vdev_ctx, esc);
    }

    return err;
}

#ifdef BPP_LITE_BUILD
int platform_mouse_x(void) { return 0; }
int platform_mouse_y(void) { return 0; }
int platform_mouse_btn(void) { return 0; }
int platform_inkey_char(void) {
    if (platform_kbhit()) {
        return platform_getch();
    }
    return 0;
}
#endif
#include "bpp_segmented_mem.h"

BppError stmt_bsave_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue file_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (file_val.type != VAL_STRING || !file_val.as.string) {
        err.code = 13; err.message = "Type mismatch: BSAVE expects filename string";
        if (file_val.type == VAL_STRING && file_val.as.string) str_release(vm_get_str(vm), file_val.as.string);
        return err;
    }

    const char *filename = str_data(file_val.as.string);
    char path[256];
    strncpy(path, filename, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    str_release(vm_get_str(vm), file_val.as.string);

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' after filename in BSAVE";
        return err;
    }
    lex_next(lex);

    BValue offset_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (offset_val.type == VAL_STRING) {
        str_release(vm_get_str(vm), offset_val.as.string);
        err.code = 13; err.message = "Offset must be numeric"; return err;
    }
    int offset = (int)offset_val.as.number;

    tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' after offset in BSAVE";
        return err;
    }
    lex_next(lex);

    BValue len_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (len_val.type == VAL_STRING) {
        str_release(vm_get_str(vm), len_val.as.string);
        err.code = 13; err.message = "Length must be numeric"; return err;
    }
    int length = (int)len_val.as.number;

    if (length <= 0) {
        err.code = 5; err.message = "Illegal function call: BSAVE length <= 0";
        return err;
    }

    FILE *fp = fopen(path, "wb");
    if (!fp) {
        err.code = 53; err.message = "File not found / Cannot create file";
        return err;
    }

    /* Dump memory */
#ifndef BPP_LITE_BUILD
    VMemContext *vmem = vm_get_vmem(vm);
    for (int i = 0; i < length; ++i) {
        uint8_t b = 0;
        vmem_peek(vmem, (uint16_t)(offset + i), &b);
        fputc(b, fp);
    }
#else
    (void)offset;
    (void)length;
#endif
    fclose(fp);

    return err;
}

BppError stmt_bload_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue file_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (file_val.type != VAL_STRING || !file_val.as.string) {
        err.code = 13; err.message = "Type mismatch: BLOAD expects filename string";
        if (file_val.type == VAL_STRING && file_val.as.string) str_release(vm_get_str(vm), file_val.as.string);
        return err;
    }

    const char *filename = str_data(file_val.as.string);
    char path[256];
    strncpy(path, filename, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    str_release(vm_get_str(vm), file_val.as.string);

    int offset = -1;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue offset_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (offset_val.type == VAL_STRING) {
            str_release(vm_get_str(vm), offset_val.as.string);
            err.code = 13; err.message = "Offset must be numeric"; return err;
        }
        offset = (int)offset_val.as.number;
    }

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        err.code = 53; err.message = "File not found";
        return err;
    }

    /* Memory inject */
#ifndef BPP_LITE_BUILD
    VMemContext *vmem = vm_get_vmem(vm);
    int current_offset = offset >= 0 ? offset : 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        vmem_poke(vmem, (uint16_t)current_offset, (uint8_t)ch);
        current_offset++;
    }
#else
    (void)offset;
#endif
    fclose(fp);

    return err;
}

BppError stmt_brun_handler(VMContext *vm, LexerContext *lex) {
    BppError err = stmt_bload_handler(vm, lex);
    if (err.code != 0) return err;

    /* Issue RUN */
    vm_run_program(vm);
    return err;
}
BppError stmt_renum_handler(VMContext *vm, LexerContext *lex) { BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_delete_handler(VMContext *vm, LexerContext *lex) { BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError vm_load_program_file(VMContext *vm, const char *filename) {
    BppError err;
    memset(&err, 0, sizeof(err));
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        err.code = 53; err.message = "File not found";
        return err;
    }
    MemoryContext *mem = vm_get_mem(vm);
    var_clear_all(vm_get_var(vm));
    mem_program_clear(mem);
    char line_buf[1024];
    while (fgets(line_buf, sizeof(line_buf), fp)) {
        size_t slen = strlen(line_buf);
        while (slen > 0 && isspace((unsigned char)line_buf[slen - 1])) {
            line_buf[slen - 1] = '\0';
            slen--;
        }
        char *ptr = line_buf;
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (*ptr == '\0') continue;
        BppLineNumber line_num = 0.0;
        const char *stmt_text = NULL;
        if (parse_line_number(ptr, &line_num, &stmt_text)) {
            while (*stmt_text && isspace((unsigned char)*stmt_text)) stmt_text++;
            if (*stmt_text != '\0') {
                mem_program_insert(mem, line_num, stmt_text);
            }
        }
    }
    fclose(fp);
    return err;
}
BppError vm_load_library_file(VMContext *vm, const char *filename) { BppError err; memset(&err, 0, sizeof(err)); return err; }




BppError vm_bload_program_from_stream(VMContext *vm, FILE *fp) {
    BppError err; memset(&err, 0, sizeof(err));
    return err;
}

BppError vm_bload_program_file_bpp(VMContext *vm, const char *filename) {
    BppError err; memset(&err, 0, sizeof(err));
    return err;
}

static DetokenizerFn g_detok = NULL;

void bytecode_set_detokenizer(DetokenizerFn fn) {
    g_detok = fn;
}

DetokenizerFn bytecode_get_detokenizer(void) {
    return g_detok;
}

