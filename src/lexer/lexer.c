/**
 * @file lexer.c
 * @brief Ephemeral Lexer/Tokenizer implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Converts a raw line of BASIC code into a sequence of transient tokens.
 *   Skips whitespace, scans numbers, strings, operators, and identifiers, mapping keywords case-insensitively.
 * - Why it exists: Provides an isolated lexical analysis layer that does not allocate persistent heap memory.
 * - Why it works this way: It traverses the string using standard pointer arithmetic. Peeking is implemented
 *   by saving the position, scanning, and rolling back, ensuring zero storage overhead for lookahead.
 *   Comments (REM or ') skip the rest of the line and return EOL.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Add mappings to the static keyword table, update symbol character matches.
 * - What cannot be changed: The case-insensitive normalization rules and EOF scanning loop bounds.
 * - What to expect: Tokens point directly to the source buffer, so tokens are only valid while the source line exists.
 * - What to do if something breaks: Check character checks (like isspace/isdigit) and ensure correct pointer boundaries.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Character pointers are ASCII. Numbers compile to double using strtod.
 * - Portability concerns: strtod is standard, but Watcom on FreeDOS must be linked with floating-point libraries.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add new multi-character operator scans (e.g. ^, **, \).
 * - How to write external extensions: New keywords are added to the mapping table or handled by dialect layers.
 */

#include "bpp_lexer.h"
#include "bpp_dialect.h"
#include "bpp_config.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct {
    const char  *name;
    BppKeywordId id;
} KeywordMap;

