/**
 * @file stmt_io_devices.c
 * @brief Phase 11b and 11c standard I/O and hardware devices handlers.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements statement and modern namespace function handlers for Sound, Mouse, Joystick,
 *   Clipboard, Window, Networking, Image, Compression, and Session subsystems.
 * - Why it exists: Combines all virtual device and abstract hardware layers into VM-accessible syntax.
 * - Why it works this way: It parses legacy syntax keywords and registers namespaces using the standard
 *   function registry to support both retro and modern BASIC programs.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Modern namespace function names, argument bounds, error descriptions.
 * - What cannot be changed: Statement parser signatures, keyword checks.
 * - What to expect: Changes here immediately alter the VM vocabulary for compiled/transpiled scripts.
 * - What to do if something breaks: Trace parser output, verify symbol names in help/catalog lists.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Relies on platform abstractions and standard C library.
 * - Portability concerns: Sockets and SDL2 are gated appropriately to allow lite builds.
 */

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "platform/platform.h"
#include "security/security.h"
#include "eval/eval.h"
#include "runtime/funcreg.h"
#include "module/module.h"
#include "runtime/strings.h"
#include "runtime/vnet.h"
#include "runtime/state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Compression LZ77 Forward Declarations */
extern unsigned char *lz77_compress(const unsigned char *src, size_t src_len, size_t *out_len);
extern unsigned char *lz77_decompress(const unsigned char *src, size_t src_len, size_t *out_len);

/* Platform / Gfx sound/mouse/joystick/window/image externs */
extern int vdev_sound_open(const char *filename);
extern void vdev_sound_play(int handle);
extern void vdev_sound_loop(int handle);
extern void vdev_sound_stop(int handle);
extern void vdev_sound_pause(int handle);
extern void vdev_sound_volume(int handle, double vol);
extern double vdev_sound_length(int handle);
extern double vdev_sound_position(int handle);
extern void vdev_sound_noise(int type, double dur_seconds);
extern void vdev_play_sound_freq(double freq, double duration_seconds);
extern void vdev_music_clear(void);
extern int vdev_music_queue_length(void);
extern void play_mml(VMContext *vm, const char *mml);

extern int platform_mouse_query(int n);
extern bool platform_mouse_input(void);
extern int platform_mouse_wheel(void);
extern void platform_mouse_hide(void);
extern void platform_mouse_show(void);

extern int platform_joystick_count(void);
extern const char *platform_joystick_name(int index);
extern double platform_joystick_axis(int stick_idx, int axis_idx);
extern int platform_joystick_button(int stick_idx, int button_idx);

extern void platform_window_title_set(const char *title);
extern void platform_window_move(int x, int y);
extern void platform_window_fullscreen(void);
extern int platform_window_x(void);
extern int platform_window_y(void);
extern int platform_desktop_width(void);
extern int platform_desktop_height(void);
extern void platform_window_resizable(bool resizable);
extern void platform_window_icon_set(const char *filename);

extern int vdev_image_load(const char *filename);
extern void vdev_image_free(int handle);
extern void vdev_image_draw(int handle, int x, int y);
extern int vdev_image_create(int w, int h, int mode);
extern int vdev_image_copy(int handle);
extern int vdev_image_width(int handle);
extern int vdev_image_height(int handle);

/* Standard Statement Checkers & Helpers */
static BppError check_vdev_permission(void) {
    BppError err;
    memset(&err, 0, sizeof(err));
#ifndef BPP_LITE_BUILD
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: Virtual device access blocked under sandbox settings";
    }
#endif
    return err;
}

/* =========================================================================
 * 11b.1: SOUND SYSTEM
 * ========================================================================= */

BppError stmt_noise_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue type_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (type_val.type == VAL_STRING && type_val.as.string) {
        str_release(vm_get_str(vm), type_val.as.string);
    }
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in NOISE statement";
        return err;
    }

    BValue dur_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (dur_val.type == VAL_STRING && dur_val.as.string) {
        str_release(vm_get_str(vm), dur_val.as.string);
    }
    if (type_val.type != VAL_NUMBER && type_val.type != VAL_INTEGER) {
        err.code = 13; err.message = "Type mismatch: NOISE type must be numeric";
        return err;
    }
    if (dur_val.type != VAL_NUMBER && dur_val.type != VAL_INTEGER) {
        err.code = 13; err.message = "Type mismatch: NOISE duration must be numeric";
        return err;
    }
    int type = (int)type_val.as.number;
    double dur = dur_val.as.number;
    vdev_sound_noise(type, dur);

    return err;
}

BppError stmt_sndplay_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue handle_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    vdev_sound_play((int)handle_val.as.number);
    return err;
}

BppError stmt_sndloop_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue handle_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    vdev_sound_loop((int)handle_val.as.number);
    return err;
}

BppError stmt_sndstop_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue handle_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    vdev_sound_stop((int)handle_val.as.number);
    return err;
}

