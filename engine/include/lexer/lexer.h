// FILENAME: lexer.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext, libkernel
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for lexer.h.
//
// ---- Includes ----

// FILENAME: lexer.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h, stmt_reboot.c)
// NEEDED BY: libcore (analyzer.c, bpp_api.c, clear.c, funcreg.h, get.c)
// NEEDED BY: libcore (keyword_props.h, list.c, metadata.h, spec.h, using.h)
// NEEDED BY: libengine (alias.h, alias.c, append.h, array_ext.h, array_ext.c)
// NEEDED BY: libengine (arrayfill.h, arrayfill.c, ask.h, ask.c, assign.h)
// NEEDED BY: libengine (ast.h, ast.c, ast_internal.h, ast_parse_block.c)
// NEEDED BY: libengine (ast_parse_expr.c, ast_parse_stmt.c, auto.h, auto.c)
// NEEDED BY: libengine (backspace.h, beep.h, beep.c, bgi.h, bios.h, bitmux.h)
// NEEDED BY: libengine (bload.h, brun.h, bsave.h, call.h, call.c)
// NEEDED BY: libengine (cause.h, cause.c, chain.h, chain.c, change.h, chdir.h)
// NEEDED BY: libengine (check.h, check.c, circle.h, circle.c, class.h, class.c)
// NEEDED BY: libengine (clear.h, close.h, close.c, clr.h, clr.c, cls.h, cls.c)
// NEEDED BY: libengine (color.h, color.c, common.h, compat.h, complex.h)
// NEEDED BY: libengine (const.h, const.c, cont.h, cont.c)
// NEEDED BY: libengine (continue.h, continue.c, create.h, create.c)
// NEEDED BY: libengine (data.h, data.c, debug.h, debug.c, declare.h, declare.c)
// NEEDED BY: libengine (def.h, def.c, def_seg.h, def_usr.h, defdbl.h, defint.h)
// NEEDED BY: libengine (deflng.h, defseg.h, defseg.c, defsng.h, defstr.h)
// NEEDED BY: libengine (delete.h, delete.c, destroy.h, dim.h, dim.c)
// NEEDED BY: libengine (do.h, do.c, doevents.h, doevents.c, draw.h, draw.c)
// NEEDED BY: libengine (echo.h, end.h, endloop.h, enter.h, enum.h, enum.c)
// NEEDED BY: libengine (erase.h, eval.h, eval_expr_internal.h, eval_internal.h)
// NEEDED BY: libengine (exchange.h, exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, exit_loop.h, exit_loop.c, extend.h)
// NEEDED BY: libengine (external.h, external.c, field.h, field.c)
// NEEDED BY: libengine (files.h, files.c, find.h, for.h, for.c, form.h)
// NEEDED BY: libengine (function.h, function.c, get.h, global.h, global.c)
// NEEDED BY: libengine (goodbye.c, gosub.h, gosub.c, goto.h, goto.c)
// NEEDED BY: libengine (handler.h, handler.c, help.h, help.c, if.h, image.h)
// NEEDED BY: libengine (incr.h, incr.c, input.h, input_file.h, input_file.c)
// NEEDED BY: libengine (interrupt.h, introspection.h, invoke.h, isam.h)
// NEEDED BY: libengine (joystick.h, joystick.c, key.h, key.c)
// NEEDED BY: libengine (keyword.h, keyword.c, kill.h, kill.c, let.h, let.c)
// NEEDED BY: libengine (lexer.c, lexer_internal.h, line.h, line.c)
// NEEDED BY: libengine (line_input.h, linput.h, list.h, llist.h, llist.c)
// NEEDED BY: libengine (load.h, load.c, lock.h, lock.c, loop.h, loop.c)
// NEEDED BY: libengine (lprint.h, lset.h, lset.c, map.h, margin.h)
// NEEDED BY: libengine (mat_input.h, mat_input.c, mat_internal.h, mat_ops.h)
// NEEDED BY: libengine (mat_print.h, mat_print.c, mat_read.h, mat_read.c)
// NEEDED BY: libengine (mat_write.h, merge.h, merge.c, mid_stmt.h, mid_stmt.c)
// NEEDED BY: libengine (mkdir.h, modify.h, module.c, mouse.h, mouse.c)
// NEEDED BY: libengine (msgbox.h, msgbox.c, mux.h, mux.c, name.h, name.c)
// NEEDED BY: libengine (new.h, new.c, next.h, next.c, on_com.h, on_com.c)
// NEEDED BY: libengine (on_error.h, on_error.c, on_key.h, on_key.c)
// NEEDED BY: libengine (on_timer.h, on_timer.c, open.h, open.c)
// NEEDED BY: libengine (option.h, option.c, out.h, override.h, override.c)
// NEEDED BY: libengine (page.h, paint.h, paint.c, palette.h, palette.c)
// NEEDED BY: libengine (param.h, param.c, pause.h, pen.h, pen.c, perform.h)
// NEEDED BY: libengine (picture.h, picture.c, play.h, play.c, poke.h, poke.c)
// NEEDED BY: libengine (prefix.h, preset.h, preset.c, print.h)
// NEEDED BY: libengine (print_file.h, print_file.c, pset.h, pset.c)
// NEEDED BY: libengine (public.h, public.c, put.h, put.c)
// NEEDED BY: libengine (randomize.h, randomize.c, read.h, read.c, record.h)
// NEEDED BY: libengine (redim.h, reformat.h, reformat.c, reformat_internal.h)
// NEEDED BY: libengine (rem.h, rem.c, remove.h, renum.h, renum.c, repeat.h)
// NEEDED BY: libengine (restore.h, restore.c, resume.h, resume.c)
// NEEDED BY: libengine (retry.h, retry.c, return.h, rewind.h, rmdir.h, rset.h)
// NEEDED BY: libengine (run.h, run.c, save.h, save.c, scale.h, scan_keyword.c)
// NEEDED BY: libengine (scan_number.c, scan_string.c, scope.h, scope.c)
// NEEDED BY: libengine (screen.h, screen.c, seek.h, seek.c, select.h)
// NEEDED BY: libengine (selftest.h, selftest.c, session_stmts.h)
// NEEDED BY: libengine (share.h, share.c, shared.h, shared.c, shell.h, sleep.h)
// NEEDED BY: libengine (sound.h, sound.c, stack.h, stmt.h, stmt_dac.c)
// NEEDED BY: libengine (stmt_deepsleep.c, stmt_delay.c, stmt_dht.c)
// NEEDED BY: libengine (stmt_dwrite.c, stmt_every.c, stmt_freq.c)
// NEEDED BY: libengine (stmt_handlers.h, stmt_home.h, stmt_home.c, stmt_i2c.c)
// NEEDED BY: libengine (stmt_mqtt.c, stmt_neopixel.c, stmt_pinmode.c)
// NEEDED BY: libengine (stmt_plot.h, stmt_plot.c, stmt_pwm.c, stmt_python.c)
// NEEDED BY: libengine (stmt_servo.c, stmt_spi.c, stmt_webrepl.c, stmt_wifi.c)
// NEEDED BY: libengine (stop.h, sub.h, sub_internal.h, suspend.h, swap.h)
// NEEDED BY: libengine (sys.h, system.h, task.h, task.c, test.h, test.c)
// NEEDED BY: libengine (text.h, try.h, try.c, txn.h, type.h, type.c, unless.h)
// NEEDED BY: libengine (unpack.h, until.h, vbdos_controls.h, vbdos_widgets.h)
// NEEDED BY: libengine (vdim.h, verify.h, verify.c, version.c, view.h, view.c)
// NEEDED BY: libengine (viewport.h, viewport.c, voice.h, voice.c)
// NEEDED BY: libengine (void.h, void.c, wait.h, wend.h, wend.c, when.h, when.c)
// NEEDED BY: libengine (whenever.h, while.h, while.c, window.h, window.c)
// NEEDED BY: libengine (with.h, with.c, write_file.h, write_file.c, zone.h)
// NEEDED BY: libext (arrayext.h)
// NEEDED BY: libkernel (gfx.c, gfx_internal.h, gfx_primitives_internal.h)
// NEEDED BY: libkernel (security.h, security.c, vdev.c, version.h)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h)
// Declares lexical scanning and token stream processing for the BASIC++ engine.
//
// ---- Includes ----