/* Static keyword lookup table */
static const KeywordMap k_keywords[] = {
    {"PRINT",  KW_PRINT},
    {"LET",    KW_LET},
    {"INPUT",  KW_INPUT},
    {"IF",     KW_IF},
    {"THEN",   KW_THEN},
    {"ELSE",   KW_ELSE},
    {"GOTO",   KW_GOTO},
    {"GOSUB",  KW_GOSUB},
    {"RETURN", KW_RETURN},
    {"END",    KW_END},
    {"REM",    KW_REM},
    {"LIST",   KW_LIST},
    {"RUN",    KW_RUN},
    {"NEW",    KW_NEW},
    {"FOR",    KW_FOR},
    {"NEXT",   KW_NEXT},
    {"TO",     KW_TO},
    {"STEP",   KW_STEP},
    {"BY",     KW_BY},
    {"WHILE",  KW_WHILE},
    {"WEND",   KW_WEND},
    {"DO",     KW_DO},
    {"LOOP",   KW_LOOP},
    {"UNTIL",  KW_UNTIL},
    {"DIM",    KW_DIM},
    {"ERASE",  KW_ERASE},
    {"OPTION", KW_OPTION},
    {"DATA",   KW_DATA},
    {"READ",   KW_READ},
    {"RESTORE",KW_RESTORE},
    {"DEFINT", KW_DEFINT},
    {"DEFSNG", KW_DEFSNG},
    {"DEFDBL", KW_DEFDBL},
    {"DEFSTR", KW_DEFSTR},
    {"ON",     KW_ON},
    {"ERROR",  KW_ERROR},
    {"RESUME", KW_RESUME},
    {"LOAD",   KW_LOAD},
    {"SAVE",   KW_SAVE},
    {"MERGE",  KW_MERGE},
    {"SELFTEST",KW_SELFTEST},
    {"OPEN",   KW_OPEN},
    {"CLOSE",  KW_CLOSE},
    {"AS",     KW_AS},
    {"FILES",  KW_FILES},
    {"KILL",   KW_KILL},
    {"CHDIR",  KW_CHDIR},
    {"MKDIR",  KW_MKDIR},
    {"RMDIR",  KW_RMDIR},
    {"DIR",    KW_DIR},
    {"SETATTR",KW_SETATTR},
    {"LOCK",   KW_LOCK},
    {"UNLOCK", KW_UNLOCK},
    {"ENVIRON",KW_ENVIRON},
    {"NAME",   KW_NAME},
    {"GET",    KW_GET},
    {"PUT",    KW_PUT},
    {"BGET",   KW_BGET},
    {"BPUT",   KW_BPUT},
    {"SEEK",   KW_SEEK},

    {"FIELD",  KW_FIELD},
    {"SELECT", KW_SELECT},
    {"CASE",   KW_CASE},
    {"DECLARE",KW_DECLARE},
    {"SUB",    KW_SUB},
    {"FUNCTION",KW_FUNCTION},
    {"CALL",   KW_CALL},
    {"AUTO",   KW_AUTO},
    {"BCOLOR", KW_BCOLOR},
    {"FCOLOR", KW_FCOLOR},
    {"CLEAR",  KW_CLEAR},
    {"CURSOR", KW_CURSOR},
    {"LOCATE", KW_LOCATE},
    {"SHARED", KW_SHARED},
    {"SCREEN", KW_SCREEN},
    {"COLOR",  KW_COLOR},
    {"LINE",   KW_LINE},
    {"CIRCLE", KW_CIRCLE},
    {"PSET",   KW_PSET},
    {"PRESET", KW_PRESET},
    {"CLS",    KW_CLS},
    {"LSET",   KW_LSET},
    {"DEFSEG", KW_DEFSEG},
    {"RSET",   KW_RSET},
    {"SWAP",   KW_SWAP},
    {"LOCAL",  KW_LOCAL},
    {"STATIC", KW_STATIC},
    {"REDIM",  KW_REDIM},
    {"PRESERVE",KW_PRESERVE},
    {"PAINT",  KW_PAINT},
    {"BEEP",   KW_BEEP},
    {"SOUND",  KW_SOUND},
    {"PLAY",   KW_PLAY},
    {"BLOAD",  KW_BLOAD},
    {"BSAVE",  KW_BSAVE},
    {"BRUN",   KW_BRUN},
    {"USING",  KW_USING},
#if BPP_SUPPORT_EDITOR
    {"EDIT",   KW_EDIT},
#endif
    {"SECURITY", KW_SECURITY},
    {"MODULE",   KW_MODULE},
    {"LEVEL",    KW_LEVEL},
    {"RESTRICT", KW_RESTRICT},
    {"RESET",    KW_RESET},
    {"INFO",     KW_INFO},
    {"UNLOAD",   KW_UNLOAD},
    {"TASK",     KW_TASK},
    {"WAIT",     KW_WAIT},
#if BPP_SUPPORT_MAT
    {"MAT",      KW_MAT},
#endif
    {"ARRAY",    KW_ARRAY},
    {"ENUM",     KW_ENUM},
    {"WITH",     KW_WITH},
    {"NOISE",         KW_NOISE},
    {"_SNDPLAY",      KW_SNDPLAY},
    {"_SNDLOOP",      KW_SNDLOOP},
    {"_SNDSTOP",      KW_SNDSTOP},
    {"_SNDPAUSE",     KW_SNDPAUSE},
    {"_SNDVOL",       KW_SNDVOL},
    {"_MOUSEINPUT",   KW_MOUSEINPUT},
    {"_MOUSEHIDE",    KW_MOUSEHIDE},
    {"_MOUSESHOW",    KW_MOUSESHOW},
    {"_TITLE",        KW_TITLE},
    {"_SCREENMOVE",   KW_SCREENMOVE},
    {"_FULLSCREEN",   KW_FULLSCREEN},
    {"_RESIZE",       KW_RESIZE},
    {"_ICON",         KW_ICON},
    {"NWRITE",        KW_NWRITE},
    {"_FREEIMAGE",    KW_FREEIMAGE},
    {"_PUTIMAGE",     KW_PUTIMAGE},
    {"_STATESAVE",    KW_STATESAVE},
    {"_STATELOAD",    KW_STATELOAD},
    {"MAP",      KW_MAP},
    {"FILTER",   KW_FILTER},
    {"REDUCE",   KW_REDUCE},
#if BPP_SUPPORT_EDITOR
    {"RENUM",    KW_RENUM},
    {"DELETE",   KW_DELETE},
#endif
    {"HELP",     KW_HELP},
    {"CATALOG",  KW_CATALOG},
    {"DEVICES",  KW_DEVICES},
    {"IOCTL",    KW_IOCTL},
#if BPP_SUPPORT_NET
    {"MOUNT",    KW_MOUNT},
    {"UMOUNT",   KW_UMOUNT},
#endif
    {"UNSAVE",   KW_UNSAVE},
    {"CHVT",     KW_CHVT},
#if BPP_SUPPORT_NET
    {"NET",      KW_NET},
#endif
    {"OUT",      KW_OUT},
    {"POKE",     KW_POKE},
#if BPP_SUPPORT_BIOS
    {"BIOS",     KW_BIOS},
#endif
#if BPP_SUPPORT_GEMINI
    {"GEMINI",   KW_GEMINI},
#endif
    {"UNLESS",   KW_UNLESS},
    {"DEMAND",   KW_DEMAND},
    {"TRY",      KW_TRY},
    {"CATCH",    KW_CATCH},
    {"THROW",    KW_THROW},
    {"ALIAS",    KW_ALIAS},
    {"METADATA", KW_METADATA},
    {"DIALECT",  KW_DIALECT},
    {"DEFINE",   KW_DEFINE},
#if BPP_SUPPORT_OOP
    {"TYPE",     KW_TYPE},
    {"CLASS",    KW_CLASS},
#endif
    {"BORDER",   KW_BORDER},
    {"INK",      KW_INK},
    {"PAPER",    KW_PAPER},
    {"PAUSE",    KW_PAUSE},
    {"SYS",      KW_SYS},
    {"GR",       KW_GR},
    {"HGR",      KW_HGR},
    {"HGR2",     KW_HGR2},
    {"HCOLOR",   KW_HCOLOR},
    {"PLOT",     KW_PLOT},
    {"HLIN",     KW_HLIN},
    {"VLIN",     KW_VLIN},
    {"HPLOT",    KW_HPLOT},
    {"ONERR",    KW_ONERR},
    {"GRAPHICS", KW_GRAPHICS},
    {"DRAWTO",   KW_DRAWTO},
    {"SYSTEM",   KW_SYSTEM},
    {"SHELL",    KW_SHELL},
    {"BYE",      KW_BYE},
    {"STOP",     KW_STOP},
    {"EXIT",     KW_EXIT},
    {"PROCEDURE", KW_PROCEDURE},
    {"ENDFUNC",  KW_ENDFUNC},
    {"ENDPROC",  KW_ENDPROC},
    {"TXN",      KW_TXN},
    {"ATOMIC",   KW_ATOMIC},
    {"COMMIT",   KW_COMMIT},
    {"ROLLBACK", KW_ROLLBACK},
    {"TIMER",    KW_TIMER},
    {"KEY",      KW_KEY},
    {"OFF",        KW_OFF},
    {"INITGRAPH",  KW_INITGRAPH},
    {"CLOSEGRAPH", KW_CLOSEGRAPH},
    {"PUTPIXEL",   KW_PUTPIXEL},
    {"GETPIXEL",   KW_GETPIXEL},
    {"BAR",        KW_BAR},
    {"ELLIPSE",    KW_ELLIPSE},
    {"RECTANGLE",  KW_RECTANGLE},
    {"OUTTEXTXY",  KW_OUTTEXTXY},
    {"PALETTE",    KW_PALETTE},
    {"ASSERT",     KW_ASSERT},
    {"TRON",       KW_TRON},
    {"TROFF",      KW_TROFF},
    {"BREAK",      KW_BREAK},
    {"VARS",       KW_VARS},
    {"AND",      KW_AND},
    {"OR",       KW_OR},
    {"NOT",      KW_NOT},
    {"XOR",      KW_XOR},
    {NULL,     KW_NONE}
};