BppError stmt_sndpause_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue handle_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    vdev_sound_pause((int)handle_val.as.number);
    return err;
}

BppError stmt_sndvol_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue handle_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in _SNDVOL statement";
        return err;
    }

    BValue vol_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    vdev_sound_volume((int)handle_val.as.number, vol_val.as.number);
    return err;
}

/* Sound Functions */
static BValue fn_sound_open(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    const char *file = str_data(args[0].as.string);
    return bval_float(vdev_sound_open(file));
}

static BValue fn_sound_play(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    vdev_sound_play((int)bval_to_float(&args[0]));
    return bval_float(1.0);
}

static BValue fn_sound_loop(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    vdev_sound_loop((int)bval_to_float(&args[0]));
    return bval_float(1.0);
}

static BValue fn_sound_stop(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    vdev_sound_stop((int)bval_to_float(&args[0]));
    return bval_float(1.0);
}

static BValue fn_sound_pause(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    vdev_sound_pause((int)bval_to_float(&args[0]));
    return bval_float(1.0);
}

static BValue fn_sound_volume(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    vdev_sound_volume((int)bval_to_float(&args[0]), bval_to_float(&args[1]));
    return bval_float(1.0);
}

static BValue fn_sound_length(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    return bval_float(vdev_sound_length((int)bval_to_float(&args[0])));
}

static BValue fn_sound_position(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    return bval_float(vdev_sound_position((int)bval_to_float(&args[0])));
}

static BValue fn_sound_noise(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    vdev_sound_noise((int)bval_to_float(&args[0]), bval_to_float(&args[1]));
    return bval_float(1.0);
}

static BValue fn_sound_tone(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    vdev_play_sound_freq(bval_to_float(&args[0]), bval_to_float(&args[1]));
    return bval_float(1.0);
}

static BValue fn_music_play(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    const char *mml = str_data(args[0].as.string);
    play_mml(vm, mml);
    return bval_float(1.0);
}

/* =========================================================================
 * 11b.2: MOUSE & JOYSTICK & DEVICES INPUT
 * ========================================================================= */
#include "module/module.h"

extern void platform_mouse_get_position(int *col, int *row);
extern void platform_mouse_set_position(int col, int row);
extern void platform_mouse_enable(bool enable);
extern void platform_mouse_set_cursor(int char_code, int attrib);
extern void vm_set_mouse_trap(VMContext *vm, int state, BppLineNumber line, int target_char);
extern void vm_set_hmouse_trap(VMContext *vm, int state, BppLineNumber line);
extern void vm_set_vmouse_trap(VMContext *vm, int state, BppLineNumber line);
extern void vm_set_trig_trap(VMContext *vm, int state, BppLineNumber line, int target_btn);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BPP_LITE_BUILD
extern void *sdl_window;
#endif

BppError stmt_mouseinput_handler(VMContext *vm, LexerContext *lex) {
    (void)vm; (void)lex;
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    platform_mouse_input();
    return err;
}

BppError stmt_hmouse_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD) {
        lex_next(lex);
        if (tok.as.keyword == KW_ON) {
            vm_set_hmouse_trap(vm, 1, -1.0);
        } else if (tok.as.keyword == KW_OFF) {
            vm_set_hmouse_trap(vm, 0, -1.0);
        } else if (tok.as.keyword == KW_STOP) {
            vm_set_hmouse_trap(vm, 2, -1.0);
        } else {
            err.code = 2; err.message = "Expected ON, OFF, or STOP after HMOUSE";
        }
        return err;
    }
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "HMOUSE position must be numeric";
        return err;
    }
    int col = (int)val.as.number;
    int cur_col = 1, cur_row = 1;
    platform_mouse_get_position(&cur_col, &cur_row);
    platform_mouse_set_position(col, cur_row);
    return err;
}

BppError stmt_vmouse_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD) {
        lex_next(lex);
        if (tok.as.keyword == KW_ON) {
            vm_set_vmouse_trap(vm, 1, -1.0);
        } else if (tok.as.keyword == KW_OFF) {
            vm_set_vmouse_trap(vm, 0, -1.0);
        } else if (tok.as.keyword == KW_STOP) {
            vm_set_vmouse_trap(vm, 2, -1.0);
        } else {
            err.code = 2; err.message = "Expected ON, OFF, or STOP after VMOUSE";
        }
        return err;
    }
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "VMOUSE position must be numeric";
        return err;
    }
    int row = (int)val.as.number;
    int cur_col = 1, cur_row = 1;
    platform_mouse_get_position(&cur_col, &cur_row);
    platform_mouse_set_position(cur_col, row);
    return err;
}

