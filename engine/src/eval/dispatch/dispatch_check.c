// FILENAME: dispatch_check.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (dispatch_internal.h)
// Provides core logic and interface definitions for dispatch_check within BASIC++.
//
// ---- Includes ----

#include "eval/dispatch_internal.h"

//
// ---- Name Normalization ----

void normalize_func_name(const char *in, char *out, size_t max_len) {
    size_t i = 0;
    while (in[i] && i < max_len - 1) {
        out[i] = (char)runtime_toupper((unsigned char)in[i]);
        i++;
    }
    out[i] = '\0';

    if (i > 0 && out[i - 1] != '$' && i < max_len - 2) {
        char test_name[64];
        runtime_strncpy(test_name, out, sizeof(test_name) - 2);
        test_name[sizeof(test_name) - 2] = '\0';
        runtime_strcat(test_name, "$");
        if (runtime_strcmp(test_name, "CHR$") == 0 ||
            runtime_strcmp(test_name, "STR$") == 0 ||
            runtime_strcmp(test_name, "LEFT$") == 0 ||
            runtime_strcmp(test_name, "RIGHT$") == 0 ||
            runtime_strcmp(test_name, "MID$") == 0 ||
            runtime_strcmp(test_name, "UCASE$") == 0 ||
            runtime_strcmp(test_name, "LCASE$") == 0 ||
            runtime_strcmp(test_name, "LTRIM$") == 0 ||
            runtime_strcmp(test_name, "RTRIM$") == 0 ||
            runtime_strcmp(test_name, "TRIM$") == 0 ||
            runtime_strcmp(test_name, "SPACE$") == 0 ||
            runtime_strcmp(test_name, "STRING$") == 0 ||
            runtime_strcmp(test_name, "REMOVE$") == 0 ||
            runtime_strcmp(test_name, "REPLACE$") == 0 ||
            runtime_strcmp(test_name, "HEX$") == 0 ||
            runtime_strcmp(test_name, "OCT$") == 0 ||
            runtime_strcmp(test_name, "BIN$") == 0 ||
            runtime_strcmp(test_name, "EDIT$") == 0 ||
            runtime_strcmp(test_name, "TCASE$") == 0 ||
            runtime_strcmp(test_name, "ICASE$") == 0 ||
            runtime_strcmp(test_name, "REVERSE$") == 0 ||
            runtime_strcmp(test_name, "SHUFFLE$") == 0 ||
            runtime_strcmp(test_name, "CATEGORY$") == 0 ||
            runtime_strcmp(test_name, "CATEGORIES$") == 0 ||
            runtime_strcmp(test_name, "BASEDIR$") == 0 ||
            runtime_strcmp(test_name, "SEG$") == 0 ||
            runtime_strcmp(test_name, "ERT$") == 0 ||
            runtime_strcmp(test_name, "UPS$") == 0 ||
            runtime_strcmp(test_name, "SUM$") == 0 ||
            runtime_strcmp(test_name, "DIF$") == 0 ||
            runtime_strcmp(test_name, "PROD$") == 0 ||
            runtime_strcmp(test_name, "QUO$") == 0 ||
            runtime_strcmp(test_name, "PLACE$") == 0 ||
            runtime_strcmp(test_name, "BASENAME$") == 0 ||
            runtime_strcmp(test_name, "BASEPATH$") == 0 ||
            runtime_strcmp(test_name, "HOSTNAME$") == 0 ||
            runtime_strcmp(test_name, "USERNAME$") == 0 ||
            runtime_strcmp(test_name, "PATH$") == 0 ||
            runtime_strcmp(test_name, "FILEMOD$") == 0 ||
            runtime_strcmp(test_name, "PACK$") == 0 ||
            runtime_strcmp(test_name, "MICROPLEX$") == 0 ||
            runtime_strcmp(test_name, "COMMAND$") == 0 ||
            runtime_strcmp(test_name, "INPUTBOX$") == 0 ||
            runtime_strcmp(test_name, "MKSMBF$") == 0 ||
            runtime_strcmp(test_name, "MKDMBF$") == 0 ||
            runtime_strcmp(test_name, "FILEOPENBOX$") == 0 ||
            runtime_strcmp(test_name, "FILESAVEBOX$") == 0 ||
            runtime_strcmp(test_name, "CURDIR$") == 0 ||
            runtime_strcmp(test_name, "PREFIX$") == 0 ||
            runtime_strcmp(test_name, "DIR$") == 0 ||
            runtime_strcmp(test_name, "VER$") == 0 ||
            runtime_strcmp(test_name, "ERR$") == 0) {
            runtime_strncpy(out, test_name, max_len - 1);
            out[max_len - 1] = '\0';
        }
    }
}


