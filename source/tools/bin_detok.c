/* =====================================================================
 * GW-BASIC Binary Detokenizer (bin_detok)
 * Reads a proprietary GW-BASIC binary .BAS file (starts with 0xFF)
 * and detokenizes it to standard ASCII text.
 * ===================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Standard GW-BASIC tokens (0x81 to 0xEF)
static const char *gw_tokens_80[] = {
    /* 0x80 */ "END", "FOR", "NEXT", "DATA", "INPUT", "DIM", "READ", "LET",
    /* 0x88 */ "GOTO", "RUN", "IF", "RESTORE", "GOSUB", "RETURN", "REM", "STOP",
    /* 0x90 */ "PRINT", "CLEAR", "LIST", "NEW", "ON", "WAIT", "DEF", "POKE",
    /* 0x98 */ "CONT", "OUT", "LPRINT", "LLIST", "RESUME", "WIDTH", "ELSE", "TRON",
    /* 0xA0 */ "TROFF", "SWAP", "ERASE", "EDIT", "ERROR", "RESUME", "DELETE", "AUTO",
    /* 0xA8 */ "RENUM", "DEFSTR", "DEFINT", "DEFSNG", "DEFDBL", "LINE", "WHILE", "WEND",
    /* 0xB0 */ "CALL", "", "", "", "WRITE", "OPTION", "RANDOMIZE", "OPEN",
    /* 0xB8 */ "CLOSE", "LOAD", "MERGE", "SAVE", "COLOR", "CLS", "MOTOR", "BSAVE",
    /* 0xC0 */ "BLOAD", "SOUND", "BEEP", "PSET", "PRESET", "SCREEN", "KEY", "LOCATE",
    /* 0xC8 */ "", "TO", "THEN", "TAB(", "STEP", "USR", "FN", "SPC(",
    /* 0xD0 */ "NOT", "ERL", "ERR", "STRING$", "USING", "INSTR", "'", "VARPTR",
    /* 0xD8 */ "CSRLIN", "POINT", "OFF", "INKEY$", "", "", "", "",
    /* 0xE0 */ "", "", "", "", "", "", ">", "=",
    /* 0xE8 */ "<", "+", "-", "*", "/", "^", "AND", "OR",
    /* 0xF0 */ "XOR", "EQV", "IMP", "MOD", "\\", "", "", "",
    /* 0xF8 */ "", "", "", "", "", "", "", ""
};

static const char *gw_tokens_FD[] = {
    /* 0x81 */ "CVI", "CVS", "CVD", "MKI$", "MKS$", "MKD$"
};

static const char *gw_tokens_FE[] = {
    /* 0x81 */ "FILES", "FIELD", "SYSTEM", "NAME", "LSET", "RSET", "KILL", "PUT",
    /* 0x89 */ "GET", "RESET", "COMMON", "CHAIN", "DATE$", "TIME$", "PAINT", "COM",
    /* 0x91 */ "CIRCLE", "DRAW", "PLAY", "TIMER", "ERDEV", "IOCTL", "CHDIR", "MKDIR",
    /* 0x99 */ "RMDIR", "SHELL", "ENVIRON", "VIEW", "WINDOW", "PMAP", "PALETTE", "LCOPY",
    /* 0xA1 */ "CALLS", "", "", "", "", "", "", "PCOPY",
    /* 0xA9 */ "SAVE", "LOAD", "MERGE"
};

static const char *gw_tokens_FF[] = {
    /* 0x81 */ "LEFT$", "RIGHT$", "MID$", "SGN", "INT", "ABS", "SQR", "RND",
    /* 0x89 */ "SIN", "LOG", "EXP", "COS", "TAN", "ATN", "FRE", "INP",
    /* 0x91 */ "POS", "LEN", "STR$", "VAL", "ASC", "CHR$", "PEEK", "SPACE$",
    /* 0x99 */ "OCT$", "HEX$", "LPOS", "CINT", "CSNG", "CDBL", "FIX", "PEN",
    /* 0xA1 */ "STICK", "STRIG", "EOF", "LOC", "LOF"
};

void parse_file(FILE *in, FILE *out) {
    int magic = fgetc(in);
    if (magic != 0xFF) {
        fprintf(stderr, "Not a valid GW-BASIC tokenized file (missing 0xFF magic byte).\n");
        return;
    }

    while (1) {
        // Read 2-byte line link
        int lsb = fgetc(in);
        if (lsb == EOF) break;
        int msb = fgetc(in);
        if (msb == EOF) break;
        if (lsb == 0 && msb == 0) break; // EOF marker

        // Read 2-byte line number
        int ln_lsb = fgetc(in);
        int ln_msb = fgetc(in);
        int line_num = ln_lsb | (ln_msb << 8);

        fprintf(out, "%d ", line_num);

        // Read tokens until 0x00
        int c;
        while ((c = fgetc(in)) != EOF && c != 0x00) {
            if (c >= 0x80 && c <= 0xF0) {
                // Single-byte token
                if (c == 0x8F) { // REM special case
                    fprintf(out, "REM");
                } else {
                    int idx = c - 0x80;
                    fprintf(out, "%s", gw_tokens_80[idx]);
                }
            } else if (c == 0xFD) { // Two-byte token FD
                c = fgetc(in);
                if (c >= 0x81 && c <= 0x86) fprintf(out, "%s", gw_tokens_FD[c - 0x81]);
            } else if (c == 0xFE) { // Two-byte token FE
                c = fgetc(in);
                if (c >= 0x81 && c <= 0xAB) fprintf(out, "%s", gw_tokens_FE[c - 0x81]);
            } else if (c == 0xFF) { // Two-byte token FF
                c = fgetc(in);
                if (c >= 0x81 && c <= 0xA5) fprintf(out, "%s", gw_tokens_FF[c - 0x81]);
            } else if (c == 0x11 || c == 0x12 || c == 0x13 || c == 0x14 || c == 0x15 || c == 0x1C || c == 0x1D) {
                // Numeric literals
                if (c == 0x11) { // 0-9
                    fprintf(out, "%d", fgetc(in));
                } else if (c == 0x12) { // 10-255 (should not happen as prefix, but okay)
                    fprintf(out, "%d", fgetc(in));
                } else if (c == 0x13) { // 10-255?
                    // Not fully implemented - literal number types
                    fgetc(in); fgetc(in); // skip for now
                } else if (c == 0x14) {
                    fgetc(in); fgetc(in); // skip
                } else if (c == 0x1D) {
                    fgetc(in); fgetc(in); fgetc(in); fgetc(in); // skip single precision
                } else if (c == 0x1F) {
                    for(int k=0;k<8;k++) fgetc(in); // skip double
                }
            } else if (c == 0x0F || c == 0x1C) {
                // Byte literals
                int v = fgetc(in);
                if (c == 0x1C) {
                    int v2 = fgetc(in);
                    v = v | (v2 << 8);
                }
                fprintf(out, "%d", v);
            } else {
                fprintf(out, "%c", c);
            }
        }
        fprintf(out, "\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: bin_detok <input.bas> [output.bas]\n");
        return 1;
    }
    
    FILE *in = fopen(argv[1], "rb");
    if (!in) {
        perror("Failed to open input file");
        return 1;
    }

    FILE *out = stdout;
    if (argc >= 3) {
        out = fopen(argv[2], "w");
        if (!out) {
            perror("Failed to open output file");
            fclose(in);
            return 1;
        }
    }

    parse_file(in, out);

    fclose(in);
    if (out != stdout) fclose(out);
    return 0;
}