BppError stmt_mouse_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD) {
        lex_next(lex);
        if (tok.as.keyword == KW_ON) {
            platform_mouse_enable(true);
            vm_set_mouse_trap(vm, 1, -1.0, -2);
        } else if (tok.as.keyword == KW_OFF) {
            platform_mouse_enable(false);
            vm_set_mouse_trap(vm, 0, -1.0, -2);
        } else if (tok.as.keyword == KW_STOP) {
            vm_set_mouse_trap(vm, 2, -1.0, -2);
        } else {
            err.code = 2; err.message = "Expected ON, OFF, or STOP after MOUSE";
        }
        return err;
    }
    
    BValue val1 = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue val2 = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue val3 = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            
            BppToken comma2 = lex_next(lex);
            if (comma2.type != TOK_COMMA) {
                err.code = 2; err.message = "Expected ',' in MOUSE statement";
                return err;
            }
            BValue val4 = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            
            platform_mouse_set_position((int)val1.as.number, (int)val2.as.number);
            platform_mouse_set_cursor((int)val3.as.number, (int)val4.as.number);
        } else {
            platform_mouse_set_cursor((int)val1.as.number, (int)val2.as.number);
        }
    } else {
        err.code = 2; err.message = "Expected ON, OFF, or coordinate/cursor values after MOUSE";
    }
    
    return err;
}

BppError stmt_trig_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD) {
        lex_next(lex);
        if (tok.as.keyword == KW_ON) {
            vm_set_trig_trap(vm, 1, -1.0, -2);
        } else if (tok.as.keyword == KW_OFF) {
            vm_set_trig_trap(vm, 0, -1.0, -2);
        } else if (tok.as.keyword == KW_STOP) {
            vm_set_trig_trap(vm, 2, -1.0, -2);
        } else {
            err.code = 2; err.message = "Expected ON, OFF, or STOP after TRIG";
        }
        return err;
    }
    err.code = 2; err.message = "Expected ON, OFF, or STOP after TRIG";
    return err;
}

BppError stmt_mousehide_handler(VMContext *vm, LexerContext *lex) {
    (void)vm; (void)lex;
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    platform_mouse_hide();
    return err;
}

BppError stmt_mouseshow_handler(VMContext *vm, LexerContext *lex) {
    (void)vm; (void)lex;
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    platform_mouse_show();
    return err;
}

static BValue fn_mouse_query(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    return bval_float(platform_mouse_query((int)bval_to_float(&args[0])));
}

static BValue fn_mouse_x(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(platform_mouse_query(1));
}

static BValue fn_mouse_y(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(platform_mouse_query(2));
}

static BValue fn_mouse_button(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    int btn = (int)bval_to_float(&args[0]);
    if (btn == 1) return bval_float(platform_mouse_query(3));
    if (btn == 2) return bval_float(platform_mouse_query(4));
    if (btn == 3) return bval_float(platform_mouse_query(5));
    return bval_float(0);
}

static BValue fn_mouse_poll(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(platform_mouse_input() ? -1.0 : 0.0);
}

static BValue fn_mouse_wheel(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(platform_mouse_wheel());
}

static BValue fn_mouse_show(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    platform_mouse_show();
    return bval_float(1.0);
}

static BValue fn_mouse_hide(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    platform_mouse_hide();
    return bval_float(1.0);
}

static BValue fn_joystick_axis(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    return bval_float(platform_joystick_axis((int)bval_to_float(&args[0]), (int)bval_to_float(&args[1])));
}

static BValue fn_joystick_button(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    return bval_float(platform_joystick_button((int)bval_to_float(&args[0]), (int)bval_to_float(&args[1])));
}

static BValue fn_input_device_count(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(platform_joystick_count());
}

static BValue fn_input_device_name(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    int idx = (int)bval_to_float(&args[0]);
    const char *name = platform_joystick_name(idx);
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), name, strlen(name));
    return res;
}

/* =========================================================================
 * 11b.3: CLIPBOARD ACCESS
 * ========================================================================= */

static BValue fn_clipboard_text(BValue *args, int argc, void *rt) {
    (void)args; (void)argc;
    VMContext *vm = (VMContext *)rt;
    char *clip = platform_clipboard_get();
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), clip ? clip : "", clip ? strlen(clip) : 0);
    if (clip) free(clip);
    return res;
}

static BValue fn_clipboard_set(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    const char *text = str_data(args[0].as.string);
    platform_clipboard_set(text ? text : "");
    return bval_float(1.0);
}

/* =========================================================================
 * 11b.4: WINDOW MANAGEMENT
 * ========================================================================= */

BppError stmt_title_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type != VAL_STRING || !val.as.string) {
        err.code = 13; err.message = "Type mismatch: TITLE expects a string";
        return err;
    }
    platform_window_title_set(str_data(val.as.string));
    str_release(vm_get_str(vm), val.as.string);
    return err;
}

BppError stmt_screenmove_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue x_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in _SCREENMOVE statement";
        return err;
    }

    BValue y_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    platform_window_move((int)x_val.as.number, (int)y_val.as.number);
    return err;
}

BppError stmt_fullscreen_handler(VMContext *vm, LexerContext *lex) {
    (void)vm; (void)lex;
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    platform_window_fullscreen();
    return err;
}

