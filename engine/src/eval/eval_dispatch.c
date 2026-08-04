/*
 * BASIC++ Engine
 * Copyright (c) 2026, BASIC++ Project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file eval_dispatch.c
 * @brief Dispatch functions for evaluation
 */

#include "eval/eval_internal.h"
#include "runtime/file.h"
#include "runtime/variables.h"
#include "runtime/map.h"
#include "runtime/funcreg.h"
#include "core/dialect.h"
#include "security/security.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef BPP_LITE_BUILD
#include "memory/segmented_mem.h"
#endif

bool eval_is_builtin_function(const char *name) {
    char uname[64];
    size_t i = 0;
    while (name[i] && i < 63) {
        uname[i] = (char)toupper((unsigned char)name[i]);
        i++;
    }
    uname[i] = '\0';

    if (i > 0 && uname[i - 1] != '$' && i < 62) {
        char test_name[64];
        strcpy(test_name, uname);
        strcat(test_name, "$");
        if (strcmp(test_name, "CHR$") == 0 ||
            strcmp(test_name, "STR$") == 0 ||
            strcmp(test_name, "LEFT$") == 0 ||
            strcmp(test_name, "RIGHT$") == 0 ||
            strcmp(test_name, "MID$") == 0 ||
            strcmp(test_name, "UCASE$") == 0 ||
            strcmp(test_name, "LCASE$") == 0 ||
            strcmp(test_name, "LTRIM$") == 0 ||
            strcmp(test_name, "RTRIM$") == 0 ||
            strcmp(test_name, "TRIM$") == 0 ||
            strcmp(test_name, "SPACE$") == 0 ||
            strcmp(test_name, "STRING$") == 0 ||
            strcmp(test_name, "REMOVE$") == 0 ||
            strcmp(test_name, "REPLACE$") == 0 ||
            strcmp(test_name, "HEX$") == 0 ||
            strcmp(test_name, "OCT$") == 0 ||
            strcmp(test_name, "BIN$") == 0 ||
            strcmp(test_name, "EDIT$") == 0 ||
            strcmp(test_name, "NUM$") == 0 ||
            strcmp(test_name, "TCASE$") == 0 ||
            strcmp(test_name, "ICASE$") == 0 ||
            strcmp(test_name, "REVERSE$") == 0 ||
            strcmp(test_name, "BASEDIR$") == 0 ||
            strcmp(test_name, "BASENAME$") == 0 ||
            strcmp(test_name, "BASEPATH$") == 0 ||
            strcmp(test_name, "HOSTNAME$") == 0 ||
            strcmp(test_name, "USERNAME$") == 0 ||
            strcmp(test_name, "PATH$") == 0 ||
            strcmp(test_name, "FILEMOD$") == 0 ||
            strcmp(test_name, "ERR$") == 0) {
            strcpy(uname, test_name);
            i = strlen(uname);
        }
    }

    if (strcmp(uname, "INKEY$") == 0 ||
        strcmp(uname, "PEN") == 0 ||
        strcmp(uname, "TIME$") == 0 ||
        strcmp(uname, "DATE$") == 0 ||
        strcmp(uname, "TIMER") == 0 ||
        strcmp(uname, "ALARM") == 0 ||
        strcmp(uname, "ALARM$") == 0 ||
        strcmp(uname, "EXISTS") == 0 ||
        strcmp(uname, "RANDOMIZE") == 0 ||
        strcmp(uname, "GUID$") == 0 ||
        strcmp(uname, "TIM") == 0 ||
        strcmp(uname, "TRUE") == 0 ||
        strcmp(uname, "FALSE") == 0 ||
        strcmp(uname, "TI") == 0 ||
        strcmp(uname, "TIME") == 0 ||
        strcmp(uname, "DATE") == 0 ||
        strcmp(uname, "TI$") == 0 ||
        strcmp(uname, "CLOCK$") == 0 ||
        strcmp(uname, "TZ") == 0 ||
        strcmp(uname, "TZ$") == 0 ||
        strcmp(uname, "TIMEZONE$") == 0 ||
        strcmp(uname, "UTC") == 0 ||
        strcmp(uname, "CSRLIN") == 0 ||
        strcmp(uname, "POS") == 0 ||
        strcmp(uname, "LPOS") == 0 ||
        strcmp(uname, "DAY") == 0 ||
        strcmp(uname, "MONTH") == 0 ||
        strcmp(uname, "YEAR") == 0 ||
        strcmp(uname, "DAY$") == 0 ||
        strcmp(uname, "MONTH$") == 0 ||
        strcmp(uname, "HOURS") == 0 ||
        strcmp(uname, "MINUTES") == 0 ||
        strcmp(uname, "SECONDS") == 0 ||
        strcmp(uname, "JIFFIES") == 0 ||
        strcmp(uname, "TICKS") == 0 ||
        strcmp(uname, "HOSTNAME$") == 0 ||
        strcmp(uname, "USERNAME$") == 0 ||
        strcmp(uname, "BASEDIR$") == 0 ||
        strcmp(uname, "BASEPATH$") == 0 ||
        strcmp(uname, "BASENAME$") == 0 ||
        strcmp(uname, "PATH$") == 0 ||
        strcmp(uname, "VER") == 0 ||
        strcmp(uname, "MEM") == 0 ||
        strcmp(uname, "SIZE") == 0 ||
        strcmp(uname, "PLAY") == 0 ||
        strcmp(uname, "TASK") == 0 ||
        strcmp(uname, "SQR") == 0 ||
        strcmp(uname, "ABS") == 0 ||
        strcmp(uname, "SIN") == 0 ||
        strcmp(uname, "COS") == 0 ||
        strcmp(uname, "TAN") == 0 ||
        strcmp(uname, "ATN") == 0 ||
        strcmp(uname, "LOG") == 0 ||
        strcmp(uname, "EXP") == 0 ||
        strcmp(uname, "INT") == 0 ||
        strcmp(uname, "FIX") == 0 ||
        strcmp(uname, "DET") == 0 ||
        strcmp(uname, "DOT") == 0 ||
        strcmp(uname, "CROSS") == 0 ||
        strcmp(uname, "RND") == 0 ||
        strcmp(uname, "LEN") == 0 ||
        strcmp(uname, "ASC") == 0 ||
        strcmp(uname, "CHR$") == 0 ||
        strcmp(uname, "CINT") == 0 ||
        strcmp(uname, "CSNG") == 0 ||
        strcmp(uname, "CDBL") == 0 ||
#ifndef BPP_LITE_BUILD
        strcmp(uname, "VARPTR") == 0 ||
        strcmp(uname, "VARPTR$") == 0 ||
        strcmp(uname, "VARSEG") == 0 ||
        strcmp(uname, "SADD") == 0 ||
#endif
        strcmp(uname, "VAL") == 0 ||
        strcmp(uname, "STR$") == 0 ||
        strcmp(uname, "LEFT$") == 0 ||
        strcmp(uname, "RIGHT$") == 0 ||
        strcmp(uname, "MID$") == 0 ||
        strcmp(uname, "INSTR") == 0 ||
        strcmp(uname, "UCASE$") == 0 ||
        strcmp(uname, "LCASE$") == 0 ||
        strcmp(uname, "LTRIM$") == 0 ||
        strcmp(uname, "RTRIM$") == 0 ||
        strcmp(uname, "TRIM$") == 0 ||
        strcmp(uname, "SPACE$") == 0 ||
        strcmp(uname, "STRING$") == 0 ||
        strcmp(uname, "REPLACE$") == 0 ||
        strcmp(uname, "HEX$") == 0 ||
        strcmp(uname, "OCT$") == 0 ||
        strcmp(uname, "BIN$") == 0 ||
        strcmp(uname, "EDIT$") == 0 ||
        strcmp(uname, "NUM$") == 0 ||
        strcmp(uname, "TCASE$") == 0 ||
        strcmp(uname, "ICASE$") == 0 ||
        strcmp(uname, "REVERSE$") == 0 ||
        strcmp(uname, "REMOVE$") == 0 ||
        strcmp(uname, "REMOVE") == 0 ||
        strcmp(uname, "HASH") == 0 ||
        strcmp(uname, "UBOUND") == 0 ||
        strcmp(uname, "LBOUND") == 0 ||
        strcmp(uname, "EOF") == 0 ||
        strcmp(uname, "LOF") == 0 ||
        strcmp(uname, "LOC") == 0 ||
        strcmp(uname, "SEEK") == 0 ||
        strcmp(uname, "HELP") == 0 ||
        strcmp(uname, "HELP$") == 0 ||
        strcmp(uname, "FREEFILE") == 0 ||
        strcmp(uname, "INPUT$") == 0 ||
        strcmp(uname, "SCREEN") == 0 ||
        strcmp(uname, "IOCTL$") == 0 ||
        strcmp(uname, "TXNSTATUS") == 0 ||
        strcmp(uname, "SIOREAD$") == 0 ||
        strcmp(uname, "SIOREADLN$") == 0 ||
        strcmp(uname, "SIOWRITE") == 0 ||
        strcmp(uname, "SIOSEEK") == 0 ||
        strcmp(uname, "SIOFLUSH") == 0 ||
        strcmp(uname, "SIOSTATUS") == 0 ||
        strcmp(uname, "SIOAVAIL") == 0 ||
        strcmp(uname, "BIOREAD$") == 0 ||
        strcmp(uname, "BIOWRITE") == 0 ||
        strcmp(uname, "BIOCOPY") == 0 ||
        strcmp(uname, "BIOFILL") == 0 ||
        strcmp(uname, "BIOSTATUS") == 0 ||
        strcmp(uname, "BIOSIZE") == 0 ||
        strcmp(uname, "BIOCHECKSUM") == 0 ||
        strcmp(uname, "BIOCOMPARE") == 0 ||
        strcmp(uname, "FILEATTR") == 0 ||
        strcmp(uname, "MKI$") == 0 ||
        strcmp(uname, "MKS$") == 0 ||
        strcmp(uname, "MKD$") == 0 ||
        strcmp(uname, "CVI") == 0 ||
        strcmp(uname, "CVS") == 0 ||
        strcmp(uname, "CVD") == 0 ||
        strcmp(uname, "DEVICECOUNT") == 0 ||
        strcmp(uname, "DEVICE$") == 0 ||
        strcmp(uname, "DEVICECLASS$") == 0 ||
        strcmp(uname, "DEVICEINFO$") == 0 ||
        strcmp(uname, "POLL") == 0 ||
#if BPP_SUPPORT_NET
        strcmp(uname, "NSTATUS") == 0 ||
        strcmp(uname, "NCONNECTED") == 0 ||
        strcmp(uname, "NHTTPSTATUS") == 0 ||
        strcmp(uname, "HTTP_GET$") == 0 ||
#endif
#if BPP_SUPPORT_BIOS
        strcmp(uname, "MEMMAP$") == 0 ||
#endif
        strcmp(uname, "PEEK") == 0 ||
        strcmp(uname, "MAP") == 0 ||
        strcmp(uname, "MAP_NEW") == 0 ||
        strcmp(uname, "MAP_SET") == 0 ||
        strcmp(uname, "MAP_GET") == 0 ||
        strcmp(uname, "MAP_GET$") == 0 ||
        strcmp(uname, "MAP_REMOVE") == 0 ||
        strcmp(uname, "MAP_COUNT") == 0 ||
        strcmp(uname, "MAP_KEY$") == 0 ||
        strcmp(uname, "MAP_HAS") == 0 ||
        strcmp(uname, "JSON_PARSE") == 0 ||
        strcmp(uname, "JSON_STRINGIFY$") == 0 ||
        strcmp(uname, "XML_PARSE") == 0 ||
        strcmp(uname, "XML_STRINGIFY$") == 0 ||
        strcmp(uname, "USR") == 0 || strcmp(uname, "USR0") == 0 ||
        strcmp(uname, "USR1") == 0 || strcmp(uname, "USR2") == 0 ||
        strcmp(uname, "USR3") == 0 || strcmp(uname, "USR4") == 0 ||
        strcmp(uname, "USR5") == 0 || strcmp(uname, "USR6") == 0 ||
        strcmp(uname, "USR7") == 0 || strcmp(uname, "USR8") == 0 ||
        strcmp(uname, "USR9") == 0 ||
        strcmp(uname, "ERDEV") == 0 || strcmp(uname, "ERDEV$") == 0 ||
        strcmp(uname, "EXTERR") == 0 ||
        strcmp(uname, "HASH$") == 0 || strcmp(uname, "SALT$") == 0 ||
        strcmp(uname, "AUDITCRACK") == 0 || strcmp(uname, "AUDITCRACK$") == 0 ||
        strcmp(uname, "SANDBOXAUDIT") == 0 || strcmp(uname, "VMCHECK") == 0 ||
        strcmp(uname, "NETHOST$") == 0 || strcmp(uname, "NETIP$") == 0 ||
        strcmp(uname, "YAML_PARSE") == 0 ||
        strcmp(uname, "YAML_STRINGIFY$") == 0 ||
        strcmp(uname, "INI_PARSE") == 0 ||
        strcmp(uname, "INI_STRINGIFY$") == 0 ||
        strcmp(uname, "DIALECT_LOAD") == 0 ||
        strcmp(uname, "DIALECT_REGISTER") == 0 ||
        strcmp(uname, "DIALECT_VALIDATE") == 0 ||
        strcmp(uname, "DIALECT_DOC$") == 0 ||
        strcmp(uname, "ENVIRON$") == 0 ||
        strcmp(uname, "DIR$") == 0 ||
        strcmp(uname, "GETATTR") == 0 ||
        strcmp(uname, "_SHL") == 0 || strcmp(uname, "BITS.SHL") == 0 ||
        strcmp(uname, "_SHR") == 0 || strcmp(uname, "BITS.SHR") == 0 ||
        strcmp(uname, "_READBIT") == 0 || strcmp(uname, "BITS.READ") == 0 ||
        strcmp(uname, "_SETBIT") == 0 || strcmp(uname, "BITS.SET") == 0 ||
        strcmp(uname, "_RESETBIT") == 0 || strcmp(uname, "BITS.RESET") == 0 ||
        strcmp(uname, "_TOGGLEBIT") == 0 || strcmp(uname, "BITS.TOGGLE") == 0 ||
        strcmp(uname, "_BITCOUNT") == 0 || strcmp(uname, "BITS.COUNT") == 0 ||
        strcmp(uname, "_ACOS") == 0 || strcmp(uname, "MATH.ACOS") == 0 ||
        strcmp(uname, "_ASIN") == 0 || strcmp(uname, "MATH.ASIN") == 0 ||
        strcmp(uname, "_ATAN2") == 0 || strcmp(uname, "MATH.ATAN2") == 0 ||
        strcmp(uname, "_ACOSH") == 0 || strcmp(uname, "MATH.ACOSH") == 0 ||
        strcmp(uname, "_ASINH") == 0 || strcmp(uname, "MATH.ASINH") == 0 ||
        strcmp(uname, "_ATANH") == 0 || strcmp(uname, "MATH.ATANH") == 0 ||
        strcmp(uname, "_CEIL") == 0 || strcmp(uname, "MATH.CEIL") == 0 ||
        strcmp(uname, "_HYPOT") == 0 || strcmp(uname, "MATH.HYPOT") == 0 ||
        strcmp(uname, "_PI") == 0 || strcmp(uname, "MATH.PI") == 0 ||
        strcmp(uname, "_D2R") == 0 || strcmp(uname, "MATH.D2R") == 0 ||
        strcmp(uname, "_R2D") == 0 || strcmp(uname, "MATH.R2D") == 0 ||
        strcmp(uname, "_D2G") == 0 || strcmp(uname, "MATH.D2G") == 0 ||
        strcmp(uname, "_G2D") == 0 || strcmp(uname, "MATH.G2D") == 0 ||
        strcmp(uname, "INP") == 0) {
        return true;
    }

    if (funcreg_find_by_name(uname) != NULL) {
        return true;
    }

    return false;
}