//
// ---- Builtin Existence Check ----

bool eval_is_builtin_function(const char *name) {
    if (!name || !*name) return false;

    char uname[64];
    normalize_func_name(name, uname, sizeof(uname));

    // Core Built-in Functions
    if (runtime_strcmp(uname, "INKEY$") == 0 || runtime_strcmp(uname, "PEN") == 0 ||
        runtime_strcmp(uname, "TIME$") == 0 || runtime_strcmp(uname, "DATE$") == 0 ||
        runtime_strcmp(uname, "TIMER") == 0 || runtime_strcmp(uname, "ALARM") == 0 ||
        runtime_strcmp(uname, "ALARM$") == 0 || runtime_strcmp(uname, "EXISTS") == 0 ||
        runtime_strcmp(uname, "RANDOMIZE") == 0 || runtime_strcmp(uname, "GUID$") == 0 ||
        runtime_strcmp(uname, "PACK$") == 0 || runtime_strcmp(uname, "MICROPLEX$") == 0 ||
        runtime_strcmp(uname, "MICROPLEX") == 0 || runtime_strcmp(uname, "TIM") == 0 ||
        runtime_strcmp(uname, "TRUE") == 0 || runtime_strcmp(uname, "FALSE") == 0 ||
        runtime_strcmp(uname, "TI") == 0 || runtime_strcmp(uname, "TIME") == 0 ||
        runtime_strcmp(uname, "DATE") == 0 || runtime_strcmp(uname, "TI$") == 0 ||
        runtime_strcmp(uname, "CLOCK$") == 0 || runtime_strcmp(uname, "CLOCK") == 0 ||
        runtime_strcmp(uname, "PI") == 0 || runtime_strcmp(uname, "TZ") == 0 ||
        runtime_strcmp(uname, "TZ$") == 0 || runtime_strcmp(uname, "TIMEZONE$") == 0 ||
        runtime_strcmp(uname, "UTC") == 0 || runtime_strcmp(uname, "CSRLIN") == 0 ||
        runtime_strcmp(uname, "POS") == 0 || runtime_strcmp(uname, "LPOS") == 0 ||
        runtime_strcmp(uname, "TAB") == 0 || runtime_strcmp(uname, "SPC") == 0 ||
        runtime_strcmp(uname, "ERL") == 0 || runtime_strcmp(uname, "ERR") == 0 ||
        runtime_strcmp(uname, "EOF") == 0 || runtime_strcmp(uname, "LOF") == 0 ||
        runtime_strcmp(uname, "LOC") == 0 || runtime_strcmp(uname, "FREEFILE") == 0 ||
        runtime_strcmp(uname, "POINT") == 0 || runtime_strcmp(uname, "LBOUND") == 0 ||
        runtime_strcmp(uname, "UBOUND") == 0 || runtime_strcmp(uname, "DAY") == 0 ||
        runtime_strcmp(uname, "MONTH") == 0 || runtime_strcmp(uname, "YEAR") == 0 ||
        runtime_strcmp(uname, "DAY$") == 0 || runtime_strcmp(uname, "MONTH$") == 0 ||
        runtime_strcmp(uname, "HOURS") == 0 || runtime_strcmp(uname, "HRS") == 0 ||
        runtime_strcmp(uname, "MINUTES") == 0 || runtime_strcmp(uname, "SECONDS") == 0 ||
        runtime_strcmp(uname, "JULIAN") == 0 || runtime_strcmp(uname, "JULIAN$") == 0 ||
        runtime_strcmp(uname, "DAT") == 0 || runtime_strcmp(uname, "JIFFIES") == 0 ||
        runtime_strcmp(uname, "TICKS") == 0 || runtime_strcmp(uname, "HOSTNAME$") == 0 ||
        runtime_strcmp(uname, "USERNAME$") == 0 || runtime_strcmp(uname, "BASEDIR$") == 0 ||
        runtime_strcmp(uname, "BASEPATH$") == 0 || runtime_strcmp(uname, "BASENAME$") == 0 ||
        runtime_strcmp(uname, "PATH$") == 0 || runtime_strcmp(uname, "COMMAND$") == 0 ||
        runtime_strcmp(uname, "COMMAND") == 0 || runtime_strcmp(uname, "DOEVENTS") == 0 ||
        runtime_strcmp(uname, "VER") == 0 || runtime_strcmp(uname, "MEM") == 0 ||
        runtime_strcmp(uname, "FRE") == 0 || runtime_strcmp(uname, "SIZE") == 0 ||
        runtime_strcmp(uname, "PLAY") == 0) return true;

    // Math Built-in Functions
    if (runtime_strcmp(uname, "SQR") == 0 || runtime_strcmp(uname, "ABS") == 0 ||
        runtime_strcmp(uname, "SIN") == 0 || runtime_strcmp(uname, "COS") == 0 ||
        runtime_strcmp(uname, "TAN") == 0 || runtime_strcmp(uname, "ATN") == 0 ||
        runtime_strcmp(uname, "LOG") == 0 || runtime_strcmp(uname, "EXP") == 0 ||
        runtime_strcmp(uname, "INT") == 0 || runtime_strcmp(uname, "SGN") == 0 ||
        runtime_strcmp(uname, "FIX") == 0 || runtime_strcmp(uname, "DET") == 0 ||
        runtime_strcmp(uname, "DOT") == 0 || runtime_strcmp(uname, "CROSS") == 0 ||
        runtime_strcmp(uname, "EPS") == 0 || runtime_strcmp(uname, "_EPS") == 0 ||
        runtime_strcmp(uname, "INF") == 0 || runtime_strcmp(uname, "_INF") == 0 ||
        runtime_strcmp(uname, "MAXNUM") == 0 || runtime_strcmp(uname, "_MAXNUM") == 0 ||
        runtime_strcmp(uname, "DEGREES") == 0 || runtime_strcmp(uname, "_DEGREES") == 0 ||
        runtime_strcmp(uname, "RADIANS") == 0 || runtime_strcmp(uname, "_RADIANS") == 0 ||
        runtime_strcmp(uname, "REMAINDER") == 0 || runtime_strcmp(uname, "_REMAINDER") == 0 ||
        runtime_strcmp(uname, "ANGLE") == 0 || runtime_strcmp(uname, "_ANGLE") == 0 ||
        runtime_strcmp(uname, "TRUNCATE") == 0 || runtime_strcmp(uname, "_TRUNCATE") == 0 ||
        runtime_strcmp(uname, "TRUNC") == 0 || runtime_strcmp(uname, "_TRUNC") == 0 ||
        runtime_strcmp(uname, "CEIL") == 0 || runtime_strcmp(uname, "_CEIL") == 0 ||
        runtime_strcmp(uname, "FLOOR") == 0 || runtime_strcmp(uname, "_FLOOR") == 0 ||
        runtime_strcmp(uname, "ROUND") == 0 || runtime_strcmp(uname, "_ROUND") == 0 ||
        runtime_strcmp(uname, "CLAMP") == 0 || runtime_strcmp(uname, "_CLAMP") == 0 ||
        runtime_strcmp(uname, "LERP") == 0 || runtime_strcmp(uname, "_LERP") == 0 ||
        runtime_strcmp(uname, "ACOS") == 0 || runtime_strcmp(uname, "_ACOS") == 0 ||
        runtime_strcmp(uname, "ASIN") == 0 || runtime_strcmp(uname, "_ASIN") == 0 ||
        runtime_strcmp(uname, "ATAN2") == 0 || runtime_strcmp(uname, "_ATAN2") == 0 ||
        runtime_strcmp(uname, "SHL") == 0 || runtime_strcmp(uname, "_SHL") == 0 ||
        runtime_strcmp(uname, "SHR") == 0 || runtime_strcmp(uname, "_SHR") == 0 ||
        runtime_strcmp(uname, "MIN") == 0 || runtime_strcmp(uname, "MAX") == 0 ||
        runtime_strcmp(uname, "HYPOT") == 0 || runtime_strcmp(uname, "MOD") == 0 ||
        runtime_strcmp(uname, "AND") == 0 || runtime_strcmp(uname, "OR") == 0 ||
        runtime_strcmp(uname, "XOR") == 0 || runtime_strcmp(uname, "IMP") == 0 ||
        runtime_strcmp(uname, "EQV") == 0 || runtime_strcmp(uname, "NOT") == 0 ||
        runtime_strcmp(uname, "READBIT") == 0 || runtime_strcmp(uname, "_READBIT") == 0 ||
        runtime_strcmp(uname, "SETBIT") == 0 || runtime_strcmp(uname, "_SETBIT") == 0 ||
        runtime_strcmp(uname, "RESETBIT") == 0 || runtime_strcmp(uname, "_RESETBIT") == 0 ||
        runtime_strcmp(uname, "TOGGLEBIT") == 0 || runtime_strcmp(uname, "_TOGGLEBIT") == 0 ||
        runtime_strcmp(uname, "BITCOUNT") == 0 || runtime_strcmp(uname, "_BITCOUNT") == 0 ||
        runtime_strcmp(uname, "RND") == 0) return true;

    // String & Conversion Built-ins
    if (runtime_strcmp(uname, "LEN") == 0 || runtime_strcmp(uname, "ASC") == 0 ||
        runtime_strcmp(uname, "CHR$") == 0 || runtime_strcmp(uname, "CINT") == 0 ||
        runtime_strcmp(uname, "CSNG") == 0 || runtime_strcmp(uname, "CDBL") == 0 ||
        runtime_strcmp(uname, "VAL") == 0 || runtime_strcmp(uname, "STR$") == 0 ||
        runtime_strcmp(uname, "LEFT$") == 0 || runtime_strcmp(uname, "RIGHT$") == 0 ||
        runtime_strcmp(uname, "MID$") == 0 || runtime_strcmp(uname, "INSTR") == 0 ||
        runtime_strcmp(uname, "UCASE$") == 0 || runtime_strcmp(uname, "LCASE$") == 0 ||
        runtime_strcmp(uname, "LTRIM$") == 0 || runtime_strcmp(uname, "RTRIM$") == 0 ||
        runtime_strcmp(uname, "TRIM$") == 0 || runtime_strcmp(uname, "SPACE$") == 0 ||
        runtime_strcmp(uname, "STRING$") == 0 || runtime_strcmp(uname, "REPLACE$") == 0 ||
        runtime_strcmp(uname, "HEX$") == 0 || runtime_strcmp(uname, "OCT$") == 0 ||
        runtime_strcmp(uname, "BIN$") == 0 || runtime_strcmp(uname, "SHUFFLE$") == 0 ||
        runtime_strcmp(uname, "SHUFFLE") == 0 || runtime_strcmp(uname, "EDIT$") == 0 ||
        runtime_strcmp(uname, "NUM$") == 0 || runtime_strcmp(uname, "TCASE$") == 0 ||
        runtime_strcmp(uname, "ICASE$") == 0 || runtime_strcmp(uname, "REVERSE$") == 0 ||
        runtime_strcmp(uname, "REMOVE$") == 0 || runtime_strcmp(uname, "REMOVE") == 0 ||
        runtime_strcmp(uname, "HASH") == 0 || runtime_strcmp(uname, "SEEK") == 0 ||
        runtime_strcmp(uname, "HELP") == 0 || runtime_strcmp(uname, "HELP$") == 0 ||
        runtime_strcmp(uname, "CATEGORY") == 0 || runtime_strcmp(uname, "CATEGORY$") == 0 ||
        runtime_strcmp(uname, "CATEGORIES") == 0 || runtime_strcmp(uname, "CATEGORIES$") == 0 ||
        runtime_strcmp(uname, "INPUT$") == 0 || runtime_strcmp(uname, "SCREEN") == 0 ||
        runtime_strcmp(uname, "IOCTL$") == 0 || runtime_strcmp(uname, "TXNSTATUS") == 0 ||
        runtime_strcmp(uname, "FILEATTR") == 0 || runtime_strcmp(uname, "MKI$") == 0 ||
        runtime_strcmp(uname, "MKS$") == 0 || runtime_strcmp(uname, "MKD$") == 0 ||
        runtime_strcmp(uname, "MKSMBF$") == 0 || runtime_strcmp(uname, "MKDMBF$") == 0 ||
        runtime_strcmp(uname, "CVI") == 0 || runtime_strcmp(uname, "CVS") == 0 ||
        runtime_strcmp(uname, "CVD") == 0 || runtime_strcmp(uname, "CVSMBF") == 0 ||
        runtime_strcmp(uname, "CVDMBF") == 0 || runtime_strcmp(uname, "PV") == 0 ||
        runtime_strcmp(uname, "FV") == 0 || runtime_strcmp(uname, "PMT") == 0 ||
        runtime_strcmp(uname, "IPMT") == 0 || runtime_strcmp(uname, "PPMT") == 0 ||
        runtime_strcmp(uname, "NPER") == 0 || runtime_strcmp(uname, "RATE") == 0 ||
        runtime_strcmp(uname, "NPV") == 0 || runtime_strcmp(uname, "IRR") == 0 ||
        runtime_strcmp(uname, "FILEOPENBOX$") == 0 || runtime_strcmp(uname, "FILESAVEBOX$") == 0 ||
        runtime_strcmp(uname, "DATESERIAL") == 0 || runtime_strcmp(uname, "TIMESERIAL") == 0 ||
        runtime_strcmp(uname, "DATEVALUE") == 0 || runtime_strcmp(uname, "TIMEVALUE") == 0 ||
        runtime_strcmp(uname, "WEEKDAY") == 0 || runtime_strcmp(uname, "HOUR") == 0 ||
        runtime_strcmp(uname, "MINUTE") == 0 || runtime_strcmp(uname, "SECOND") == 0 ||
        runtime_strcmp(uname, "UNIXTIME") == 0 || runtime_strcmp(uname, "EPOCHDATE") == 0 ||
        runtime_strcmp(uname, "DIR$") == 0 || runtime_strcmp(uname, "DIR") == 0 ||
        runtime_strcmp(uname, "CURDIR$") == 0 || runtime_strcmp(uname, "CURDIR") == 0 ||
        runtime_strcmp(uname, "PREFIX$") == 0 || runtime_strcmp(uname, "PREFIX") == 0 ||
        runtime_strcmp(uname, "MODDIR$") == 0 || runtime_strcmp(uname, "MODDIR") == 0 ||
        runtime_strcmp(uname, "SETMEM") == 0 || runtime_strcmp(uname, "SSEG") == 0 ||
        runtime_strcmp(uname, "CVT$$") == 0 || runtime_strcmp(uname, "_CVT$$") == 0 ||
        runtime_strcmp(uname, "CVT$%") == 0 || runtime_strcmp(uname, "CVT%$") == 0 ||
        runtime_strcmp(uname, "CVT$F") == 0 || runtime_strcmp(uname, "CVTF$") == 0 ||
        runtime_strcmp(uname, "SWAP%") == 0 || runtime_strcmp(uname, "TYP") == 0 ||
        runtime_strcmp(uname, "_TYP") == 0 || runtime_strcmp(uname, "RECOUNT") == 0 ||
        runtime_strcmp(uname, "_RECOUNT") == 0 || runtime_strcmp(uname, "STATUS") == 0 ||
        runtime_strcmp(uname, "_STATUS") == 0 || runtime_strcmp(uname, "DEVICECOUNT") == 0 ||
        runtime_strcmp(uname, "DEVICE$") == 0 || runtime_strcmp(uname, "DEVICECLASS$") == 0 ||
        runtime_strcmp(uname, "DEVICEINFO$") == 0 || runtime_strcmp(uname, "POLL") == 0) return true;

    // SIO / BIO / Data Structures / Metaprogramming
    if (runtime_strcmp(uname, "SIOREAD$") == 0 || runtime_strcmp(uname, "SIOREADLN$") == 0 ||
        runtime_strcmp(uname, "SIOWRITE") == 0 || runtime_strcmp(uname, "SIOSEEK") == 0 ||
        runtime_strcmp(uname, "SIOFLUSH") == 0 || runtime_strcmp(uname, "SIOSTATUS") == 0 ||
        runtime_strcmp(uname, "SIOAVAIL") == 0 || runtime_strcmp(uname, "BIOREAD$") == 0 ||
        runtime_strcmp(uname, "BIOWRITE") == 0 || runtime_strcmp(uname, "BIOCOPY") == 0 ||
        runtime_strcmp(uname, "BIOFILL") == 0 || runtime_strcmp(uname, "BIOSTATUS") == 0 ||
        runtime_strcmp(uname, "BIOSIZE") == 0 || runtime_strcmp(uname, "BIOCHECKSUM") == 0 ||
        runtime_strcmp(uname, "BIOCOMPARE") == 0 || runtime_strcmp(uname, "PEEK") == 0 ||
        runtime_strcmp(uname, "MAP") == 0 || runtime_strcmp(uname, "MAP_NEW") == 0 ||
        runtime_strcmp(uname, "MAP_SET") == 0 || runtime_strcmp(uname, "MAP_GET") == 0 ||
        runtime_strcmp(uname, "MAP_GET$") == 0 || runtime_strcmp(uname, "MAP_REMOVE") == 0 ||
        runtime_strcmp(uname, "MAP_COUNT") == 0 || runtime_strcmp(uname, "MAP_KEY$") == 0 ||
        runtime_strcmp(uname, "MAP_HAS") == 0 || runtime_strcmp(uname, "JSON_PARSE") == 0 ||
        runtime_strcmp(uname, "JSON_STRINGIFY$") == 0 || runtime_strcmp(uname, "XML_PARSE") == 0 ||
        runtime_strcmp(uname, "XML_STRINGIFY$") == 0 || runtime_strcmp(uname, "YAML_PARSE") == 0 ||
        runtime_strcmp(uname, "YAML_STRINGIFY$") == 0 || runtime_strcmp(uname, "INI_PARSE") == 0 ||
        runtime_strcmp(uname, "INI_STRINGIFY$") == 0 || runtime_strcmp(uname, "DCOUNT") == 0 ||
        runtime_strcmp(uname, "FIELD") == 0 || runtime_strcmp(uname, "EXTRACT") == 0 ||
        runtime_strcmp(uname, "COUNT") == 0 || runtime_strcmp(uname, "INS") == 0 ||
        runtime_strcmp(uname, "DEL") == 0 || runtime_strcmp(uname, "REPLACE") == 0 ||
        runtime_strcmp(uname, "FID") == 0 || runtime_strcmp(uname, "FID$") == 0 ||
        runtime_strcmp(uname, "FIN") == 0 || runtime_strcmp(uname, "HTA") == 0 ||
        runtime_strcmp(uname, "HTA$") == 0 || runtime_strcmp(uname, "ATH") == 0 ||
        runtime_strcmp(uname, "ATH$") == 0 || runtime_strcmp(uname, "HEXIN") == 0 ||
        runtime_strcmp(uname, "HEXIN$") == 0 || runtime_strcmp(uname, "HEXOUT") == 0 ||
        runtime_strcmp(uname, "HEXOUT$") == 0 || runtime_strcmp(uname, "TEK") == 0 ||
        runtime_strcmp(uname, "TEK$") == 0 || runtime_strcmp(uname, "VEC") == 0 ||
        runtime_strcmp(uname, "VEC$") == 0 || runtime_strcmp(uname, "FPT") == 0 ||
        runtime_strcmp(uname, "IPT") == 0 || runtime_strcmp(uname, "KEYIN") == 0 ||
        runtime_strcmp(uname, "KEYIN$") == 0 || runtime_strcmp(uname, "MSGBOX") == 0 ||
        runtime_strcmp(uname, "MSGBOX$") == 0 || runtime_strcmp(uname, "INPUTBOX") == 0 ||
        runtime_strcmp(uname, "INPUTBOX$") == 0 || runtime_strcmp(uname, "ENVIRON$") == 0 ||
        runtime_strcmp(uname, "GETATTR") == 0 || runtime_strcmp(uname, "INP") == 0 ||
        runtime_strcmp(uname, "MKL$") == 0 || runtime_strcmp(uname, "CVL") == 0 ||
        runtime_strcmp(uname, "MKC$") == 0 || runtime_strcmp(uname, "CVC") == 0 ||
        runtime_strcmp(uname, "CMPLX") == 0 || runtime_strcmp(uname, "REAL") == 0 ||
        runtime_strcmp(uname, "IMAG") == 0 || runtime_strcmp(uname, "CONJ") == 0 ||
        runtime_strcmp(uname, "ARG") == 0 || runtime_strcmp(uname, "CMPLX$") == 0 ||
        runtime_strcmp(uname, "CSQR") == 0 || runtime_strcmp(uname, "CLOG") == 0 ||
        runtime_strcmp(uname, "CEXP") == 0 || runtime_strcmp(uname, "CSIN") == 0 ||
        runtime_strcmp(uname, "CCOS") == 0 || runtime_strcmp(uname, "CTAN") == 0 ||
        runtime_strcmp(uname, "CASN") == 0 || runtime_strcmp(uname, "CACS") == 0 ||
        runtime_strcmp(uname, "CATN") == 0 || runtime_strcmp(uname, "ASN") == 0 ||
        runtime_strcmp(uname, "ACS") == 0 || runtime_strcmp(uname, "HSN") == 0 ||
        runtime_strcmp(uname, "HCS") == 0 || runtime_strcmp(uname, "HTN") == 0 ||
        runtime_strcmp(uname, "EXN") == 0 || runtime_strcmp(uname, "LGT") == 0 ||
        runtime_strcmp(uname, "LOG10") == 0 || runtime_strcmp(uname, "_LOG10") == 0 ||
        runtime_strcmp(uname, "MATH.LOG10") == 0 || runtime_strcmp(uname, "LOG2") == 0 ||
        runtime_strcmp(uname, "_LOG2") == 0 || runtime_strcmp(uname, "MATH.LOG2") == 0 ||
        runtime_strcmp(uname, "SINH") == 0 || runtime_strcmp(uname, "_SINH") == 0 ||
        runtime_strcmp(uname, "MATH.SINH") == 0 || runtime_strcmp(uname, "COSH") == 0 ||
        runtime_strcmp(uname, "_COSH") == 0 || runtime_strcmp(uname, "MATH.COSH") == 0 ||
        runtime_strcmp(uname, "TANH") == 0 || runtime_strcmp(uname, "_TANH") == 0 ||
        runtime_strcmp(uname, "MATH.TANH") == 0 || runtime_strcmp(uname, "COT") == 0 ||
        runtime_strcmp(uname, "_COT") == 0 || runtime_strcmp(uname, "MATH.COT") == 0 ||
        runtime_strcmp(uname, "SEC") == 0 || runtime_strcmp(uname, "_SEC") == 0 ||
        runtime_strcmp(uname, "MATH.SEC") == 0 || runtime_strcmp(uname, "CSC") == 0 ||
        runtime_strcmp(uname, "_CSC") == 0 || runtime_strcmp(uname, "MATH.CSC") == 0 ||
        runtime_strcmp(uname, "RAD") == 0 || runtime_strcmp(uname, "DEG") == 0 ||
        runtime_strcmp(uname, "COMP") == 0 || runtime_strcmp(uname, "_COMP") == 0 ||
        runtime_strcmp(uname, "MATH.COMP") == 0 || runtime_strcmp(uname, "PDIF") == 0 ||
        runtime_strcmp(uname, "_PDIF") == 0 || runtime_strcmp(uname, "MATH.PDIF") == 0 ||
        runtime_strcmp(uname, "RAD$") == 0 || runtime_strcmp(uname, "NUM1$") == 0 ||
        runtime_strcmp(uname, "NUM2$") == 0 || runtime_strcmp(uname, "SEG$") == 0 ||
        runtime_strcmp(uname, "ERT$") == 0 || runtime_strcmp(uname, "UPS$") == 0 ||
        runtime_strcmp(uname, "SUM$") == 0 || runtime_strcmp(uname, "DIF$") == 0 ||
        runtime_strcmp(uname, "PROD$") == 0 || runtime_strcmp(uname, "QUO$") == 0 ||
        runtime_strcmp(uname, "PLACE$") == 0 || runtime_strcmp(uname, "SSEGADD") == 0 ||
        runtime_strcmp(uname, "SWAP") == 0 || runtime_strcmp(uname, "VAL%") == 0 ||
        runtime_strcmp(uname, "XLATE$") == 0 || runtime_strcmp(uname, "ASCII") == 0 ||
        runtime_strcmp(uname, "NUM") == 0 || runtime_strcmp(uname, "INDEX") == 0 ||
        runtime_strcmp(uname, "SPEC%") == 0 || runtime_strcmp(uname, "MAGTAPE") == 0 ||
        runtime_strcmp(uname, "MAG") == 0 || runtime_strcmp(uname, "SYS") == 0 ||
        runtime_strcmp(uname, "COMPLEX") == 0 || runtime_strcmp(uname, "RE") == 0 ||
        runtime_strcmp(uname, "IM") == 0 || runtime_strcmp(uname, "CONJG") == 0 ||
        runtime_strcmp(uname, "CABS") == 0 || runtime_strcmp(uname, "VERIFY") == 0) return true;

    // USR / Hardware / Platform Specific
    if (runtime_strncmp(uname, "USR", 3) == 0) return true;
    if (runtime_strcmp(uname, "ERDEV") == 0 || runtime_strcmp(uname, "ERDEV$") == 0 ||
        runtime_strcmp(uname, "EXTERR") == 0 || runtime_strcmp(uname, "HASH$") == 0 ||
        runtime_strcmp(uname, "SALT$") == 0 || runtime_strcmp(uname, "AUDITCRACK") == 0 ||
        runtime_strcmp(uname, "AUDITCRACK$") == 0 || runtime_strcmp(uname, "SANDBOXAUDIT") == 0 ||
        runtime_strcmp(uname, "VMCHECK") == 0 || runtime_strcmp(uname, "NETHOST$") == 0 ||
        runtime_strcmp(uname, "NETIP$") == 0) return true;

#ifndef BASIC_LITE_BUILD
    if (runtime_strcmp(uname, "VARPTR") == 0 || runtime_strcmp(uname, "VARPTR$") == 0 ||
        runtime_strcmp(uname, "VARSEG") == 0 || runtime_strcmp(uname, "SADD") == 0) return true;
#endif

    // IoT & Microcontroller Built-in Functions
    if (runtime_strcmp(uname, "DREAD") == 0 || runtime_strcmp(uname, "AREAD") == 0 ||
        runtime_strcmp(uname, "TOUCH") == 0 || runtime_strcmp(uname, "TOUCH.READ") == 0 ||
        runtime_strcmp(uname, "HALL") == 0 || runtime_strcmp(uname, "HALL.READ") == 0 ||
        runtime_strcmp(uname, "TICKS_MS") == 0 || runtime_strcmp(uname, "TICKS_US") == 0 ||
        runtime_strcmp(uname, "TICKS_DIFF") == 0 || runtime_strcmp(uname, "MEM.FREE") == 0 ||
        runtime_strcmp(uname, "MEM.ALLOC") == 0 || runtime_strcmp(uname, "HTTP.GET$") == 0 ||
        runtime_strcmp(uname, "HTTP.POST") == 0 || runtime_strcmp(uname, "GEMINI.GET$") == 0 ||
        runtime_strcmp(uname, "GEMINI.STATUS%") == 0 || runtime_strcmp(uname, "GEMINI.STATUS") == 0 ||
        runtime_strcmp(uname, "GEMINI.META$") == 0 || runtime_strcmp(uname, "GOPHER.GET$") == 0 ||
        runtime_strcmp(uname, "TNFS.DIR$") == 0 || runtime_strcmp(uname, "FUJI.STATUS$") == 0 ||
        runtime_strcmp(uname, "FUJI.SSID$") == 0 || runtime_strcmp(uname, "FUJI.IP$") == 0 ||
        runtime_strcmp(uname, "FUJI.JSON.GET$") == 0 || runtime_strcmp(uname, "PYTHON$") == 0 ||
        runtime_strcmp(uname, "COMSTR$") == 0 || runtime_strcmp(uname, "DECOMSTR$") == 0 ||
        runtime_strcmp(uname, "NIL.COMPRESS$") == 0 || runtime_strcmp(uname, "NIL.DECOMPRESS$") == 0 ||
        runtime_strcmp(uname, "NET.PACK$") == 0 || runtime_strcmp(uname, "NIL.PACK$") == 0 ||
        runtime_strcmp(uname, "REMOTE.EVAL$") == 0 || runtime_strcmp(uname, "IOT.RPC$") == 0 ||
        runtime_strcmp(uname, "SOCK.OPEN%") == 0 || runtime_strcmp(uname, "SOCK.OPEN") == 0 ||
        runtime_strcmp(uname, "SOCK.ACCEPT%") == 0 || runtime_strcmp(uname, "SOCK.ACCEPT") == 0 ||
        runtime_strcmp(uname, "SOCK.RECV$") == 0 || runtime_strcmp(uname, "SOCK.POLL%") == 0 ||
        runtime_strcmp(uname, "SOCK.POLL") == 0 || runtime_strcmp(uname, "SOCK.STATUS%") == 0 ||
        runtime_strcmp(uname, "SOCK.STATUS") == 0 || runtime_strcmp(uname, "PACKET.MAC$") == 0 ||
        runtime_strcmp(uname, "PACKET.RSSI%") == 0 || runtime_strcmp(uname, "PACKET.RSSI") == 0 ||
        runtime_strcmp(uname, "PACKET.PAYLOAD$") == 0 || runtime_strcmp(uname, "PACKET.LEN%") == 0 ||
        runtime_strcmp(uname, "PACKET.LEN") == 0 || runtime_strcmp(uname, "PACKET.SRC$") == 0 ||
        runtime_strcmp(uname, "PACKET.PORT%") == 0 || runtime_strcmp(uname, "PACKET.PORT") == 0 ||
        runtime_strcmp(uname, "PACKET.TYPE%") == 0 || runtime_strcmp(uname, "PACKET.TYPE") == 0 ||
        runtime_strcmp(uname, "CRYPTO.ENCRYPT$") == 0 || runtime_strcmp(uname, "CRYPTO.DECRYPT$") == 0 ||
        runtime_strcmp(uname, "CRYPTO.HASH$") == 0 || runtime_strcmp(uname, "CRYPTO.HMAC$") == 0 ||
        runtime_strcmp(uname, "CRYPTO.KEY$") == 0 ||
        runtime_strcmp(uname, "PYTHON") == 0) return true;

    if (funcreg_find_by_name(uname) != NULL) {
        return true;
    }

    return false;
}