struct LexerContext {
    MemoryContext *mem;
    const char    *source;
    const char    *pos;
    BppDialect    *dialect;
};

void lex_set_dialect(LexerContext *ctx, BppDialect *dialect) {
    if (ctx) {
        ctx->dialect = dialect;
    }
}

LexerContext *lex_init(MemoryContext *mem, const char *source) {
    if (!source) return NULL;
    LexerContext *ctx = (LexerContext *)malloc(sizeof(LexerContext));
    if (!ctx) return NULL;
    ctx->mem = mem;
    ctx->source = source;
    ctx->pos = source;
    ctx->dialect = NULL;
    return ctx;
}

void lex_shutdown(LexerContext *ctx) {
    free(ctx);
}

const char *lex_get_pos(LexerContext *ctx) {
    return ctx ? ctx->pos : NULL;
}

void lex_set_pos(LexerContext *ctx, const char *pos) {
    if (ctx && pos) {
        ctx->pos = pos;
    }
}

#define MAX_CUSTOM_KEYWORDS 128
typedef struct {
    char name[64];
    BppKeywordId id;
} CustomKeywordMap;

static CustomKeywordMap g_custom_keywords[MAX_CUSTOM_KEYWORDS];
static int g_custom_keyword_count = 0;

/**
 * @brief Reset all custom keyword registrations.
 *
 * What it does: Clears the global custom keyword table so that
 * dynamically registered keywords (ID >= 1000) from LOAD FEATURE
 * or spec_register_inline() are forgotten.
 *
 * Why it exists: When RUN "file" loads a new program, stale custom
 * keywords from prior programs can collide with ALIAS expansions,
 * causing the lexer to tokenize an alias name as TOK_KEYWORD instead
 * of TOK_IDENT. This results in incorrect dispatch paths and crashes.
 *
 * When to call: In the RUN handler before loading a new program file.
 */