BppError stmt_resize_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    bool enable = true;
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_OFF) {
        enable = false;
    }

    platform_window_resizable(enable);
    return err;
}

BppError stmt_icon_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type != VAL_STRING || !val.as.string) {
        err.code = 13; err.message = "Type mismatch: ICON expects a string";
        return err;
    }
    platform_window_icon_set(str_data(val.as.string));
    str_release(vm_get_str(vm), val.as.string);
    return err;
}

static BValue fn_window_title(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    platform_window_title_set(str_data(args[0].as.string));
    return bval_float(1.0);
}

static BValue fn_window_move(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    platform_window_move((int)bval_to_float(&args[0]), (int)bval_to_float(&args[1]));
    return bval_float(1.0);
}

static BValue fn_window_fullscreen(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    platform_window_fullscreen();
    return bval_float(1.0);
}

static BValue fn_window_resizable(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    platform_window_resizable(bval_to_float(&args[0]) != 0.0);
    return bval_float(1.0);
}

static BValue fn_window_icon(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    platform_window_icon_set(str_data(args[0].as.string));
    return bval_float(1.0);
}

static BValue fn_screen_width(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(platform_desktop_width());
}

static BValue fn_screen_height(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(platform_desktop_height());
}

/* =========================================================================
 * 11c.1: TCP/IP NETWORKING
 * ========================================================================= */

BppError stmt_nwrite_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type == VAL_STRING && ch_val.as.string) {
        str_release(vm_get_str(vm), ch_val.as.string);
    }
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in NWRITE";
        return err;
    }

    BValue data_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
        if (data_val.type == VAL_STRING && data_val.as.string) str_release(vm_get_str(vm), data_val.as.string);
        err.code = 13; err.message = "Type mismatch: NWRITE channel must be numeric";
        return err;
    }
    if (data_val.type != VAL_STRING || !data_val.as.string) {
        err.code = 13; err.message = "Type mismatch: NWRITE data must be a string";
        return err;
    }
    int channel = (int)ch_val.as.number;
    const char *data = str_data(data_val.as.string);
    size_t len = data ? strlen(data) : 0;

    err = vnet_send(vm_get_vnet(vm), channel, data, len);
    str_release(vm_get_str(vm), data_val.as.string);
    return err;
}

static BValue fn_net_connect(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    const char *host = str_data(args[0].as.string);
    int port = (int)bval_to_float(&args[1]);

    /* Automatically find and return an open channel slot */
    int slot = -1;
    for (int i = 1; i < VNET_MAX_CHANNELS; i++) {
        if (vnet_status(vm_get_vnet(vm), i) == 0) {
            slot = i;
            break;
        }
    }

    if (slot != -1) {
        BppError err = vnet_open(vm_get_vnet(vm), slot, "TCP", host, port);
        if (err.code == 0) {
            return bval_float(slot);
        }
    }
    return bval_float(-1.0);
}

static BValue fn_net_listen(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    int port = (int)bval_to_float(&args[0]);

    int slot = -1;
    for (int i = 1; i < VNET_MAX_CHANNELS; i++) {
        if (vnet_status(vm_get_vnet(vm), i) == 0) {
            slot = i;
            break;
        }
    }

    if (slot != -1) {
        BppError err = vnet_open_host(vm_get_vnet(vm), slot, port);
        if (err.code == 0) {
            return bval_float(slot);
        }
    }
    return bval_float(-1.0);
}

static BValue fn_net_accept(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    int listen_ch = (int)bval_to_float(&args[0]);

    int client_ch = -1;
    for (int i = 1; i < VNET_MAX_CHANNELS; i++) {
        if (vnet_status(vm_get_vnet(vm), i) == 0) {
            client_ch = i;
            break;
        }
    }

    if (client_ch != -1) {
        char ip[64] = {0};
        BppError err = vnet_accept(vm_get_vnet(vm), listen_ch, client_ch, ip, sizeof(ip));
        if (err.code == 0 && vnet_status(vm_get_vnet(vm), client_ch) > 0) {
            return bval_float(client_ch);
        }
    }
    return bval_float(-1.0);
}

static BValue fn_net_connected(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    int channel = (int)bval_to_float(&args[0]);
    return bval_float(vnet_connected(vm_get_vnet(vm), channel) ? -1.0 : 0.0);
}

static BValue fn_net_address(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    int channel = (int)bval_to_float(&args[0]);
    const char *addr = vnet_address(vm_get_vnet(vm), channel);
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), addr ? addr : "", addr ? strlen(addr) : 0);
    return res;
}

static BValue fn_net_send(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    int channel = (int)bval_to_float(&args[0]);
    const char *data = str_data(args[1].as.string);
    size_t len = data ? strlen(data) : 0;
    BppError err = vnet_send(vm_get_vnet(vm), channel, data, len);
    return bval_float(err.code == 0 ? 1.0 : 0.0);
}

