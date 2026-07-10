/* =====================================================================
 * gw_detok.c
 * Native GW-BASIC Detokenizer for BASIC++
 * ===================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gw_detok.h"
#include "../memory.h"
#include "../errors.h"

static const char* get_token(int group, int code) {
    if (group == 0x00) {
        switch(code) {
            case 0x81: return "END";
            case 0x82: return "FOR";
            case 0x83: return "NEXT";
            case 0x84: return "DATA";
            case 0x85: return "INPUT";
            case 0x86: return "DIM";
            case 0x87: return "READ";
            case 0x88: return "LET";
            case 0x89: return "GOTO";
            case 0x8a: return "RUN";
            case 0x8b: return "IF";
            case 0x8c: return "RESTORE";
            case 0x8d: return "GOSUB";
            case 0x8e: return "RETURN";
            case 0x8f: return "REM";
            case 0x90: return "STOP";
            case 0x91: return "PRINT";
            case 0x92: return "CLEAR";
            case 0x93: return "LIST";
            case 0x94: return "NEW";
            case 0x95: return "ON";
            case 0x96: return "WAIT";
            case 0x97: return "DEF";
            case 0x98: return "POKE";
            case 0x99: return "CONT";
            case 0x9a: return "";
            case 0x9b: return "";
            case 0x9c: return "OUT";
            case 0x9d: return "LPRINT";
            case 0x9e: return "LLIST";
            case 0x9f: return "";
            case 0xa0: return "WIDTH";
            case 0xa1: return "ELSE";
            case 0xa2: return "TRON";
            case 0xa3: return "TROFF";
            case 0xa4: return "SWAP";
            case 0xa5: return "ERASE";
            case 0xa6: return "EDIT";
            case 0xa7: return "ERROR";
            case 0xa8: return "RESUME";
            case 0xa9: return "DELETE";
            case 0xaa: return "AUTO";
            case 0xab: return "RENUM";
            case 0xac: return "DEFSTR";
            case 0xad: return "DEFINT";
            case 0xae: return "DEFSNG";
            case 0xaf: return "DEFDBL";
            case 0xb0: return "LINE";
            case 0xb1: return "WHILE";
            case 0xb2: return "WEND";
            case 0xb3: return "CALL";
            case 0xb4: return "";
            case 0xb5: return "";
            case 0xb6: return "";
            case 0xb7: return "WRITE";
            case 0xb8: return "OPTION";
            case 0xb9: return "RANDOMIZE";
            case 0xba: return "OPEN";
            case 0xbb: return "CLOSE";
            case 0xbc: return "LOAD";
            case 0xbd: return "MERGE";
            case 0xbe: return "SAVE";
            case 0xbf: return "COLOR";
            case 0xc0: return "CLS";
            case 0xc1: return "MOTOR";
            case 0xc2: return "BSAVE";
            case 0xc3: return "BLOAD";
            case 0xc4: return "SOUND";
            case 0xc5: return "BEEP";
            case 0xc6: return "PSET";
            case 0xc7: return "PRESET";
            case 0xc8: return "SCREEN";
            case 0xc9: return "KEY";
            case 0xca: return "LOCATE";
            case 0xcb: return "";
            case 0xcc: return "TO";
            case 0xcd: return "THEN";
            case 0xce: return "TAB(";
            case 0xcf: return "STEP";
            case 0xd0: return "USR";
            case 0xd1: return "FN";
            case 0xd2: return "SPC(";
            case 0xd3: return "NOT";
            case 0xd4: return "ERL";
            case 0xd5: return "ERR";
            case 0xd6: return "STRING$";
            case 0xd7: return "USING";
            case 0xd8: return "INSTR";
            case 0xd9: return "\'";
            case 0xda: return "VARPTR";
            case 0xdb: return "CSRLIN";
            case 0xdc: return "POINT";
            case 0xdd: return "OFF";
            case 0xde: return "INKEY$";
            case 0xdf: return "";
            case 0xe0: return "";
            case 0xe1: return "";
            case 0xe2: return "";
            case 0xe3: return "";
            case 0xe4: return "";
            case 0xe5: return "";
            case 0xe6: return ">";
            case 0xe7: return "=";
            case 0xe8: return "<";
            case 0xe9: return "+";
            case 0xea: return "-";
            case 0xeb: return "*";
            case 0xec: return "/";
            case 0xed: return "^";
            case 0xee: return "AND";
            case 0xef: return "OR";
            case 0xf0: return "XOR";
            case 0xf1: return "EQV";
            case 0xf2: return "IMP";
            case 0xf3: return "MOD";
            case 0xf4: return "\\";
            case 0xf5: return "";
            case 0xf6: return "";
            case 0xf7: return "";
            case 0xf8: return "";
            case 0xf9: return "";
            case 0xfa: return "";
            case 0xfb: return "";
            case 0xfc: return "";
        }
    }
    if (group == 0xFD) {
        switch(code) {
            case 0x81: return "CVI";
            case 0x82: return "CVS";
            case 0x83: return "CVD";
            case 0x84: return "MKI$";
            case 0x85: return "MKS$";
            case 0x86: return "MKD$";
            case 0x8b: return "EXTERR";
        }
    }
    if (group == 0xFE) {
        switch(code) {
            case 0x81: return "FILES";
            case 0x82: return "FIELD";
            case 0x83: return "SYSTEM";
            case 0x84: return "NAME";
            case 0x85: return "LSET";
            case 0x86: return "RSET";
            case 0x87: return "KILL";
            case 0x88: return "PUT";
            case 0x89: return "GET";
            case 0x8a: return "RESET";
            case 0x8b: return "COMMON";
            case 0x8c: return "CHAIN";
            case 0x8d: return "DATE$";
            case 0x8e: return "TIME$";
            case 0x8f: return "PAINT";
            case 0x90: return "COM";
            case 0x91: return "CIRCLE";
            case 0x92: return "DRAW";
            case 0x93: return "PLAY";
            case 0x94: return "TIMER";
            case 0x95: return "ERDEV";
            case 0x96: return "IOCTL";
            case 0x97: return "CHDIR";
            case 0x98: return "MKDIR";
            case 0x99: return "RMDIR";
            case 0x9a: return "SHELL";
            case 0x9b: return "ENVIRON";
            case 0x9c: return "VIEW";
            case 0x9d: return "WINDOW";
            case 0x9e: return "PMAP";
            case 0x9f: return "PALETTE";
            case 0xa0: return "LCOPY";
            case 0xa1: return "CALLS";
            case 0xa4: return "NOISE";
            case 0xa5: return "PCOPY";
            case 0xa6: return "TERM";
            case 0xa7: return "LOCK";
            case 0xa8: return "UNLOCK";
        }
    }
    if (group == 0xFF) {
        switch(code) {
            case 0x81: return "LEFT$";
            case 0x82: return "RIGHT$";
            case 0x83: return "MID$";
            case 0x84: return "SGN";
            case 0x85: return "INT";
            case 0x86: return "ABS";
            case 0x87: return "SQR";
            case 0x88: return "RND";
            case 0x89: return "SIN";
            case 0x8a: return "LOG";
            case 0x8b: return "EXP";
            case 0x8c: return "COS";
            case 0x8d: return "TAN";
            case 0x8e: return "ATN";
            case 0x8f: return "FRE";
            case 0x90: return "INP";
            case 0x91: return "POS";
            case 0x92: return "LEN";
            case 0x93: return "STR$";
            case 0x94: return "VAL";
            case 0x95: return "ASC";
            case 0x96: return "CHR$";
            case 0x97: return "PEEK";
            case 0x98: return "SPACE$";
            case 0x99: return "OCT$";
            case 0x9a: return "HEX$";
            case 0x9b: return "LPOS";
            case 0x9c: return "CINT";
            case 0x9d: return "CSNG";
            case 0x9e: return "CDBL";
            case 0x9f: return "FIX";
            case 0xa0: return "PEN";
            case 0xa1: return "STICK";
            case 0xa2: return "STRIG";
            case 0xa3: return "EOF";
            case 0xa4: return "LOC";
            case 0xa5: return "LOF";
        }
    }
    return "";
}


bool gw_detok_load(ProgramStore *store, const char *filename) {
    FILE *in = fopen(filename, "rb");
    if (!in) return false;

    int magic = fgetc(in);
    if (magic != 0xFF) {
        fclose(in);
        return false;
    }

    int lsb, msb, ln_lsb, ln_msb, line_num, c, k, v1, v2, val;
    char line_buf[4096];

    program_clear(store);

    while (1) {
        lsb = fgetc(in);
        if (lsb == EOF) break;
        msb = fgetc(in);
        if (msb == EOF) break;
        if (lsb == 0 && msb == 0) break;

        ln_lsb = fgetc(in);
        ln_msb = fgetc(in);
        if (ln_lsb == EOF || ln_msb == EOF) break;
        
        line_num = ln_lsb | (ln_msb << 8);
        int buf_idx = 0;

        while ((c = fgetc(in)) != EOF && c != 0x00) {
            char temp[64];
            const char *append_str = NULL;
            temp[0] = '\0';

            if (c >= 0x81 && c <= 0xFF) {
                if (c == 0xFD || c == 0xFE || c == 0xFF) {
                    int c2 = fgetc(in);
                    append_str = get_token(c, c2);
                } else {
                    append_str = get_token(0, c);
                }
            } else if ((c >= 0x11 && c <= 0x1B) || c == 0x1C || c == 0x1D || c == 0x1F || c == 0x0B || c == 0x0C || c == 0x0E || c == 0x0D) {
                if (c >= 0x11 && c <= 0x1B) {
                    snprintf(temp, sizeof(temp), "%d", c - 0x11);
                } else if (c == 0x0B) {
                    v1 = fgetc(in); v2 = fgetc(in);
                    snprintf(temp, sizeof(temp), "&O%o", v1 | (v2 << 8));
                } else if (c == 0x0C) {
                    v1 = fgetc(in); v2 = fgetc(in);
                    snprintf(temp, sizeof(temp), "&H%X", v1 | (v2 << 8));
                } else if (c == 0x0E || c == 0x0D) {
                    v1 = fgetc(in); v2 = fgetc(in);
                    snprintf(temp, sizeof(temp), "%d", v1 | (v2 << 8));
                } else if (c == 0x1C) {
                    v1 = fgetc(in); v2 = fgetc(in);
                    val = v1 | (v2 << 8);
                    if (val > 32767) val -= 65536;
                    snprintf(temp, sizeof(temp), "%d", val);
                } else if (c == 0x1D) {
                    unsigned char b[4];
                    for(k=0;k<4;k++) b[k] = fgetc(in);
                    if (b[3] == 0) {
                        snprintf(temp, sizeof(temp), "0!");
                    } else {
                        int exp = b[3] - 152;
                        unsigned long mantissa = ((b[2] | 0x80) << 16) | (b[1] << 8) | b[0];
                        double number = ldexp((double)mantissa, exp);
                        if (b[2] & 0x80) number = -number;
                        snprintf(temp, sizeof(temp), "%g!", number);
                    }
                } else if (c == 0x1F) {
                    unsigned char b[8];
                    for(k=0;k<8;k++) b[k] = fgetc(in);
                    if (b[7] == 0) {
                        snprintf(temp, sizeof(temp), "0#");
                    } else {
                        int exp = b[7] - 184;
                        unsigned long long mantissa = ((unsigned long long)(b[6] | 0x80) << 48) |
                                                      ((unsigned long long)b[5] << 40) |
                                                      ((unsigned long long)b[4] << 32) |
                                                      ((unsigned long long)b[3] << 24) |
                                                      ((unsigned long long)b[2] << 16) |
                                                      ((unsigned long long)b[1] << 8) |
                                                      ((unsigned long long)b[0]);
                        double number = ldexp((double)mantissa, exp);
                        if (b[6] & 0x80) number = -number;
                        snprintf(temp, sizeof(temp), "%g#", number);
                    }
                }
                append_str = temp;
            } else if (c == 0x0F) {
                snprintf(temp, sizeof(temp), "%d", fgetc(in));
                append_str = temp;
            } else {
                if (buf_idx < sizeof(line_buf) - 1) {
                    line_buf[buf_idx++] = (char)c;
                }
                continue;
            }

            if (append_str) {
                size_t len = strlen(append_str);
                if (buf_idx + len < sizeof(line_buf) - 1) {
                    strcpy(line_buf + buf_idx, append_str);
                    buf_idx += (int)len;
                }
            }
        }
        line_buf[buf_idx] = '\0';
        
        char full_line[4096];
        snprintf(full_line, sizeof(full_line), "%d %s", line_num, line_buf);
        
        if (program_insert(store, (double)line_num, full_line) != 0) {
            fclose(in);
            return false;
        }
    }

    fclose(in);
    return true;
}
bool gw_detok_to_file(const char *in_filename, const char *out_filename) {
    FILE *in = fopen(in_filename, "rb");
    if (!in) return false;

    FILE *out = fopen(out_filename, "w");
    if (!out) {
        fclose(in);
        return false;
    }

    int magic = fgetc(in);
    if (magic != 0xFF) {
        fclose(in);
        fclose(out);
        return false;
    }

    int lsb, msb, ln_lsb, ln_msb, line_num, c, k, v1, v2, val;

    while (1) {
        lsb = fgetc(in);
        if (lsb == EOF) break;
        msb = fgetc(in);
        if (msb == EOF) break;
        if (lsb == 0 && msb == 0) break;

        ln_lsb = fgetc(in);
        ln_msb = fgetc(in);
        if (ln_lsb == EOF || ln_msb == EOF) break;
        
        line_num = ln_lsb | (ln_msb << 8);
        fprintf(out, "%d ", line_num);

        while ((c = fgetc(in)) != EOF && c != 0x00) {
            if (c >= 0x81 && c <= 0xFF) {
                if (c == 0xFD || c == 0xFE || c == 0xFF) {
                    int c2 = fgetc(in);
                    fprintf(out, "%s", get_token(c, c2));
                } else {
                    fprintf(out, "%s", get_token(0, c));
                }
            } else if ((c >= 0x11 && c <= 0x1B) || c == 0x1C || c == 0x1D || c == 0x1F || c == 0x0B || c == 0x0C || c == 0x0E || c == 0x0D) {
                if (c >= 0x11 && c <= 0x1B) {
                    fprintf(out, "%d", c - 0x11);
                } else if (c == 0x0B) {
                    v1 = fgetc(in); v2 = fgetc(in);
                    fprintf(out, "&O%o", v1 | (v2 << 8));
                } else if (c == 0x0C) {
                    v1 = fgetc(in); v2 = fgetc(in);
                    fprintf(out, "&H%X", v1 | (v2 << 8));
                } else if (c == 0x0E || c == 0x0D) {
                    v1 = fgetc(in); v2 = fgetc(in);
                    fprintf(out, "%d", v1 | (v2 << 8));
                } else if (c == 0x1C) {
                    v1 = fgetc(in); v2 = fgetc(in);
                    val = v1 | (v2 << 8);
                    if (val > 32767) val -= 65536;
                    fprintf(out, "%d", val);
                } else if (c == 0x1D) {
                    unsigned char b[4];
                    for(k=0;k<4;k++) b[k] = fgetc(in);
                    if (b[3] == 0) {
                        fprintf(out, "0!");
                    } else {
                        int exp = b[3] - 152;
                        unsigned long mantissa = ((b[2] | 0x80) << 16) | (b[1] << 8) | b[0];
                        double number = ldexp((double)mantissa, exp);
                        if (b[2] & 0x80) number = -number;
                        fprintf(out, "%g!", number);
                    }
                } else if (c == 0x1F) {
                    unsigned char b[8];
                    for(k=0;k<8;k++) b[k] = fgetc(in);
                    if (b[7] == 0) {
                        fprintf(out, "0#");
                    } else {
                        int exp = b[7] - 184;
                        unsigned long long mantissa = ((unsigned long long)(b[6] | 0x80) << 48) |
                                                      ((unsigned long long)b[5] << 40) |
                                                      ((unsigned long long)b[4] << 32) |
                                                      ((unsigned long long)b[3] << 24) |
                                                      ((unsigned long long)b[2] << 16) |
                                                      ((unsigned long long)b[1] << 8) |
                                                      ((unsigned long long)b[0]);
                        double number = ldexp((double)mantissa, exp);
                        if (b[6] & 0x80) number = -number;
                        fprintf(out, "%g#", number);
                    }
                }
            } else if (c == 0x0F) {
                fprintf(out, "%d", fgetc(in));
            } else {
                fprintf(out, "%c", c);
            }
        }
        fprintf(out, "\n");
    }

    fclose(in);
    fclose(out);
    return true;
}