#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "types/types.h"
#include "memory/memory.h"

//
// ---- Token Types ----

// ephemeral token types emitted by the scanner
typedef enum {
    TOK_EOF = 0,
    TOK_EOL,            // Newline or statement separator ':'
    TOK_NUMBER,         // Numeric constant
    TOK_IMAGINARY,      // Imaginary constant e.g. 4I, 3.14i
    TOK_STRING,         // String literal inside double quotes
    TOK_RPN_LITERAL,    // RPN literal inside curly braces
    TOK_IDENT,          // Identifier (variable name or label reference)
    TOK_KEYWORD,        // Built-in language statement/command
    // Operators
    TOK_PLUS,           // +
    TOK_MINUS,          // -
    TOK_MUL,            //
    TOK_DIV,            // /
    TOK_POW,            // ^ or **
    TOK_EQ,             // =
    TOK_NE,             // <>
    TOK_LT,             // <
    TOK_GT,             // >
    TOK_LE,             // <=
    TOK_GE,             // >=
    TOK_AND,            // AND
    TOK_OR,             // OR
    TOK_NOT,            // NOT
    TOK_XOR,            // XOR
    TOK_IMP,            // IMP
    TOK_EQV,            // EQV
    TOK_MOD,            // MOD
    TOK_SHL,            // SHL
    TOK_SHR,            // SHR
    TOK_READBIT,        // READBIT
    TOK_SETBIT,         // SETBIT
    TOK_RESETBIT,       // RESETBIT
    TOK_TOGGLEBIT,      // TOGGLEBIT
    TOK_MIN,            // MIN
    TOK_MAX,            // MAX
    TOK_HYPOT,          // HYPOT
    TOK_REMAINDER,      // REMAINDER
    TOK_ATAN2,          // ATAN2
    // Symbols
    TOK_LPAREN,         // (
    TOK_RPAREN,         // )
    TOK_COMMA,          // ,
    TOK_SEMICOLON,      // ;
    TOK_HASH,           // #
    TOK_UNARY_MINUS,    // Internal unary minus
    TOK_UNARY_PLUS,     // Internal unary plus
    TOK_DIRECTIVE,      // ::DIRECTIVE
    TOK_GLOBAL_LABEL,   // ::label:
    TOK_NAMESPACE_DECL, // ::[namespace]
    TOK_DOCSTRING,      // // docstring
    TOK_DOUBLE_COLON,   // ::
    TOK_PERIOD,         // .
    TOK_LBRACKET,       // [
    TOK_RBRACKET,       // ]
    TOK_AMPERSAND,      // & (infix string concatenation / token)
    TOK_BACKSLASH,      // \ (statement separator / token)
    TOK_AT,             // @ (HP path/channel descriptor prefix)
    TOK_UNKNOWN
} BppTokenType;

