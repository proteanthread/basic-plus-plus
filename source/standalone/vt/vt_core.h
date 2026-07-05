/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vt_core.h
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

#ifndef STANDALONE_VT_CORE_H
#define STANDALONE_VT_CORE_H

typedef enum VtAction {
    VT_ACTION_PRINT,
    VT_ACTION_CURSOR_MOVE,
    VT_ACTION_CURSOR_HOME,
    VT_ACTION_CLEAR_SCREEN,
    VT_ACTION_SET_COLOR_FG,
    VT_ACTION_SET_COLOR_BG,
    VT_ACTION_RESET_ATTRS
} VtAction;

typedef void (*VtCallback)(void *user_data, VtAction action, int param1, int param2, char char_val);

typedef enum VtState {
    VT_STATE_NORMAL,
    VT_STATE_ESC,
    VT_STATE_CSI
} VtState;

typedef struct VtDecoder {
    VtState state;
    int params[8];
    int param_count;
    int current_param;
    VtCallback callback;
    void *user_data;
} VtDecoder;

void vt_core_init(VtDecoder *dec, VtCallback callback, void *user_data);
void vt_core_feed(VtDecoder *dec, char c);
void vt_core_feed_string(VtDecoder *dec, const char *str);

#endif /* STANDALONE_VT_CORE_H */
