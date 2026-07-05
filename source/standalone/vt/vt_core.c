#include "vt_core.h"
#include <string.h>
#include <stdlib.h>

/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vt_core.c
 * Subsystem: Standalone ANSI Escape Terminal Decoder
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    State machine that decodes ANSI/VT100 escape sequences from a char stream.
 *
 * 2. WHAT TO EXPECT:
 *    Non-allocating state-based parsing. Feeds characters one by one.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Supported escape sequence list, callback event parameters.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Basic parsing states (Normal, Esc, CSI).
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If sequences are ignored, verify the terminator character matches standard ANSI specs.
 * ===================================================================== */

void vt_core_init(VtDecoder *dec, VtCallback callback, void *user_data)
{
    memset(dec, 0, sizeof(*dec));
    dec->state = VT_STATE_NORMAL;
    dec->callback = callback;
    dec->user_data = user_data;
}

static void vt_core_handle_csi(VtDecoder *dec, char cmd)
{
    if (dec->param_count < 8 && dec->current_param >= 0) {
        dec->params[dec->param_count++] = dec->current_param;
    }
    
    if (cmd == 'H' || cmd == 'f') {
        int r = (dec->param_count > 0) ? dec->params[0] : 1;
        int c = (dec->param_count > 1) ? dec->params[1] : 1;
        if (dec->callback) {
            if (r == 1 && c == 1) {
                dec->callback(dec->user_data, VT_ACTION_CURSOR_HOME, 0, 0, 0);
            } else {
                dec->callback(dec->user_data, VT_ACTION_CURSOR_MOVE, r, c, 0);
            }
        }
    } else if (cmd == 'J') {
        int mode = (dec->param_count > 0) ? dec->params[0] : 0;
        if (mode == 2 && dec->callback) {
            dec->callback(dec->user_data, VT_ACTION_CLEAR_SCREEN, 0, 0, 0);
        }
    } else if (cmd == 'm') {
        int i;
        if (dec->param_count == 0) {
            if (dec->callback) dec->callback(dec->user_data, VT_ACTION_RESET_ATTRS, 0, 0, 0);
        }
        for (i = 0; i < dec->param_count; i++) {
            int p = dec->params[i];
            if (p == 0) {
                if (dec->callback) dec->callback(dec->user_data, VT_ACTION_RESET_ATTRS, 0, 0, 0);
            } else if (p >= 30 && p <= 37) {
                if (dec->callback) dec->callback(dec->user_data, VT_ACTION_SET_COLOR_FG, p - 30, 0, 0);
            } else if (p >= 40 && p <= 47) {
                if (dec->callback) dec->callback(dec->user_data, VT_ACTION_SET_COLOR_BG, p - 40, 0, 0);
            }
        }
    }
}

void vt_core_feed(VtDecoder *dec, char c)
{
    switch (dec->state) {
        case VT_STATE_NORMAL:
            if (c == '\033') {
                dec->state = VT_STATE_ESC;
            } else {
                if (dec->callback) {
                    dec->callback(dec->user_data, VT_ACTION_PRINT, 0, 0, c);
                }
            }
            break;
            
        case VT_STATE_ESC:
            if (c == '[') {
                dec->state = VT_STATE_CSI;
                dec->param_count = 0;
                dec->current_param = -1;
                memset(dec->params, 0, sizeof(dec->params));
            } else {
                dec->state = VT_STATE_NORMAL;
                if (dec->callback) {
                    dec->callback(dec->user_data, VT_ACTION_PRINT, 0, 0, '\033');
                    dec->callback(dec->user_data, VT_ACTION_PRINT, 0, 0, c);
                }
            }
            break;
            
        case VT_STATE_CSI:
            if (c >= '0' && c <= '9') {
                if (dec->current_param < 0) dec->current_param = 0;
                dec->current_param = dec->current_param * 10 + (c - '0');
            } else if (c == ';') {
                if (dec->param_count < 8) {
                    dec->params[dec->param_count++] = (dec->current_param >= 0) ? dec->current_param : 0;
                }
                dec->current_param = -1;
            } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                vt_core_handle_csi(dec, c);
                dec->state = VT_STATE_NORMAL;
            } else {
                dec->state = VT_STATE_NORMAL;
            }
            break;
    }
}

void vt_core_feed_string(VtDecoder *dec, const char *str)
{
    if (!str) return;
    while (*str) {
        vt_core_feed(dec, *str);
        str++;
    }
}