// Keyword Identifiers
typedef enum {
    KW_NONE = 0,
    KW_PRINT,
    KW_LET,
    KW_INPUT,
    KW_IF,
    KW_THEN,
    KW_ELSE,
    KW_GOTO,
    KW_GOSUB,
    KW_RETURN,
    KW_END,
    KW_REM,
    KW_LIST,
    KW_RUN,
    KW_NEW,
    // Loop Keywords
    KW_FOR,
    KW_NEXT,
    KW_TO,
    KW_STEP,
    KW_BY,
    KW_WHILE,
    KW_WEND,
    KW_DO,
    KW_LOOP,
    KW_UNTIL,
    KW_DIM,
    KW_ERASE,
    KW_OPTION,
    KW_DATA,
    KW_READ,
    KW_RESTORE,
    KW_DEF,
    KW_DEFINT,
    KW_DEFSNG,
    KW_DEFDBL,
    KW_DEFSTR,
    KW_DEFLNG,
    KW_USR,
    KW_ON,
    KW_ERROR,
    KW_RESUME,
    KW_LOAD,
    KW_SAVE,
    KW_MERGE,
    KW_COMMON,
    KW_CHAIN,
    KW_SELFTEST,
    // Phase 3 & 4 Keywords
    KW_OPEN,
    KW_CLOSE,
    KW_AS,
    KW_FILES,
    KW_KILL,
    KW_SCRATCH,
    KW_CHDIR,
    KW_MKDIR,
    KW_RMDIR,
    KW_DIR,
    KW_SETATTR,
    KW_LOCK,
    KW_UNLOCK,
    KW_ENVIRON,
    KW_NAME,
    KW_GET,
    KW_PUT,
    KW_BGET,
    KW_BPUT,
    KW_SEEK,
    KW_FREEFILE,
    KW_FIELD,
    KW_SELECT,
    KW_CASE,
    KW_CONST,
    KW_DECLARE,
    KW_SUB,
    KW_FUNCTION,
    KW_CALL,
    KW_AUTO,
    KW_BCOLOR,
    KW_FCOLOR,
    KW_CLEAR,
    KW_CURSOR,
    KW_LOCATE,
    KW_SHARED,
    KW_SCREEN,
    KW_COLOR,
    KW_LINE,
    KW_CIRCLE,
    KW_PSET,
    KW_PRESET,
    KW_CLS,
    KW_HOME,
    KW_LSET,
    KW_VARPTR,
    KW_VARSEG,
    KW_SADD,
    KW_DEFSEG,
    KW_CINT,
    KW_CSNG,
    KW_CDBL,
    KW_RSET,
    KW_SWAP,
    KW_LOCAL,
    KW_STATIC,
    KW_REDIM,
    KW_PRESERVE,
    KW_PAINT,
    KW_BEEP,
    KW_SOUND,
    KW_PLAY,
    KW_BLOAD,
    KW_BSAVE,
    KW_BRUN,
    KW_USING,
    KW_EDIT,
    KW_SECURITY,
    KW_MODULE,
    KW_LEVEL,
    KW_RESTRICT,
    KW_RESET,
    KW_INFO,
    KW_UNLOAD,
    KW_TASK,
    KW_WAIT,
    KW_MAT,
    KW_MUX,
    KW_DEMUX,
    KW_UNPACK,
    KW_BITMUX,
    KW_ARRAY,
    KW_MAP,
    KW_COM,
    KW_PEN,
    KW_STRIG,
    KW_FILTER,
    KW_REDUCE,
    KW_RENUM,
    KW_REFORMAT,
    KW_DELETE,
    KW_HELP,
    KW_CATALOG,
    KW_CATEGORY,
    KW_CATEGORIES,
    KW_DEVICES,

    KW_IOCTL,
    KW_MOUNT,
    KW_UMOUNT,
    KW_UNSAVE,
    KW_CHVT,
    KW_NET,
    KW_OUT,
    KW_POKE,
    KW_BIOS,
    KW_GEMINI,
    KW_UNLESS,
    KW_DEMAND,
    KW_TRY,
    KW_CATCH,
    KW_THROW,
    KW_ALIAS,
    KW_SCOPE,
    KW_KEYWORD,
    KW_OVERRIDE,
    KW_METADATA,
    KW_DEFINE,
    KW_ENUM,
    KW_WITH,
    KW_NOISE,
    KW_SNDPLAY,
    KW_SNDLOOP,
    KW_SNDSTOP,
    KW_SNDPAUSE,
    KW_SNDVOL,
    KW_MOUSEINPUT,
    KW_MOUSEHIDE,
    KW_MOUSESHOW,
    KW_MOUSE,
    KW_HMOUSE,
    KW_VMOUSE,
    KW_TRIG,
    KW_DISPLAY,
    KW_DISP,
    KW_DOEVENTS,
    KW_MSGBOX,
    KW_LPRINT,
    KW_LLIST,
    KW_RANDOMIZE,
    KW_PWD,
    KW_HOSTNAME,
    KW_USERNAME,
    KW_PATH,
    KW_TITLE,
    KW_SCREENMOVE,
    KW_FULLSCREEN,
    KW_RESIZE,
    KW_ICON,
    KW_NWRITE,
    KW_FREEIMAGE,
    KW_PUTIMAGE,
    KW_STATESAVE,
    KW_STATELOAD,
    KW_TYPE,
    KW_CLASS,
    KW_BORDER,
    KW_INK,
    KW_PAPER,
    KW_PAUSE,
    KW_SYS,
    KW_GR,
    KW_HGR,
    KW_HGR2,
    KW_HCOLOR,
    KW_PLOT,
    KW_HLIN,
    KW_VLIN,
    KW_HPLOT,
    KW_ONERR,
    KW_GRAPHICS,
    KW_MODE,
    KW_DRAWTO,
    KW_SYSTEM,
    KW_SHELL,
    KW_BYE,
    KW_STOP,
    KW_EXIT,
    KW_PROCEDURE,
    KW_ENDFUNC,
    KW_ENDPROC,
    KW_TXN,
    KW_ATOMIC,
    KW_COMMIT,
    KW_ROLLBACK,
    KW_TIMER,
    KW_ALARM,
    KW_ALARM_STR,
    KW_KEY,
    KW_OFF,
    KW_SET,
    KW_SNOOZE,
    KW_UNSET,
    KW_INITGRAPH,
    KW_CLOSEGRAPH,
    KW_PUTPIXEL,
    KW_GETPIXEL,
    KW_BAR,
    KW_ELLIPSE,
    KW_RECTANGLE,
    KW_OUTTEXTXY,
    KW_PALETTE,
    KW_ASSERT,
    KW_TRON,
    KW_TROFF,
    KW_BREAK,
    KW_VARS,
    KW_CHECK,
    KW_VERIFY,
    KW_TEST,
    KW_ENDTEST,
    KW_TRACE,
    KW_DEBUG,
    KW_CONT,
    KW_BACKTRACE,
    KW_DUMP,
    KW_VER,
    KW_VER_STR,
    KW_VARPTR_STR,
    KW_VERSION,
    KW_REMOVE,
    KW_REMOVE_STR,
    KW_AND,
    KW_OR,
    KW_NOT,
    KW_XOR,
    KW_EQ,
    KW_NE,
    KW_LT,
    KW_GT,
    KW_LE,
    KW_GE,
    KW_IMP,
    KW_EQV,
    KW_MOD,
    KW_SHL,
    KW_SHR,
    KW_READBIT,
    KW_SETBIT,
    KW_RESETBIT,
    KW_TOGGLEBIT,
    KW_MIN,
    KW_MAX,
    KW_HYPOT,
    KW_REMAINDER,
    KW_ATAN2,
    // ECMA-116 Standard BASIC (1986) Keywords
    KW_PUBLIC,
    KW_SHARE,
    KW_WHEN,
    KW_USE,
    KW_HANDLER,
    KW_CAUSE,
    KW_RETRY,
    KW_CONTINUE,
    KW_ASK,
    KW_RECSIZE,
    KW_RECTYPE,
    KW_ORGANIZATION,
    KW_VIEWPORT,
    KW_WINDOW,
    KW_PICTURE,
    KW_DRAW,
    KW_EXTERNAL,
    KW_DET,
    KW_DOT,
    KW_CROSS,
    KW_ZER,
    KW_ONE,
    KW_IDN,
    KW_TRN,
    KW_INV,
    // Super BASIC Keywords
    KW_COMPLEX,
    KW_DEFCPX,
    KW_CHANGE,
    KW_FORM,
    KW_IMAGE,
    KW_TEXT,
    KW_MODIFY,
    KW_APPEND,
    KW_LINPUT,
    KW_ZONE,
    KW_MARGIN,
    KW_FNEND,
    // Phase 5 Scoping & Exception Keywords
    KW_GLOBAL,
    KW_EXPORT,
    KW_BYVAL,
    KW_BYREF,
    KW_EXTYPE,
    KW_EXTEXT_STR,
    KW_OPTIONAL,
    KW_PRIVATE,
    KW_IMPORT,
    // Universal Timesharing & DEC PDP-10 Keywords
    KW_EXTEND,
    KW_NOEXTEND,
    KW_SCALE,
    KW_SLEEP,
    KW_ECHO,
    KW_NOECHO,
    KW_RECORD,
    KW_ENTER,
    KW_ASSIGN,
    KW_ADVANCE,
    KW_OLD,
    KW_SUSPEND,
    KW_EVENT,
    // OOP & Timeshare Session Keywords
    KW_EXTENDS,
    KW_PROPERTY,
    KW_METHOD,
    KW_CONSTRUCTOR,
    KW_DESTRUCTOR,
    KW_PROTECTED,
    KW_ABSTRACT,
    KW_INTERFACE,
    KW_IMPLEMENTS,
    KW_OPERATOR,
    KW_NOTHING,
    KW_LOGIN,
    KW_LOGOUT,
    KW_WHO,
    KW_TTY,
    KW_PRIORITY,
    KW_HELLO,
    KW_GOODBYE,
    KW_WHENEVER,
    KW_VDIM,
    KW_REWIND,
    KW_BACKSPACE,
    KW_NOMARGIN,
    KW_NOPAGE,
    KW_PAGE,
    KW_DCOUNT,
    KW_CREATEINDEX,
    KW_DELETEINDEX,
    KW_SETINDEX,
    KW_INSERT,
    KW_UPDATE,
    KW_RETRIEVE,
    KW_SEEKEQ,
    KW_SEEKGE,
    KW_SEEKGT,
    KW_BUTTON,
    KW_MENU,
    KW_TEXTBOX,
    KW_LISTBOX,
    KW_CHECKBOX,
    KW_OPTIONBUTTON,
    KW_LABEL,
    KW_FRAME,
    KW_COMBOBOX,
    KW_HSCROLLBAR,
    KW_VSCROLLBAR,
    KW_DRIVELISTBOX,
    KW_DIRLISTBOX,
    KW_FILELISTBOX,
    KW_TIMERCONTROL,
    KW_INTERRUPT,
    KW_INTERRUPTX,
    KW_EXCHANGE,
    KW_PERFORM,
    KW_INVOKE,
    KW_PREFIX,
    KW_REPEAT,
    KW_ENDLOOP,
    KW_EXITIF,
    KW_DESTROY,
    KW_CREATE,
    KW_PARAM,
    KW_ENDTYPE,
    KW_BYTE,
    KW_BOOLEAN,
    KW_REAL,
    KW_FIND,
    KW_MAPEND,
    KW_SUBEND,
    KW_SUBEXIT,
    KW_INCR,
    KW_DECR,
    KW_CLR,
    KW_VOID,
    KW_ARRAYFILL,
    KW_CEIL,
    KW_PINMODE,
    KW_DWRITE,
    KW_DAC,
    KW_PWM,
    KW_SERVO,
    KW_I2C,
    KW_SPI,
    KW_NEOPIXEL,
    KW_DHT,
    KW_DELAY,
    KW_EVERY,
    KW_DEEPSLEEP,
    KW_LIGHTSLEEP,
    KW_REBOOT,
    KW_FREQ,
    KW_WIFI,
    KW_MQTT,
    KW_WEBREPL,
    KW_PYTHON,
    KW_PEER,
    KW_BT,
    KW_BLE,
    KW_NFC,
    KW_GOPHER,
    KW_TNFS,
    KW_FUJI,
    KW_REMOTE,
    KW_NIL,
    KW_SOCK,
    KW_PORT,
    KW_SNIFF,
    KW_PACKET,
    KW_CRYPTO
} BppKeywordId;