static BValue fn_net_recv(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    int channel = (int)bval_to_float(&args[0]);
    int max_len = (int)bval_to_float(&args[1]);

    if (max_len <= 0) max_len = 1024;
    char *buf = (char *)calloc(1, max_len + 1);
    BValue res;
    res.type = VAL_STRING;

    if (buf) {
        size_t out_len = 0;
        BppError err = vnet_recv(vm_get_vnet(vm), channel, buf, max_len, &out_len);
        if (err.code == 0 && out_len > 0) {
            buf[out_len] = '\0';
            res.as.string = str_create(vm_get_str(vm), buf, out_len);
        } else {
            res.as.string = str_create(vm_get_str(vm), "", 0);
        }
        free(buf);
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }
    return res;
}

/* =========================================================================
 * 11c.2: IMAGE LOADING
 * ========================================================================= */

BppError stmt_freeimage_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue handle_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    vdev_image_free((int)handle_val.as.number);
    return err;
}

BppError stmt_putimage_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    /* Syntax: _PUTIMAGE (x, y), handle */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_LPAREN) {
        err.code = 2; err.message = "Expected '(' in _PUTIMAGE statement";
        return err;
    }

    BValue x_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in coordinate pair";
        return err;
    }

    BValue y_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_RPAREN) {
        err.code = 2; err.message = "Expected ')' in coordinate pair";
        return err;
    }

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' before handle in _PUTIMAGE statement";
        return err;
    }

    BValue handle_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    vdev_image_draw((int)handle_val.as.number, (int)x_val.as.number, (int)y_val.as.number);
    return err;
}

static BValue fn_image_load(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    return bval_float(vdev_image_load(str_data(args[0].as.string)));
}

static BValue fn_image_free(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    vdev_image_free((int)bval_to_float(&args[0]));
    return bval_float(1.0);
}

static BValue fn_image_draw(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    vdev_image_draw((int)bval_to_float(&args[0]), (int)bval_to_float(&args[1]), (int)bval_to_float(&args[2]));
    return bval_float(1.0);
}

static BValue fn_image_create(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    return bval_float(vdev_image_create((int)bval_to_float(&args[0]), (int)bval_to_float(&args[1]), (int)bval_to_float(&args[2])));
}

static BValue fn_image_copy(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    return bval_float(vdev_image_copy((int)bval_to_float(&args[0])));
}

static BValue fn_image_width(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    return bval_float(vdev_image_width((int)bval_to_float(&args[0])));
}

static BValue fn_image_height(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    return bval_float(vdev_image_height((int)bval_to_float(&args[0])));
}

/* =========================================================================
 * 11c.3: COMPRESSION
 * ========================================================================= */

static BValue fn_compress_deflate(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    const char *data = str_data(args[0].as.string);
    size_t len = data ? strlen(data) : 0;

    size_t out_len = 0;
    unsigned char *comp = lz77_compress((const unsigned char *)data, len, &out_len);
    BValue res;
    res.type = VAL_STRING;
    if (comp) {
        res.as.string = str_create(vm_get_str(vm), (const char *)comp, out_len);
        free(comp);
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }
    return res;
}

static BValue fn_compress_inflate(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    const char *data = str_data(args[0].as.string);
    size_t len = args[0].as.string ? str_len(args[0].as.string) : 0; /* Wait, string length from pool since it can contain binary nulls! */

    size_t out_len = 0;
    unsigned char *decomp = lz77_decompress((const unsigned char *)data, len, &out_len);
    BValue res;
    res.type = VAL_STRING;
    if (decomp) {
        res.as.string = str_create(vm_get_str(vm), (const char *)decomp, out_len);
        free(decomp);
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }
    return res;
}

/* =========================================================================
 * 11c.4: SESSION STATE SAVE/LOAD
 * ========================================================================= */

BppError stmt_statesave_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type != VAL_STRING || !val.as.string) {
        err.code = 13; err.message = "Type mismatch: STATESAVE expects a filename string";
        return err;
    }

    err = vm_state_save(vm, str_data(val.as.string));
    str_release(vm_get_str(vm), val.as.string);
    return err;
}

BppError stmt_stateload_handler(VMContext *vm, LexerContext *lex) {
    BppError err = check_vdev_permission();
    if (err.code != 0) return err;

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type != VAL_STRING || !val.as.string) {
        err.code = 13; err.message = "Type mismatch: STATELOAD expects a filename string";
        return err;
    }

    err = vm_state_load(vm, str_data(val.as.string));
    str_release(vm_get_str(vm), val.as.string);
    return err;
}

static BValue fn_session_save(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    BppError err = vm_state_save(vm, str_data(args[0].as.string));
    return bval_float(err.code == 0 ? 1.0 : 0.0);
}

static BValue fn_session_load(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    BppError err = vm_state_load(vm, str_data(args[0].as.string));
    return bval_float(err.code == 0 ? 1.0 : 0.0);
}

static BValue fn_session_info(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    BppError err;
    memset(&err, 0, sizeof(err));
    char *info = vm_state_info(vm, str_data(args[0].as.string), &err);
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), info ? info : "", info ? strlen(info) : 0);
    if (info) free(info);
    return res;
}