char *eval_read_file_to_string(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }
    fseek(fp, 0, SEEK_SET);
    char *buf = calloc(size + 1, 1);
    if (!buf) {
        fclose(fp);
        return NULL;
    }
    size_t read_bytes = fread(buf, 1, size, fp);
    buf[read_bytes] = '\0';
    fclose(fp);
    return buf;
}

BValue eval_builtin_function(VMContext *vm, const char *name, LexerContext *lex, bool has_parens, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    char uname[64];
    size_t i = 0;
    while (name[i] && i < 63) {
        uname[i] = (char)toupper((unsigned char)name[i]);
        i++;
    }
    uname[i] = '\0';

    if (i > 0 && uname[i - 1] != '$' && i < 62) {
        char test_name[64];
        strcpy(test_name, uname);
        strcat(test_name, "$");
        if (strcmp(test_name, "CHR$") == 0 ||
            strcmp(test_name, "STR$") == 0 ||
            strcmp(test_name, "LEFT$") == 0 ||
            strcmp(test_name, "RIGHT$") == 0 ||
            strcmp(test_name, "MID$") == 0 ||
            strcmp(test_name, "UCASE$") == 0 ||
            strcmp(test_name, "LCASE$") == 0 ||
            strcmp(test_name, "LTRIM$") == 0 ||
            strcmp(test_name, "RTRIM$") == 0 ||
            strcmp(test_name, "TRIM$") == 0 ||
            strcmp(test_name, "SPACE$") == 0 ||
            strcmp(test_name, "STRING$") == 0 ||
            strcmp(test_name, "REMOVE$") == 0 ||
            strcmp(test_name, "REPLACE$") == 0 ||
            strcmp(test_name, "HEX$") == 0 ||
            strcmp(test_name, "OCT$") == 0 ||
            strcmp(test_name, "BIN$") == 0 ||
            strcmp(test_name, "EDIT$") == 0 ||
            strcmp(test_name, "NUM$") == 0 ||
            strcmp(test_name, "TCASE$") == 0 ||
            strcmp(test_name, "ICASE$") == 0 ||
            strcmp(test_name, "REVERSE$") == 0 ||
            strcmp(test_name, "BASEDIR$") == 0 ||
            strcmp(test_name, "BASENAME$") == 0 ||
            strcmp(test_name, "BASEPATH$") == 0 ||
            strcmp(test_name, "HOSTNAME$") == 0 ||
            strcmp(test_name, "USERNAME$") == 0 ||
            strcmp(test_name, "PATH$") == 0 ||
            strcmp(test_name, "FILEMOD$") == 0 ||
            strcmp(test_name, "ERR$") == 0) {
            strcpy(uname, test_name);
            i = strlen(uname);
        }
    }

    if (strcmp(uname, "UBOUND") == 0 || strcmp(uname, "LBOUND") == 0) {
        bool is_u = (strcmp(uname, "UBOUND") == 0);
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT) {
            err->code = 2;
            err->message = "Expected array name in UBOUND/LBOUND";
            return res;
        }
        char arr_name[256];
        size_t clen = (name_tok.length < sizeof(arr_name) - 1) ? name_tok.length : sizeof(arr_name) - 1;
        memcpy(arr_name, name_tok.start, clen);
        arr_name[clen] = '\0';

        int dim = 1;
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_COMMA) {
            lex_next(lex); /* Consume ',' */
            BValue dim_val = eval_expression(vm, lex, err);
            if (err->code != 0) return res;
            if (dim_val.type == VAL_STRING) {
                err->code = 13;
                err->message = "Dimension must be numeric";
                return res;
            }
            dim = (int)dim_val.as.number;
        }

        next_tok = lex_next(lex);
        if (next_tok.type != TOK_RPAREN) {
            err->code = 2;
            err->message = "Expected ')' in UBOUND/LBOUND";
            return res;
        }

        if (is_u) {
            bool found = false;
            int u = arr_ubound(vm_get_arr(vm), arr_name, dim, &found);
            if (!found) {
                err->code = 9;
                err->message = "Array not dimensioned";
                return res;
            }
            res.type = VAL_NUMBER;
            res.as.number = (double)u;
        } else {
            if (!arr_exists(vm_get_arr(vm), arr_name)) {
                err->code = 9;
                err->message = "Array not dimensioned";
                return res;
            }
            res.type = VAL_NUMBER;
            res.as.number = (double)arr_get_option_base(vm_get_arr(vm));
        }
        return res;
    }

    if (strcmp(uname, "DET") == 0) {
        if (!has_parens) {
            res.type = VAL_NUMBER;
            res.as.number = arr_get_last_det(vm_get_arr(vm));
            return res;
        }
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT) {
            err->code = 2; err->message = "Expected array name in DET()"; return res;
        }
        char arr_name[256];
        size_t clen = (name_tok.length < sizeof(arr_name) - 1) ? name_tok.length : sizeof(arr_name) - 1;
        memcpy(arr_name, name_tok.start, clen);
        arr_name[clen] = '\0';
        if (lex_next(lex).type != TOK_RPAREN) {
            err->code = 2; err->message = "Expected ')' in DET()"; return res;
        }
        
        bool found = false;
        int d1 = arr_ubound(vm_get_arr(vm), arr_name, 1, &found);
        int d2 = arr_ubound(vm_get_arr(vm), arr_name, 2, &found);
        if (!found || d1 != d2 || d1 < 1) {
            err->code = 9; err->message = "DET expects a square matrix"; return res;
        }
        int total_size = 0;
        BValue *elems = arr_get_flat_elements(vm_get_arr(vm), arr_name, &total_size);
        if (!elems) { err->code = 9; err->message = "Array not found"; return res; }
        
        /* Basic 2x2 or 3x3 determinant calculation for now */
        int base = arr_get_option_base(vm_get_arr(vm));
        int n = d1 - base + 1;
        double det = 0.0;
        if (n == 2) {
            double a = elems[0*n + 0].as.number;
            double b = elems[0*n + 1].as.number;
            double c = elems[1*n + 0].as.number;
            double d = elems[1*n + 1].as.number;
            det = a*d - b*c;
        } else if (n == 3) {
            double a = elems[0*n + 0].as.number;
            double b = elems[0*n + 1].as.number;
            double c = elems[0*n + 2].as.number;
            double d = elems[1*n + 0].as.number;
            double e = elems[1*n + 1].as.number;
            double f = elems[1*n + 2].as.number;
            double g = elems[2*n + 0].as.number;
            double h = elems[2*n + 1].as.number;
            double i_val = elems[2*n + 2].as.number;
            det = a*(e*i_val - f*h) - b*(d*i_val - f*g) + c*(d*h - e*g);
        } else {
            err->code = 5; err->message = "DET only supports 2x2 and 3x3 matrices in this version"; return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = det;
        return res;
    }

    if (strcmp(uname, "DOT") == 0) {
        if (lex_next(lex).type != TOK_LPAREN) { err->code = 2; err->message = "Expected '(' in DOT()"; return res; }
        
        BppToken name_tok1 = lex_next(lex);
        if (name_tok1.type != TOK_IDENT) { err->code = 2; err->message = "Expected first array name in DOT()"; return res; }
        char arr1[256];
        size_t clen1 = (name_tok1.length < sizeof(arr1) - 1) ? name_tok1.length : sizeof(arr1) - 1;
        memcpy(arr1, name_tok1.start, clen1);
        arr1[clen1] = '\0';
        
        if (lex_next(lex).type != TOK_COMMA) { err->code = 2; err->message = "Expected ',' in DOT()"; return res; }
        
        BppToken name_tok2 = lex_next(lex);
        if (name_tok2.type != TOK_IDENT) { err->code = 2; err->message = "Expected second array name in DOT()"; return res; }
        char arr2[256];
        size_t clen2 = (name_tok2.length < sizeof(arr2) - 1) ? name_tok2.length : sizeof(arr2) - 1;
        memcpy(arr2, name_tok2.start, clen2);
        arr2[clen2] = '\0';
        
        if (lex_next(lex).type != TOK_RPAREN) { err->code = 2; err->message = "Expected ')' in DOT()"; return res; }
        
        int b1[4], b2[4];
        int dim1 = arr_get_dimensions(vm_get_arr(vm), arr1, b1, 4);
        int dim2 = arr_get_dimensions(vm_get_arr(vm), arr2, b2, 4);
        
        if (dim1 < 1 || dim2 < 1 || b1[0] != b2[0]) {
            err->code = 9; err->message = "DOT expects two 1D arrays of same size"; return res;
        }
        
        int sz1=0, sz2=0;
        BValue *e1 = arr_get_flat_elements(vm_get_arr(vm), arr1, &sz1);
        BValue *e2 = arr_get_flat_elements(vm_get_arr(vm), arr2, &sz2);
        if (!e1 || !e2 || sz1 != sz2) { err->code = 9; err->message = "Array mismatch in DOT"; return res; }
        
        double dot = 0.0;
        for (int idx=0; idx<sz1; idx++) {
            dot += e1[idx].as.number * e2[idx].as.number;
        }
        res.type = VAL_NUMBER;
        res.as.number = dot;
        return res;
    }