typedef BppTokenType LexTokenType;
typedef BppKeywordId KeywordId;

//
// ---- Token Structure ----

// ephemeral token struct parsed on demand from source line
typedef struct {
    BppTokenType type;
    union {
        double       number;
        const char  *string;  // lexer source slice pointer (not null-terminated)
        BppKeywordId keyword;
    } as;
    const char *start;        // pointer to first character in source
    size_t      length;       // length of token in source
} BppToken;

typedef BppToken Token;

//
// ---- Forward Declarations ----

// opaque lexer context
typedef struct LexerContext LexerContext;

//
// ---- Public Scanner API ----

LexerContext *lex_init(MemoryContext *mem, const char *source);
void          lex_shutdown(LexerContext *ctx);
BppToken      lex_next(LexerContext *ctx);
BppToken      lex_peek(LexerContext *ctx);
const char   *lex_get_pos(LexerContext *ctx);
void          lex_set_pos(LexerContext *ctx, const char *pos);
const char   *lex_keyword_name(BppKeywordId kw);
BppKeywordId  lex_find_keyword_by_name(const char *name);
void          keyword_clear_custom(void);
BppKeywordId  keyword_register_custom(const char *name);

//
// ---- C17 Inline Helpers ----

static inline bool tok_str_equals_ci(const char *s1, const char *s2, size_t n) {
    if (!s1 || !s2) return false;
    for (size_t i = 0; i < n; i++) {
        char c1 = s1[i];
        char c2 = s2[i];
        if (c1 >= 'a' && c1 <= 'z') c1 = (char)(c1 - ('a' - 'A'));
        if (c2 >= 'a' && c2 <= 'z') c2 = (char)(c2 - ('a' - 'A'));
        if (c1 != c2) return false;
    }
    return true;
}

// checks whether token matches a keyword ID or its case-insensitive string literal
static inline bool tok_is_keyword(BppToken tok, BppKeywordId kw, const char *name_literal) {
    if (tok.type == TOK_KEYWORD && tok.as.keyword == kw) return true;
    if (tok.type == TOK_IDENT && name_literal != NULL) {
        size_t len = 0;
        while (name_literal[len]) len++;
        return (tok.length == len && tok_str_equals_ci(tok.start, name_literal, len));
    }
    return false;
}

// checks whether token is an identifier matching the given literal
static inline bool tok_is_ident(BppToken tok, const char *ident_literal) {
    if (tok.type != TOK_IDENT || !ident_literal) return false;
    size_t len = 0;
    while (ident_literal[len]) len++;
    return (tok.length == len && tok_str_equals_ci(tok.start, ident_literal, len));
}

// checks whether token matches a specific token type symbol
static inline bool tok_is_symbol(BppToken tok, BppTokenType type) {
    return tok.type == type;
}

// checks whether token is end-of-line or end-of-file delimiter
static inline bool tok_is_eol_or_eof(BppToken tok) {
    return tok.type == TOK_EOL || tok.type == TOK_EOF;
}

#endif // LEXER_H