/* =========================================================================
 * MODULE REGISTRATIONS
 * ========================================================================= */

static const FunctionEntry sound_funcs[] = {
    { "_SNDOPEN",   KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_open,   "Load sound file", NULL },
    { "_SNDPLAY",   KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_play,   "Play sound file", NULL },
    { "_SNDLOOP",   KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_loop,   "Loop sound file", NULL },
    { "_SNDSTOP",   KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_stop,   "Stop sound playback", NULL },
    { "_SNDPAUSE",  KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_pause,  "Pause sound playback", NULL },
    { "_SNDVOL",    KW_NONE, FCAT_IO, FRET_FLOAT,  2, 2, FSAFE_IO, 0, fn_sound_volume, "Set sound volume", NULL },
    { "_SNDLEN",    KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_length, "Get sound length", NULL },
    { "_SNDGETPOS", KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_position,"Get sound position", NULL },
    { "sound.open",   KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_open,   "Load sound file", NULL },
    { "sound.play",   KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_play,   "Play sound file", NULL },
    { "sound.loop",   KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_loop,   "Loop sound file", NULL },
    { "sound.stop",   KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_stop,   "Stop sound playback", NULL },
    { "sound.pause",  KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_pause,  "Pause sound playback", NULL },
    { "sound.volume", KW_NONE, FCAT_IO, FRET_FLOAT,  2, 2, FSAFE_IO, 0, fn_sound_volume, "Set sound volume", NULL },
    { "sound.length", KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_length, "Get sound length", NULL },
    { "sound.position",KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_sound_position,"Get sound position", NULL },
    { "sound.noise",  KW_NONE, FCAT_IO, FRET_FLOAT,  2, 2, FSAFE_IO, 0, fn_sound_noise,  "Generate noise", NULL },
    { "sound.tone",   KW_NONE, FCAT_IO, FRET_FLOAT,  2, 2, FSAFE_IO, 0, fn_sound_tone,   "Generate tone", NULL },
    { "music.play",   KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_music_play,   "Play MML music", NULL }
};

static const FunctionEntry mouse_funcs[] = {
    { "MOUSE",         KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_mouse_query,  "Get mouse coordinate/button state", NULL },
    { "_MOUSEWHEEL",   KW_NONE, FCAT_IO, FRET_FLOAT, 0, 0, FSAFE_IO, 0, fn_mouse_wheel,  "Get mouse scroll wheel delta", NULL },
    { "mouse.poll",    KW_NONE, FCAT_IO, FRET_FLOAT, 0, 0, FSAFE_IO, 0, fn_mouse_poll,   "Poll mouse event queue", NULL },
    { "mouse.x",       KW_NONE, FCAT_IO, FRET_FLOAT, 0, 0, FSAFE_IO, 0, fn_mouse_x,      "Get mouse X coordinate", NULL },
    { "mouse.y",       KW_NONE, FCAT_IO, FRET_FLOAT, 0, 0, FSAFE_IO, 0, fn_mouse_y,      "Get mouse Y coordinate", NULL },
    { "mouse.button",  KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_mouse_button, "Get mouse button pressed status", NULL },
    { "mouse.wheel",   KW_NONE, FCAT_IO, FRET_FLOAT, 0, 0, FSAFE_IO, 0, fn_mouse_wheel,  "Get mouse scroll wheel", NULL },
    { "mouse.show",    KW_NONE, FCAT_IO, FRET_FLOAT, 0, 0, FSAFE_IO, 0, fn_mouse_show,   "Show mouse cursor", NULL },
    { "mouse.hide",    KW_NONE, FCAT_IO, FRET_FLOAT, 0, 0, FSAFE_IO, 0, fn_mouse_hide,   "Hide mouse cursor", NULL }
};

static const FunctionEntry joystick_funcs[] = {
    { "STICK",            KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_joystick_axis,   "Get joystick axis coordinate", NULL },
    { "STRIG",            KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_joystick_button, "Get joystick button state", NULL },
    { "joystick.axis",    KW_NONE, FCAT_IO, FRET_FLOAT,  2, 2, FSAFE_IO, 0, fn_joystick_axis,   "Get joystick axis", NULL },
    { "joystick.button",  KW_NONE, FCAT_IO, FRET_FLOAT,  2, 2, FSAFE_IO, 0, fn_joystick_button, "Get joystick button status", NULL }
};

static const FunctionEntry input_funcs[] = {
    { "_DEVICES",        KW_NONE, FCAT_IO, FRET_FLOAT,  0, 0, FSAFE_IO, 0, fn_input_device_count, "Get count of connected input devices", NULL },
    { "_DEVICE$",        KW_NONE, FCAT_IO, FRET_STRING, 1, 1, FSAFE_IO, 0, fn_input_device_name,  "Get name of connected input device", NULL },
    { "input.device_count",KW_NONE, FCAT_IO, FRET_FLOAT, 0, 0, FSAFE_IO, 0, fn_input_device_count, "Get device count", NULL },
    { "input.device_name", KW_NONE, FCAT_IO, FRET_STRING,1, 1, FSAFE_IO, 0, fn_input_device_name,  "Get device name", NULL }
};