#ifndef BPP_LITE_BUILD
    if (strcmp(uname, "VARPTR") == 0 || strcmp(uname, "VARPTR$") == 0 || strcmp(uname, "VARSEG") == 0 || strcmp(uname, "SADD") == 0) {
        bool is_seg = (strcmp(uname, "VARSEG") == 0);
        bool is_sadd = (strcmp(uname, "SADD") == 0);
        bool is_str = (strcmp(uname, "VARPTR$") == 0);
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT) {
            err->code = 2;
            err->message = "Expected variable name in VARPTR/VARSEG/SADD";
            return res;
        }
        char var_name[64];
        size_t clen = (name_tok.length < 63) ? name_tok.length : 63;
        memcpy(var_name, name_tok.start, clen);
        var_name[clen] = '\0';
        
        VariableContext *var = vm_get_var(vm);
        BValue *target = var_lookup(var, var_name, true);
        
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex); /* Consume '(' */
            while (lex_peek(lex).type != TOK_RPAREN && lex_peek(lex).type != TOK_EOF) {
                lex_next(lex);
            }
            if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
        }
        
        BppToken next_tok = lex_next(lex);
        if (next_tok.type != TOK_RPAREN) {
            err->code = 2;
            err->message = "Expected ')'";
            return res;
        }
        
        uint32_t handle = vmem_register_handle(vm_get_vmem(vm), target, is_sadd);

        if (is_str) {
            char desc[4];
            int type_code = 3; /* default string */
            if (target) {
                if (target->type == VAL_NUMBER) {
                    type_code = 8; /* double */
                } else if (target->type == VAL_STRING) {
                    type_code = 3; /* string */
                }
            }
            desc[0] = (char)type_code;
            desc[1] = (char)(handle & 0xFF);
            desc[2] = (char)((handle >> 8) & 0xFF);
            desc[3] = '\0';
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), desc, 3);
        } else {
            res.type = VAL_NUMBER;
            if (is_seg) {
                res.as.number = (double)((handle >> 16) & 0xFFFF);
            } else {
                res.as.number = (double)(handle & 0xFFFF);
            }
        }
        return res;
    }