void keyword_clear_custom(void) {
    g_custom_keyword_count = 0;
}
BppKeywordId keyword_register_custom(const char *name) {
    if (!name || g_custom_keyword_count >= MAX_CUSTOM_KEYWORDS) {
        return KW_NONE;
    }
    /* Check if already registered */
    for (int i = 0; i < g_custom_keyword_count; ++i) {
        if (strcasecmp(g_custom_keywords[i].name, name) == 0) {
            return g_custom_keywords[i].id;
        }
    }
    /* Register new custom keyword */
    size_t len = strlen(name);
    if (len > 63) len = 63;
    memcpy(g_custom_keywords[g_custom_keyword_count].name, name, len);
    g_custom_keywords[g_custom_keyword_count].name[len] = '\0';
    
    /* Dynamic keyword IDs start at 1000 */
    BppKeywordId new_id = (BppKeywordId)(1000 + g_custom_keyword_count);
    g_custom_keywords[g_custom_keyword_count].id = new_id;
    g_custom_keyword_count++;
    return new_id;
}

const char *lex_keyword_name(BppKeywordId kw) {
    for (int i = 0; k_keywords[i].name != NULL; ++i) {
        if (k_keywords[i].id == kw) {
            return k_keywords[i].name;
        }
    }
    for (int i = 0; i < g_custom_keyword_count; ++i) {
        if (g_custom_keywords[i].id == kw) {
            return g_custom_keywords[i].name;
        }
    }
    return "UNKNOWN";
}

BppKeywordId lex_find_keyword_by_name(const char *name) {
    if (!name) return KW_NONE;
    for (int i = 0; k_keywords[i].name != NULL; ++i) {
        if (strcasecmp(k_keywords[i].name, name) == 0) {
            return k_keywords[i].id;
        }
    }
    for (int i = 0; i < g_custom_keyword_count; ++i) {
        if (strcasecmp(g_custom_keywords[i].name, name) == 0) {
            return g_custom_keywords[i].id;
        }
    }
    return KW_NONE;
}

/* Helper: skip whitespace */
static void skip_whitespace(LexerContext *ctx) {
    while (*ctx->pos && *ctx->pos != '\n' && isspace((unsigned char)*ctx->pos)) {
        ctx->pos++;
    }
}

static bool match_directive(const char *start, int len, const char *target) {
    if (len != (int)strlen(target)) return false;
    for (int i = 0; i < len; i++) {
        if (toupper((unsigned char)start[i]) != target[i]) return false;
    }
    return true;
}