static const FunctionEntry clipboard_funcs[] = {
    { "clipboard.text", KW_NONE, FCAT_IO, FRET_STRING, 0, 0, FSAFE_IO, 0, fn_clipboard_text, "Get clipboard text", NULL },
    { "clipboard.set",  KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_clipboard_set,  "Set clipboard text", NULL }
};

static const FunctionEntry window_funcs[] = {
    { "_SCREENX",         KW_NONE, FCAT_IO, FRET_FLOAT,  0, 0, FSAFE_IO, 0, fn_window_move,      "Get window X position", NULL },
    { "_SCREENY",         KW_NONE, FCAT_IO, FRET_FLOAT,  0, 0, FSAFE_IO, 0, fn_window_fullscreen,"Get window Y position", NULL },
    { "_DESKTOPWIDTH",    KW_NONE, FCAT_IO, FRET_FLOAT,  0, 0, FSAFE_IO, 0, fn_screen_width,     "Get desktop width", NULL },
    { "_DESKTOPHEIGHT",   KW_NONE, FCAT_IO, FRET_FLOAT,  0, 0, FSAFE_IO, 0, fn_screen_height,    "Get desktop height", NULL },
    { "window.title",     KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_window_title,     "Set window title", NULL },
    { "window.move",      KW_NONE, FCAT_IO, FRET_FLOAT,  2, 2, FSAFE_IO, 0, fn_window_move,      "Move window to coordinates", NULL },
    { "window.fullscreen",KW_NONE, FCAT_IO, FRET_FLOAT,  0, 0, FSAFE_IO, 0, fn_window_fullscreen,"Toggle fullscreen mode", NULL },
    { "window.resizable", KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_window_resizable, "Set resizable window status", NULL },
    { "window.icon",      KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_window_icon,      "Set window icon", NULL },
    { "screen.width",     KW_NONE, FCAT_IO, FRET_FLOAT,  0, 0, FSAFE_IO, 0, fn_screen_width,     "Get screen/desktop width", NULL },
    { "screen.height",    KW_NONE, FCAT_IO, FRET_FLOAT,  0, 0, FSAFE_IO, 0, fn_screen_height,    "Get screen/desktop height", NULL }
};

static const FunctionEntry net_funcs[] = {
    { "_CONNECTED",          KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_net_connected, "Check connection status", NULL },
    { "_CONNECTIONADDRESS$", KW_NONE, FCAT_IO, FRET_STRING, 1, 1, FSAFE_IO, 0, fn_net_address,   "Get remote address", NULL },
    { "net.connect",         KW_NONE, FCAT_IO, FRET_FLOAT,  2, 2, FSAFE_IO, 0, fn_net_connect,   "Establish TCP client connection", NULL },
    { "net.listen",          KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_net_listen,    "Establish TCP server listener", NULL },
    { "net.accept",          KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_net_accept,    "Accept client connection", NULL },
    { "net.connected",       KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_net_connected, "Check network connection", NULL },
    { "net.address",         KW_NONE, FCAT_IO, FRET_STRING, 1, 1, FSAFE_IO, 0, fn_net_address,   "Get connection IP address", NULL },
    { "net.send",            KW_NONE, FCAT_IO, FRET_FLOAT,  2, 2, FSAFE_IO, 0, fn_net_send,      "Send data over network", NULL },
    { "net.receive",         KW_NONE, FCAT_IO, FRET_STRING, 2, 2, FSAFE_IO, 0, fn_net_recv,      "Receive data from network", NULL }
};

static const FunctionEntry image_funcs[] = {
    { "_LOADIMAGE",   KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_image_load,   "Load image", NULL },
    { "_NEWIMAGE",    KW_NONE, FCAT_IO, FRET_FLOAT, 3, 3, FSAFE_IO, 0, fn_image_create, "Create blank image", NULL },
    { "_COPYIMAGE",   KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_image_copy,   "Duplicate image", NULL },
    { "_WIDTH",       KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_image_width,  "Get image width", NULL },
    { "_HEIGHT",      KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_image_height, "Get image height", NULL },
    { "image.load",   KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_image_load,   "Load image", NULL },
    { "image.free",   KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_image_free,   "Free image resource", NULL },
    { "image.draw",   KW_NONE, FCAT_IO, FRET_FLOAT, 3, 3, FSAFE_IO, 0, fn_image_draw,   "Draw image at coordinate", NULL },
    { "image.create", KW_NONE, FCAT_IO, FRET_FLOAT, 3, 3, FSAFE_IO, 0, fn_image_create, "Create new image canvas", NULL },
    { "image.copy",   KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_image_copy,   "Copy image buffer", NULL },
    { "image.width",  KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_image_width,  "Get image canvas width", NULL },
    { "image.height", KW_NONE, FCAT_IO, FRET_FLOAT, 1, 1, FSAFE_IO, 0, fn_image_height, "Get image canvas height", NULL }
};