#endif

    if (!has_parens) {
        return eval_builtin_function_impl(vm, uname, 0, NULL, err);
    }

    BValue args[10];
    int arg_count = 0;
    memset(args, 0, sizeof(args));

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_RPAREN) {
        while (true) {
            if (arg_count >= 10) {
                err->code = 2;
                err->message = "Too many arguments for function";
                break;
            }
            args[arg_count] = eval_expression(vm, lex, err);
            if (err->code != 0) break;
            arg_count++;

            tok = lex_peek(lex);
            if (tok.type == TOK_COMMA) {
                lex_next(lex); /* Consume ',' */
            } else if (tok.type == TOK_RPAREN) {
                break;
            } else {
                err->code = 2;
                err->message = "Expected ',' or ')' in function call";
                break;
            }
        }
    }

    if (err->code == 0) {
        tok = lex_next(lex);
        if (tok.type != TOK_RPAREN) {
            err->code = 2;
            err->message = "Expected ')' to close function call";
        }
    }

    if (err->code == 0) {
        res = eval_builtin_function_impl(vm, uname, arg_count, args, err);
    }

    /* Standard leak-free cleanup for all args if function execution failed or parsing failed mid-way */
    if (err->code != 0) {
        for (int j = 0; j < arg_count; j++) {
            if (args[j].type == VAL_STRING && args[j].as.string) {
                str_release(vm_get_str(vm), args[j].as.string);
                args[j].as.string = NULL;
            } else if (args[j].type == VAL_MAP && args[j].as.map) {
                bpp_map_release(vm_get_str(vm), args[j].as.map);
                args[j].as.map = NULL;
            }
        }
    }

    return res;
}