BppToken lex_next(LexerContext *ctx) {
    BppToken tok;
    memset(&tok, 0, sizeof(tok));
    tok.type = TOK_UNKNOWN;

    if (!ctx) {
        tok.type = TOK_EOF;
        return tok;
    }

    skip_whitespace(ctx);

    tok.start = ctx->pos;

    /* Scan double-colon constructs (::DIRECTIVE, ::label:, ::[namespace]) */
    if (*ctx->pos == ':' && *(ctx->pos + 1) == ':') {
        ctx->pos += 2;
        
        // 1. ::[namespace]
        if (*ctx->pos == '[') {
            ctx->pos++;
            tok.start = ctx->pos;
            while (*ctx->pos && *ctx->pos != ']' && *ctx->pos != '\n') {
                ctx->pos++;
            }
            tok.type = TOK_NAMESPACE_DECL;
            tok.length = ctx->pos - tok.start;
            tok.as.string = tok.start;
            if (*ctx->pos == ']') {
                ctx->pos++;
            }
            return tok;
        }

        // Scan identifier after ::
        const char *ident_start = ctx->pos;
        while (isalnum((unsigned char)*ctx->pos) || *ctx->pos == '_') {
            ctx->pos++;
        }
        int len = (int)(ctx->pos - ident_start);

        if (len > 0) {
            // Check if it's a known directive
            bool is_directive = false;
            if (match_directive(ident_start, len, "DIALECT")) is_directive = true;
            else if (match_directive(ident_start, len, "OPTION")) is_directive = true;
            else if (match_directive(ident_start, len, "INCLUDE")) is_directive = true;
            else if (match_directive(ident_start, len, "IMPORT")) is_directive = true;
            else if (match_directive(ident_start, len, "KEYWORD")) is_directive = true;
            else if (match_directive(ident_start, len, "SCOPE")) is_directive = true;
            else if (match_directive(ident_start, len, "ALIAS")) is_directive = true;

            if (is_directive) {
                tok.type = TOK_DIRECTIVE;
                tok.start = ident_start;
                tok.length = len;
                tok.as.string = ident_start;
                return tok;
            } else {
                tok.type = TOK_GLOBAL_LABEL;
                tok.start = ident_start;
                tok.length = len;
                tok.as.string = ident_start;
                if (*ctx->pos == ':') {
                    ctx->pos++; // consume definition suffix
                }
                return tok;
            }
        } else {
            tok.type = TOK_DOUBLE_COLON;
            tok.length = 2;
            tok.as.string = tok.start;
            return tok;
        }
    }

    /* Scan docstring constructs (// docstring) */
    if (*ctx->pos == '/' && *(ctx->pos + 1) == '/') {
        ctx->pos += 2;
        while (*ctx->pos == ' ' || *ctx->pos == '\t') {
            ctx->pos++;
        }
        tok.start = ctx->pos;
        if (*ctx->pos == '"') {
            ctx->pos++;
            tok.start = ctx->pos;
            while (*ctx->pos && *ctx->pos != '"' && *ctx->pos != '\n') {
                ctx->pos++;
            }
            tok.type = TOK_DOCSTRING;
            tok.length = ctx->pos - tok.start;
            tok.as.string = tok.start;
            if (*ctx->pos == '"') {
                ctx->pos++;
            }
        } else {
            while (*ctx->pos && *ctx->pos != '\n') {
                ctx->pos++;
            }
            tok.type = TOK_DOCSTRING;
            tok.length = ctx->pos - tok.start;
            tok.as.string = tok.start;
        }
        return tok;
    }

    if (*ctx->pos == '\0') {
        tok.type = TOK_EOF;
        tok.length = 0;
        return tok;
    }

    if (*ctx->pos == '\n') {
        tok.type = TOK_EOL;
        tok.length = 1;
        ctx->pos++;
        return tok;
    }

    char sep = ':';
    if (ctx->dialect && ctx->dialect->stmt_separator != 0) {
        sep = ctx->dialect->stmt_separator;
    }
    if (*ctx->pos == sep || *ctx->pos == ':') {
        tok.type = TOK_EOL;
        tok.length = 1;
        ctx->pos++;
        return tok;
    }

    /* Skip single-quote or custom comment */
    char comm = '\'';
    if (ctx->dialect && ctx->dialect->comment_char != 0) {
        comm = ctx->dialect->comment_char;
    }
    if (*ctx->pos == comm || *ctx->pos == '\'' || (*ctx->pos == '!' && *(ctx->pos + 1) != '=')) {
        /* Consume till EOL or EOF */
        while (*ctx->pos && *ctx->pos != '\n') {
            ctx->pos++;
        }
        tok.type = TOK_EOL;
        tok.length = ctx->pos - tok.start;
        return tok;
    }

    /* Scan RPN literal in curly braces */
    if (*ctx->pos == '{') {
        const char *orig = ctx->pos;
        ctx->pos++; /* Skip '{' */
        tok.start = ctx->pos;
        while (*ctx->pos && *ctx->pos != '}' && *ctx->pos != '\n') {
            ctx->pos++;
        }
        if (*ctx->pos == '}') {
            tok.type = TOK_RPN_LITERAL;
            tok.length = ctx->pos - tok.start;
            tok.as.string = tok.start;
            ctx->pos++; /* Skip '}' */
            return tok;
        } else {
            /* Unmatched '{' on this line: reset to parse as standard characters */
            ctx->pos = orig;
        }
    }

    /* Scan string literal */
    if (*ctx->pos == '"') {
        ctx->pos++; /* Skip opening quote */
        tok.start = ctx->pos;
        while (*ctx->pos && *ctx->pos != '"' && *ctx->pos != '\n') {
            ctx->pos++;
        }
        tok.type = TOK_STRING;
        tok.length = ctx->pos - tok.start;
        tok.as.string = tok.start;
        if (*ctx->pos == '"') {
            ctx->pos++; /* Skip closing quote */
        }
        return tok;
    }

    /* Scan &H hex, &O/& octal, &B binary literals */
    if (*ctx->pos == '&') {
        const char *orig = ctx->pos;
        ctx->pos++; /* Consume '&' */
        long val = 0;
        bool found = false;

        if (*ctx->pos == 'H' || *ctx->pos == 'h') {
            ctx->pos++; /* Consume 'H' */
            while (*ctx->pos) {
                char hc = *ctx->pos;
                if (hc >= '0' && hc <= '9') {
                    val = val * 16 + (hc - '0');
                    found = true;
                } else if (hc >= 'A' && hc <= 'F') {
                    val = val * 16 + (hc - 'A' + 10);
                    found = true;
                } else if (hc >= 'a' && hc <= 'f') {
                    val = val * 16 + (hc - 'a' + 10);
                    found = true;
                } else {
                    break;
                }
                ctx->pos++;
            }
        } else if (*ctx->pos == 'O' || *ctx->pos == 'o') {
            ctx->pos++; /* Consume 'O' */
            while (*ctx->pos >= '0' && *ctx->pos <= '7') {
                val = val * 8 + (*ctx->pos - '0');
                found = true;
                ctx->pos++;
            }
        } else if (*ctx->pos == 'B' || *ctx->pos == 'b') {
            ctx->pos++; /* Consume 'B' */
            while (*ctx->pos == '0' || *ctx->pos == '1') {
                val = val * 2 + (*ctx->pos - '0');
                found = true;
                ctx->pos++;
            }
        } else if (*ctx->pos >= '0' && *ctx->pos <= '7') {
            /* Bare octal */
            while (*ctx->pos >= '0' && *ctx->pos <= '7') {
                val = val * 8 + (*ctx->pos - '0');
                found = true;
                ctx->pos++;
            }
        }

        if (found) {
            tok.type = TOK_NUMBER;
            tok.length = ctx->pos - orig;
            tok.as.number = (double)val;
            return tok;
        } else {
            ctx->pos = orig;
        }
    }

    /* Scan C-style Hex/Octal/Binary literals starting with 0 */
    if (*ctx->pos == '0' && *(ctx->pos + 1) != '\0') {
        char next = *(ctx->pos + 1);
        if (next == 'x' || next == 'X' || next == 'o' || next == 'O' || next == 'b' || next == 'B') {
            const char *orig = ctx->pos;
            ctx->pos += 2; /* Skip '0x'/'0o'/'0b' */
            long val = 0;
            bool found = false;

            if (next == 'x' || next == 'X') {
                while (*ctx->pos) {
                    char hc = *ctx->pos;
                    if (hc >= '0' && hc <= '9') {
                        val = val * 16 + (hc - '0');
                        found = true;
                    } else if (hc >= 'A' && hc <= 'F') {
                        val = val * 16 + (hc - 'A' + 10);
                        found = true;
                    } else if (hc >= 'a' && hc <= 'f') {
                        val = val * 16 + (hc - 'a' + 10);
                        found = true;
                    } else {
                        break;
                    }
                    ctx->pos++;
                }
            } else if (next == 'o' || next == 'O') {
                while (*ctx->pos >= '0' && *ctx->pos <= '7') {
                    val = val * 8 + (*ctx->pos - '0');
                    found = true;
                    ctx->pos++;
                }
            } else {
                while (*ctx->pos == '0' || *ctx->pos == '1') {
                    val = val * 2 + (*ctx->pos - '0');
                    found = true;
                    ctx->pos++;
                }
            }

            if (found) {
                tok.type = TOK_NUMBER;
                tok.length = ctx->pos - orig;
                tok.as.number = (double)val;
                return tok;
            } else {
                ctx->pos = orig;
            }
        }
    }

    /* Scan number */
    if (isdigit((unsigned char)*ctx->pos) || (*ctx->pos == '.' && isdigit((unsigned char)*(ctx->pos + 1)))) {
        char *endptr;
        double val = strtod(ctx->pos, &endptr);
        if (endptr != ctx->pos) {
            tok.type = TOK_NUMBER;
            tok.length = endptr - ctx->pos;
            tok.as.number = val;
            ctx->pos = endptr;
            return tok;
        }
    }

    /* Scan identifier or keyword */
    if (isalpha((unsigned char)*ctx->pos) || *ctx->pos == '_') {
        const char *start = ctx->pos;
        ctx->pos++;
        char mac = (ctx->dialect) ? ctx->dialect->member_access_char : '\0';
        while (isalnum((unsigned char)*ctx->pos) || *ctx->pos == '_' || (*ctx->pos == '.' && mac != '.')) {
            ctx->pos++;
        }
        /* Match suffix */
        if (*ctx->pos == '$' || *ctx->pos == '%' || *ctx->pos == '&' || *ctx->pos == '!' || *ctx->pos == '#') {
            ctx->pos++;
        }
        tok.length = ctx->pos - start;
        tok.as.string = start;

        /* Compare case-insensitively or case-sensitively with keyword tables */
        bool is_kw = false;
        bool case_sens = (ctx->dialect && ctx->dialect->case_sensitive);

        /* Check custom dialect keywords first */
        if (!is_kw && tok.length >= 3 && strncasecmp(start, "REM", 3) == 0) {
            char c4 = (tok.length > 3) ? start[3] : '\0';
            if (c4 != '$' && c4 != '%' && c4 != '&' && c4 != '!' && c4 != '#') {
                bool is_start = true;
                const char *p = start - 1;
                while (p >= ctx->source) {
                    if (*p == ':') break;
                    if (!isspace((unsigned char)*p)) {
                        const char *w_end = p;
                        while (p >= ctx->source && !isspace((unsigned char)*p) && *p != ':') {
                            p--;
                        }
                        const char *w_start = p + 1;
                        ptrdiff_t w_len = w_end - w_start + 1;
                        if ((w_len == 4 && strncasecmp(w_start, "THEN", 4) == 0) ||
                            (w_len == 4 && strncasecmp(w_start, "ELSE", 4) == 0)) {
                            break;
                        }
                        is_start = false;
                        break;
                    }
                    p--;
                }

                bool followed_by_eq = false;
                const char *q = ctx->pos;
                while (*q) {
                    if (*q == '=') {
                        if (*(q + 1) != '=') followed_by_eq = true;
                        break;
                    }
                    if (!isspace((unsigned char)*q)) break;
                    q++;
                }

                if (is_start && !followed_by_eq) {
                    tok.type = TOK_KEYWORD;
                    tok.as.keyword = KW_REM;
                    is_kw = true;
                }
            }
        }

        if (ctx->dialect) {
            for (int i = 0; i < ctx->dialect->keyword_count; ++i) {
                size_t kw_len = strlen(ctx->dialect->keywords[i].name);
                if (tok.length == kw_len) {
                    bool match = true;
                    for (size_t j = 0; j < kw_len; ++j) {
                        char a = start[j];
                        char b = ctx->dialect->keywords[i].name[j];
                        if (case_sens) {
                            if (a != b) { match = false; break; }
                        } else {
                            if (toupper((unsigned char)a) != toupper((unsigned char)b)) { match = false; break; }
                        }
                    }
                    if (match) {
                        BppKeywordId kw_id = ctx->dialect->keywords[i].id;
                        if (kw_id == KW_AND) tok.type = TOK_AND;
                        else if (kw_id == KW_OR) tok.type = TOK_OR;
                        else if (kw_id == KW_NOT) tok.type = TOK_NOT;
                        else if (kw_id == KW_XOR) tok.type = TOK_XOR;
                        else {
                            tok.type = TOK_KEYWORD;
                            tok.as.keyword = kw_id;
                        }
                        is_kw = true;
                        break;
                    }
                }
            }
        }

        /* Check core k_keywords */
        if (!is_kw) {
            for (int i = 0; k_keywords[i].name != NULL; ++i) {
                size_t kw_len = strlen(k_keywords[i].name);
                if (tok.length == kw_len) {
                    bool match = true;
                    for (size_t j = 0; j < kw_len; ++j) {
                        if (case_sens) {
                            if (start[j] != k_keywords[i].name[j]) { match = false; break; }
                        } else {
                            if (toupper((unsigned char)start[j]) != k_keywords[i].name[j]) { match = false; break; }
                        }
                    }
                    if (match) {
                        BppKeywordId kw_id = k_keywords[i].id;
                        if (kw_id == KW_AND) tok.type = TOK_AND;
                        else if (kw_id == KW_OR) tok.type = TOK_OR;
                        else if (kw_id == KW_NOT) tok.type = TOK_NOT;
                        else if (kw_id == KW_XOR) tok.type = TOK_XOR;
                        else {
                            tok.type = TOK_KEYWORD;
                            tok.as.keyword = kw_id;
                        }
                        is_kw = true;
                        break;
                    }
                }
            }
        }

        /* Check dynamic registered custom keywords */
        if (!is_kw) {
            for (int i = 0; i < g_custom_keyword_count; ++i) {
                size_t kw_len = strlen(g_custom_keywords[i].name);
                if (tok.length == kw_len) {
                    bool match = true;
                    for (size_t j = 0; j < kw_len; ++j) {
                        char a = start[j];
                        char b = g_custom_keywords[i].name[j];
                        if (case_sens) {
                            if (a != b) { match = false; break; }
                        } else {
                            if (toupper((unsigned char)a) != toupper((unsigned char)b)) { match = false; break; }
                        }
                    }
                    if (match) {
                        /* Contextual check: custom keywords are only TOK_KEYWORD if starting a statement and not followed by '=' */
                        bool is_start_of_statement = true;
                        const char *p = start - 1;
                        while (p >= ctx->source) {
                            if (*p == ':') {
                                break;
                            }
                            if (!isspace((unsigned char)*p)) {
                                const char *w_end = p;
                                while (p >= ctx->source && !isspace((unsigned char)*p) && *p != ':') {
                                    p--;
                                }
                                const char *w_start = p + 1;
                                ptrdiff_t w_len = w_end - w_start + 1;
                                if ((w_len == 4 && strncasecmp(w_start, "THEN", 4) == 0) ||
                                    (w_len == 4 && strncasecmp(w_start, "ELSE", 4) == 0)) {
                                    break;
                                }
                                is_start_of_statement = false;
                                break;
                            }
                            p--;
                        }

                        bool followed_by_eq = false;
                        const char *q = ctx->pos;
                        while (*q) {
                            if (*q == '=') {
                                if (*(q + 1) != '=') {
                                    followed_by_eq = true;
                                }
                                break;
                            }
                            if (!isspace((unsigned char)*q)) {
                                break;
                            }
                            q++;
                        }

                        if (is_start_of_statement && !followed_by_eq) {
                            tok.type = TOK_KEYWORD;
                            tok.as.keyword = g_custom_keywords[i].id;
                            is_kw = true;
                        }
                        break;
                    }
                }
            }
        }

        if (!is_kw) {
            tok.type = TOK_IDENT;
        }

        /* Special REM comment bypass: if it's REM, skip rest of the line */
        if (is_kw && tok.as.keyword == KW_REM) {
            while (*ctx->pos && *ctx->pos != '\n') {
                ctx->pos++;
            }
            tok.type = TOK_EOL;
            tok.length = ctx->pos - tok.start;
        }

        return tok;
    }

    /* Multi-character operators */
    if (*ctx->pos == '<') {
        if (*(ctx->pos + 1) == '=') {
            tok.type = TOK_LE;
            tok.length = 2;
            ctx->pos += 2;
            return tok;
        }
        if (*(ctx->pos + 1) == '>') {
            tok.type = TOK_NE;
            tok.length = 2;
            ctx->pos += 2;
            return tok;
        }
        tok.type = TOK_LT;
        tok.length = 1;
        ctx->pos++;
        return tok;
    }

    if (*ctx->pos == '>') {
        if (*(ctx->pos + 1) == '=') {
            tok.type = TOK_GE;
            tok.length = 2;
            ctx->pos += 2;
            return tok;
        }
        tok.type = TOK_GT;
        tok.length = 1;
        ctx->pos++;
        return tok;
    }

    /* Single character operators and punctuation */
    char c = *ctx->pos;
    char member_access_char = (ctx->dialect) ? ctx->dialect->member_access_char : '\0';
    if (member_access_char != 0 && c == member_access_char) {
        tok.type = TOK_PERIOD;
        tok.length = 1;
        ctx->pos++;
        return tok;
    }
    ctx->pos++;
    tok.length = 1;

    switch (c) {
        case '+': tok.type = TOK_PLUS; break;
        case '-': tok.type = TOK_MINUS; break;
        case '*': tok.type = TOK_MUL; break;
        case '/': tok.type = TOK_DIV; break;
        case '=': tok.type = TOK_EQ; break;
        case '.': tok.type = TOK_PERIOD; break;
        case '(': tok.type = TOK_LPAREN; break;
        case ')': tok.type = TOK_RPAREN; break;
        case ',': tok.type = TOK_COMMA; break;
        case ';': tok.type = TOK_SEMICOLON; break;
        case '#': tok.type = TOK_HASH; break;
        case '[': tok.type = TOK_LBRACKET; break;
        case ']': tok.type = TOK_RBRACKET; break;
        default:
            tok.type = TOK_UNKNOWN;
            break;
    }

    return tok;
}

BppToken lex_peek(LexerContext *ctx) {
    if (!ctx) {
        BppToken tok;
        memset(&tok, 0, sizeof(tok));
        tok.type = TOK_EOF;
        return tok;
    }
    const char *old_pos = ctx->pos;
    BppToken tok = lex_next(ctx);
    ctx->pos = old_pos;
    return tok;
}