static const FunctionEntry compress_funcs[] = {
    { "_DEFLATE$",       KW_NONE, FCAT_IO, FRET_STRING, 1, 1, FSAFE_PURE, 0, fn_compress_deflate, "Compress string data", NULL },
    { "_INFLATE$",       KW_NONE, FCAT_IO, FRET_STRING, 2, 2, FSAFE_PURE, 0, fn_compress_inflate, "Decompress string data", NULL },
    { "compress.deflate",KW_NONE, FCAT_IO, FRET_STRING, 1, 1, FSAFE_PURE, 0, fn_compress_deflate, "Compress string", NULL },
    { "compress.inflate",KW_NONE, FCAT_IO, FRET_STRING, 2, 2, FSAFE_PURE, 0, fn_compress_inflate, "Decompress string", NULL }
};

static const FunctionEntry session_funcs[] = {
    { "_STATEINFO$",     KW_NONE, FCAT_IO, FRET_STRING, 1, 1, FSAFE_IO, 0, fn_session_info, "Get session file metadata", NULL },
    { "session.save",    KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_session_save,  "Save session state to file", NULL },
    { "session.load",    KW_NONE, FCAT_IO, FRET_FLOAT,  1, 1, FSAFE_IO, 0, fn_session_load,  "Restore session state from file", NULL },
    { "session.info",    KW_NONE, FCAT_IO, FRET_STRING, 1, 1, FSAFE_IO, 0, fn_session_info,  "Get session state info", NULL }
};

static BValue fn_window_x_mock(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(platform_window_x());
}

static BValue fn_window_y_mock(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(platform_window_y());
}

static int mod_io_devices_init(void *rt) {
    (void)rt;
    
    /* Register Sound */
    int count = sizeof(sound_funcs) / sizeof(sound_funcs[0]);
    for (int i = 0; i < count; i++) funcreg_register(&sound_funcs[i]);

    /* Register Mouse */
    count = sizeof(mouse_funcs) / sizeof(mouse_funcs[0]);
    for (int i = 0; i < count; i++) funcreg_register(&mouse_funcs[i]);

    /* Register Joystick */
    count = sizeof(joystick_funcs) / sizeof(joystick_funcs[0]);
    for (int i = 0; i < count; i++) funcreg_register(&joystick_funcs[i]);

    /* Register Input Devices */
    count = sizeof(input_funcs) / sizeof(input_funcs[0]);
    for (int i = 0; i < count; i++) funcreg_register(&input_funcs[i]);

    /* Register Clipboard */
    count = sizeof(clipboard_funcs) / sizeof(clipboard_funcs[0]);
    for (int i = 0; i < count; i++) funcreg_register(&clipboard_funcs[i]);

    /* Register Window */
    count = sizeof(window_funcs) / sizeof(window_funcs[0]);
    for (int i = 0; i < count; i++) funcreg_register(&window_funcs[i]);

    /* Register Net */
    count = sizeof(net_funcs) / sizeof(net_funcs[0]);
    for (int i = 0; i < count; i++) funcreg_register(&net_funcs[i]);

    /* Register Image */
    count = sizeof(image_funcs) / sizeof(image_funcs[0]);
    for (int i = 0; i < count; i++) funcreg_register(&image_funcs[i]);

    /* Register Compress */
    count = sizeof(compress_funcs) / sizeof(compress_funcs[0]);
    for (int i = 0; i < count; i++) funcreg_register(&compress_funcs[i]);

    /* Register Session */
    count = sizeof(session_funcs) / sizeof(session_funcs[0]);
    for (int i = 0; i < count; i++) funcreg_register(&session_funcs[i]);

    /* Register mocks/additional */
    static const FunctionEntry win_x_entry = { "_SCREENX", KW_NONE, FCAT_IO, FRET_FLOAT, 0, 0, FSAFE_IO, 0, fn_window_x_mock, "Get window X", NULL };
    static const FunctionEntry win_y_entry = { "_SCREENY", KW_NONE, FCAT_IO, FRET_FLOAT, 0, 0, FSAFE_IO, 0, fn_window_y_mock, "Get window Y", NULL };
    funcreg_register(&win_x_entry);
    funcreg_register(&win_y_entry);

    return 0;
}

static const BppModuleInfo io_devices_info = {
    "IODEVICES",
    "1.0",
    "Sound, Mouse, Joystick, Clipboard, Window, Net, Image, Compress, Session standard modules",
    MOD_LIBRARY,
    CAP_IO | CAP_SOUND | CAP_NETWORK | CAP_GRAPHICS,
    SEC_STANDARD,
    mod_io_devices_init,
    NULL
};

void mod_io_devices_register(void) {
    module_register(&io_devices_info);
}
