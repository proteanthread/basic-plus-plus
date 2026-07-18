/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: codegen.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Stored program editor commands (RENUM, DELETE), compiler driver pipeline (BASIC-to-C), and bytecode serializers.
 *
 * 2. WHAT TO EXPECT:
 *    Code generator constructs self-contained C89 files. Bytecode serializes code to files.
 *
 * 3. WHAT CAN BE CHANGED:
 *    C89 codegen shims, editor warnings, target language mapping layout.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    AST translation loops, bytecode file format specs.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Verify target C compiler settings. If transpiled C file has compilation warnings, check codegen expressions mapping.
 * ===================================================================== */

 // ---
 // BASIC++ Compiler - codegen.c
 // ---
 //
 // C89 code generator.
 //
 // DESIGN RATIONALE:
 // Walks the AST and emits a complete, self-contained ANSI C89
 // program. The generated code includes:
 //
 // 1. A runtime shim - static helper functions for PRINT, INPUT,
 // string operations, GOSUB stack, and DIM arrays.
 //
 // 2. Variable declarations - A-Z integer/float, A$-Z$ strings,
 // @() legacy array, DIM arrays.
 //
 // 3. Program body - each BASIC line becomes a labeled block
 // (L10:, L20:, etc.) using C goto for flow control.
 //
 // The output compiles with any C89 compiler and runs standalone.
 //
//
// HOW TO EXTEND:
//   Adding support for a new statement in code generation:
//   1. Add the AST node type in ast.h.
//   2. Add the emit case in this file's switch statement.
//   3. Generate the corresponding C code output.
 // ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "codegen.h"
#include "ast.h"
#include "errors.h"
#include "lexer.h"
#include "../console.h"

// --- Internal helpers ---

static char num_vars[1024][64];
static int num_vars_count = 26;

static int get_numvar_index(const char *name) {
    char clean_name[64];
    int len = 0;
    // convert name to uppercase and strip trailing % or ! or #
    while (name[len] && name[len] != '%' && name[len] != '!' && name[len] != '#') {
        if (len < 63) {
            clean_name[len] = (char)toupper((unsigned char)name[len]);
            len++;
        } else {
            break;
        }
    }
    clean_name[len] = '\0';

    if (len == 0) return 0;
    if (len == 1) {
        if (clean_name[0] >= 'A' && clean_name[0] <= 'Z') {
            return clean_name[0] - 'A';
        }
    }

    // Search in registered named vars
    for (int i = 26; i < num_vars_count; i++) {
        if (strcmp(num_vars[i], clean_name) == 0) {
            return i;
        }
    }

    // Register new named var
    if (num_vars_count < 1024) {
        strcpy(num_vars[num_vars_count], clean_name);
        return num_vars_count++;
    }
    return 0; // fallback
}

static char str_vars[1024][64];
static int str_vars_count = 26;

static int get_strvar_index(const char *name) {
    char clean_name[64];
    int len = 0;
    // convert name to uppercase and strip trailing $
    while (name[len] && name[len] != '$') {
        if (len < 63) {
            clean_name[len] = (char)toupper((unsigned char)name[len]);
            len++;
        } else {
            break;
        }
    }
    clean_name[len] = '\0';

    if (len == 0) return 0;
    if (len == 1) {
        if (clean_name[0] >= 'A' && clean_name[0] <= 'Z') {
            return clean_name[0] - 'A';
        }
    }

    // Search in registered named vars
    for (int i = 26; i < str_vars_count; i++) {
        if (strcmp(str_vars[i], clean_name) == 0) {
            return i;
        }
    }

    // Register new named var
    if (str_vars_count < 1024) {
        strcpy(str_vars[str_vars_count], clean_name);
        return str_vars_count++;
    }
    return 0; // fallback
}

static void emit_runtime_shim(FILE *out, const TargetConfig *target)
{
    fprintf(out,
        "#ifdef _MSC_VER\n"
        "#ifndef _CRT_SECURE_NO_WARNINGS\n"
        "#define _CRT_SECURE_NO_WARNINGS\n"
        "#endif\n"
        "#pragma warning(disable: 4996)\n"
        "#pragma warning(disable: 4101)\n"
        "#pragma warning(disable: 4244)\n"
        "#pragma warning(disable: 4102)\n"
        "#pragma warning(disable: 4311)\n"
        "#pragma warning(disable: 4312)\n"
        "#endif\n\n"
        "#ifdef __GNUC__\n"
        "#pragma GCC diagnostic ignored \"-Wunused-function\"\n"
        "#pragma GCC diagnostic ignored \"-Wunused-variable\"\n"
        "#pragma GCC diagnostic ignored \"-Wunused-label\"\n"
        "#pragma GCC diagnostic ignored \"-Wstringop-truncation\"\n"
        "#pragma GCC diagnostic ignored \"-Wrestrict\"\n"
        "#pragma GCC diagnostic ignored \"-Wdouble-promotion\"\n"
        "#pragma GCC diagnostic ignored \"-Wpointer-to-int-cast\"\n"
        "#pragma GCC diagnostic ignored \"-Wint-to-pointer-cast\"\n"
        "#endif\n\n"
);

    fprintf(out,
        "/* Generated by BASIC++ Compiler for target: %s */\n"
        "#define %s\n"
        "#define BPP_SUPPORT_MULTITASKING %d\n"
        "#define BPP_SUPPORT_GRAPHICS %d\n"
        "#define BPP_SUPPORT_FILESYSTEM %d\n"
        "\n",
        target->name,
        target->target_macro,
        target->has_multitasking ? 1 : 0,
        target->has_graphics ? 1 : 0,
        target->has_filesystem ? 1 : 0);

    fprintf(out,
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "#include <string.h>\n"
        "#include <math.h>\n"
        "\n");

    if (target->platform == TARGET_WINDOWS) {
        fprintf(out, "/* Windows specific platform headers */\n");
        fprintf(out, "#include <windows.h>\n\n");
    } else if (target->platform == TARGET_LINUX) {
        fprintf(out, "/* Linux specific platform headers */\n");
        fprintf(out, "#include <unistd.h>\n");
        fprintf(out, "#include <sys/types.h>\n\n");
    } else if (target->platform == TARGET_FREEDOS) {
        fprintf(out, "/* FreeDOS specific platform headers */\n");
        fprintf(out, "#include <dos.h>\n");
        fprintf(out, "#include <conio.h>\n\n");
    }

    fprintf(out,
        "/* === Runtime Variables === */\n"
        "static double bpp_vars[1024]; /* A-Z and named */\n"
        "static char bpp_strvars[1024][256]; /* A$-Z$ and named */\n"
        "static double bpp_at_array[4096]; /* @() legacy array */\n"
        "static int bpp_print_width = 14;\n"
        "\n"
        "/* === GOSUB Stack === */\n"
        "#define BPP_MAX_STACK 256\n"
        "static int bpp_gosub_stack[BPP_MAX_STACK];\n"
        "static int bpp_gosub_sp = 0;\n"
        "\n"
        "/* === DIM Arrays === */\n"
        "#define BPP_MAX_DIM 64\n"
        "#define BPP_MAX_ELEM %ld\n"
        "static double bpp_dim_pool[BPP_MAX_ELEM];\n"
        "static int bpp_dim_used = 0;\n"
        "typedef struct { char name[32]; int d1,d2,off; } BppDim;\n"
        "static BppDim bpp_dims[BPP_MAX_DIM];\n"
        "static int bpp_dim_count = 0;\n"
        "\n",
        target->default_array_limit);
    fprintf(out,
        "static void bpp_dim_alloc(const char *nm, int d1, int d2) {\n"
 " int sz = (d1+1) * (d2>0 ? d2+1 : 1);\n"
 " BppDim *d = &bpp_dims[bpp_dim_count++];\n"
 " strncpy(d->name, nm, 31); d->name[31]='\\0';\n"
 " d->d1=d1; d->d2=d2; d->off=bpp_dim_used;\n"
 " memset(&bpp_dim_pool[bpp_dim_used], 0,"
 " sizeof(double)*(size_t)sz);\n"
 " bpp_dim_used += sz;\n"
 "}\n"
 "\n"
 "static double *bpp_dim_ref(const char *nm, int i1, int i2) {\n"
 " int k;\n"
 " for (k=0; k<bpp_dim_count; k++) {\n"
 " if (strcmp(bpp_dims[k].name, nm)==0) {\n"
 " int idx = (bpp_dims[k].d2>0)\n"
 " ? i1*(bpp_dims[k].d2+1)+i2 : i1;\n"
 " return &bpp_dim_pool[bpp_dims[k].off + idx];\n"
 " }\n"
 " }\n"
 " fprintf(stderr, \"Array not found: %%s\\n\", nm);\n"
 " exit(1);\n"
 " return NULL;\n"
 "}\n"
 "\n"
 "/* === String DIM arrays === */\n"
 "#define BPP_MAX_STRDIM 26\n"
 "#define BPP_MAX_STRDIM_SIZE 256\n"
 "static struct { char nm[16]; int d1; int d2;\n"
 " char data[BPP_MAX_STRDIM_SIZE][256]; } bpp_strdims[BPP_MAX_STRDIM];\n"
 "static int bpp_strdim_count = 0;\n"
 "\n"
 "static char *bpp_strdim_ref(const char *nm, int i1, int i2) {\n"
 " int k;\n"
 " (void)i2;\n"
 " for (k = 0; k < bpp_strdim_count; k++) {\n"
 " if (strcmp(bpp_strdims[k].nm, nm) == 0) {\n"
 " if (i1 < 0 || i1 >= BPP_MAX_STRDIM_SIZE) i1 = 0;\n"
 " return bpp_strdims[k].data[i1];\n"
 " }\n"
 " }\n"
 " /* Auto-create */\n"
 " if (bpp_strdim_count < BPP_MAX_STRDIM) {\n"
 " int j;\n"
 " strncpy(bpp_strdims[bpp_strdim_count].nm, nm, 15);\n"
 " bpp_strdims[bpp_strdim_count].nm[15] = '\\0';\n"
 " for (j = 0; j < BPP_MAX_STRDIM_SIZE; j++)\n"
 " bpp_strdims[bpp_strdim_count].data[j][0] = '\\0';\n"
 " bpp_strdim_count++;\n"
 " if (i1 < 0 || i1 >= BPP_MAX_STRDIM_SIZE) i1 = 0;\n"
 " return bpp_strdims[bpp_strdim_count-1].data[i1];\n"
 " }\n"
 " fprintf(stderr, \"String array not found: %%s\\n\", nm);\n"
 " exit(1);\n"
 " return NULL;\n"
 "}\n"
 "\n"
 "/* === DATA/READ support === */\n"
 "#define BPP_MAX_DATA 4096\n"
 "static const char *bpp_data_pool[BPP_MAX_DATA];\n"
 "static int bpp_data_count = 0;\n"
 "static int bpp_data_ptr = 0;\n"
 "\n"
 "static double bpp_read_num(void) {\n"
 "  if (bpp_data_ptr >= bpp_data_count) {\n"
 "    fprintf(stderr, \"OUT OF DATA\\n\"); exit(1);\n"
 "  }\n"
 "  return atof(bpp_data_pool[bpp_data_ptr++]);\n"
 "}\n"
 "\n"
 "static const char *bpp_read_str(void) {\n"
 "  if (bpp_data_ptr >= bpp_data_count) {\n"
 "    fprintf(stderr, \"OUT OF DATA\\n\"); exit(1);\n"
 "  }\n"
 "  return bpp_data_pool[bpp_data_ptr++];\n"
 "}\n"
 "\n"
 "/* === String helpers === */\n"
 "static char bpp_strtmp[8][256];\n"
 "static int bpp_strtmp_idx = 0;\n"
 "\n"
 "static char *bpp_tmp(void) {\n"
 " char *p = bpp_strtmp[bpp_strtmp_idx];\n"
 " bpp_strtmp_idx = (bpp_strtmp_idx+1) & 7;\n"
 " return p;\n"
 "}\n"
 "\n"
 "static const char *bpp_concat(const char *a, const char *b) {\n"
 " char *t = bpp_tmp();\n"
 " t[0]='\\0';\n"
 " strncat(t, a, 254);\n"
 " strncat(t, b, 254-(int)strlen(t));\n"
 " return t;\n"
 "}\n"
 "\n"
 "static const char *bpp_left(const char *s, int n) {\n"
 " char *t = bpp_tmp();\n"
 " int len = (int)strlen(s);\n"
 " if (n>len) n=len;\n"
 " if (n<0) n=0;\n"
 " memcpy(t, s, (size_t)n); t[n]='\\0';\n"
 " return t;\n"
 "}\n"
 "\n"
 "static const char *bpp_right(const char *s, int n) {\n"
 " char *t = bpp_tmp();\n"
 " int len = (int)strlen(s);\n"
 " if (n>len) n=len;\n"
 " if (n<0) n=0;\n"
 " memcpy(t, s+len-n, (size_t)n); t[n]='\\0';\n"
 " return t;\n"
 "}\n"
 "\n"
 "static const char *bpp_mid(const char *s, int st, int n) {\n"
 " char *t = bpp_tmp();\n"
 " int len = (int)strlen(s);\n"
 " if (st<0) st=0;\n"
 " if (st>len) st=len;\n"
 " if (n<0) n=0;\n"
 " if (st+n>len) n=len-st;\n"
 " memcpy(t, s+st, (size_t)n); t[n]='\\0';\n"
 " return t;\n"
 "}\n"
 "\n"
 "static const char *bpp_chr(int n) {\n"
 " char *t = bpp_tmp();\n"
 " t[0]=(char)n; t[1]='\\0';\n"
 " return t;\n"
 "}\n"
 "\n"
 "static const char *bpp_str(double n) {\n"
 " char *t = bpp_tmp();\n"
 " sprintf(t, \"%%g\", n);\n"
 " return t;\n"
 "}\n"
 "\n"
 "/* === TAB helper === */\n"
 "static const char *bpp_tab(int col) {\n"
 " static char buf[256];\n"
 " int i;\n"
 " if (col < 0) col = 0;\n"
 " if (col > 255) col = 255;\n"
 " for (i = 0; i < col; i++) buf[i] = ' ';\n"
 " buf[col] = '\\0';\n"
 " return buf;\n"
 "}\n"
 "\n"
 "/* === DEF FN support === */\n"
 "#define BPP_MAX_FN 26\n"
 "typedef double (*BppFnPtr)(double);\n"
 "static double bpp_fn_param;\n"
 "\n"
 "/* === Debugger trace and state variables === */\n"
 "static int bpp_tron = 0;\n"
 "static void bpp_trace_statement(double ln, const char *file) {\n"
 "    if (bpp_tron) {\n"
 "        printf(\"[Line %%g] in %%s\\n\", ln, file);\n"
 "    }\n"
 "}\n"
 "static void bpp_assert_fail(const char *expr, const char *msg, double ln, const char *file) {\n"
 "    fprintf(stderr, \"BPP_ASSERT failed: (%%s) at line %%g in %%s\", expr, ln, file);\n"
 "    if (msg && msg[0]) {\n"
 "        fprintf(stderr, \" - %%s\", msg);\n"
 "    }\n"
 "    fprintf(stderr, \"\\nStack Trace:\\n  at line %%g in %%s\\n\", ln, file);\n"
 "    exit(99);\n"
 "}\n"
 "#define BPP_ASSERT(cond, expr, msg, ln, file) do { if (!(cond)) bpp_assert_fail(expr, msg, ln, file); } while(0)\n"
 "static void bpp_breakpoint(double ln, const char *file) {\n"
 "    printf(\"[BREAKPOINT] hit at line %%g in %%s\\n\", ln, file);\n"
 "    printf(\"Press ENTER to continue...\\n\");\n"
 "    getchar();\n"
 "}\n"
 "static void bpp_dump_vars(void) {\n"
 "    int i;\n"
 "    printf(\"--- Active Variables (Transpiled) ---\\n\");\n"
 "    for (i = 0; i < 26; i++) {\n"
 "        if (bpp_vars[i] != 0.0) {\n"
 "            printf(\"  %%c = %%g\\n\", 'A' + i, bpp_vars[i]);\n"
 "        }\n"
 "        if (bpp_strvars[i][0] != '\\0') {\n"
 "            printf(\"  %%c$ = \\\"%%s\\\"\\n\", 'A' + i, bpp_strvars[i]);\n"
 "        }\n"
 "    }\n"
 "}\n"
 "\n"
 );
}

 // expr_is_string - Check if an expression produces a string value.
static int expr_is_string(AstExpr *e)
{
 if (!e) return 0;
 if (e->type == EXPR_STRING_LIT) return 1;
 if (e->type == EXPR_STRING_VAR) return 1;
 if (e->type == EXPR_DIM_ACCESS) {
 int nlen = (int)strlen(e->v.dim_access.name);
 if (nlen > 0 && e->v.dim_access.name[nlen-1] == '$')
 return 1;
 }
   if (e->type == EXPR_FUNC_CALL) {
       switch (e->v.func_call.func) {
           case FUNC_CHR: case FUNC_STR:
           case FUNC_LEFT: case FUNC_RIGHT: case FUNC_MID:
           case FUNC_TAB:
           case FUNC_LCASES: case FUNC_UCASES: case FUNC_TCASES:
           case FUNC_TRIMS: case FUNC_LTRIMS: case FUNC_RTRIMS:
           case FUNC_REPLACES: case FUNC_REVERSES:
           case FUNC_MCASES: case FUNC_ICASES:
               return 1;
           default:
               return 0;
       }
   }
 if (e->type == EXPR_BINOP && e->v.binop.op == BOP_ADD) {
 return expr_is_string(e->v.binop.left) ||
 expr_is_string(e->v.binop.right);
 }
 if (e->type == EXPR_BINOP && e->v.binop.op == BOP_CONCAT)
 return 1;
 if (e->type == EXPR_NAMED_VAR) {
 int nlen = e->v.named.name_len;
 if (nlen > 0 && e->v.named.name[nlen - 1] == '$')
 return 1;
 }
 return 0;
}

 // emit_expr - Recursively emit a C expression from an AST node.
static void emit_expr(FILE *out, AstExpr *e)
{
 if (!e) { fprintf(out, "0"); return; }

 switch (e->type) {
 case EXPR_INT_LIT:
 fprintf(out, "%ld", e->v.ival);
 break;

 case EXPR_FLOAT_LIT:
 fprintf(out, "%.17g", e->v.fval);
 break;

 case EXPR_STRING_LIT:
 {
 int i;
 fprintf(out, "\"");
 for (i = 0; i < e->v.sval.length; i++) {
 char c = e->v.sval.data[i];
 if (c == '"') fprintf(out, "\\\"");
 else if (c == '\\') fprintf(out, "\\\\");
 else if (c == '\n') fprintf(out, "\\n");
 else if (c == '\r') fprintf(out, "\\r");
 else if (c >= 32 && c < 127) fputc(c, out);
 else fprintf(out, "\\x%02x", (unsigned char)c);
 }
 fprintf(out, "\"");
 }
 break;

	case EXPR_VAR:
		{
			char nm[2] = { e->v.var_name, '\0' };
			fprintf(out, "bpp_vars[%d]", get_numvar_index(nm));
		}
		break;

	case EXPR_STRING_VAR:
		{
			char nm[2] = { e->v.var_name, '\0' };
			fprintf(out, "bpp_strvars[%d]", get_strvar_index(nm));
		}
		break;

	case EXPR_NAMED_VAR:
		{
			int nlen = e->v.named.name_len;
			if (nlen > 0 && e->v.named.name[nlen - 1] == '$') {
				fprintf(out, "bpp_strvars[%d]", get_strvar_index(e->v.named.name));
			} else {
				fprintf(out, "bpp_vars[%d]", get_numvar_index(e->v.named.name));
			}
		}
		break;

 case EXPR_ARRAY_AT:
 fprintf(out, "bpp_at_array[(int)(");
 emit_expr(out, e->v.array_at.index);
 fprintf(out, ")]");
 break;

 case EXPR_DIM_ACCESS:
 {
 int is_str = 0;
 int nlen = (int)strlen(e->v.dim_access.name);
 if (nlen > 0 && e->v.dim_access.name[nlen-1] == '$')
 is_str = 1;
 if (is_str) {
 char nm[2];
 nm[0] = e->v.dim_access.name[0];
 nm[1] = '\0';
 fprintf(out, "bpp_strdim_ref(\"%s\", (int)(", nm);
 } else {
 fprintf(out, "*bpp_dim_ref(\"%s\", (int)(",
 e->v.dim_access.name);
 }
 emit_expr(out, e->v.dim_access.idx1);
 fprintf(out, "), ");
 if (e->v.dim_access.idx2) {
 fprintf(out, "(int)(");
 emit_expr(out, e->v.dim_access.idx2);
 fprintf(out, ")");
 } else {
 fprintf(out, "0");
 }
 fprintf(out, ")");
 }
 break;

 case EXPR_BINOP:
 fprintf(out, "(");
 switch (e->v.binop.op) {
 case BOP_ADD:
 if (expr_is_string(e->v.binop.left) ||
 expr_is_string(e->v.binop.right)) {
 fprintf(out, "bpp_concat(");
 emit_expr(out, e->v.binop.left);
 fprintf(out, ", ");
 emit_expr(out, e->v.binop.right);
 fprintf(out, ")");
 } else {
 emit_expr(out, e->v.binop.left);
 fprintf(out, " + ");
 emit_expr(out, e->v.binop.right);
 }
 break;
 case BOP_SUB:
 emit_expr(out, e->v.binop.left);
 fprintf(out, " - ");
 emit_expr(out, e->v.binop.right);
 break;
 case BOP_MUL:
 emit_expr(out, e->v.binop.left);
 fprintf(out, " * ");
 emit_expr(out, e->v.binop.right);
 break;
 case BOP_DIV:
 emit_expr(out, e->v.binop.left);
 fprintf(out, " / ");
 emit_expr(out, e->v.binop.right);
 break;
 case BOP_MOD:
 fprintf(out, "fmod(");
 emit_expr(out, e->v.binop.left);
 fprintf(out, ", ");
 emit_expr(out, e->v.binop.right);
 fprintf(out, ")");
 break;
 case BOP_CONCAT:
 fprintf(out, "bpp_concat(");
 emit_expr(out, e->v.binop.left);
 fprintf(out, ", ");
 emit_expr(out, e->v.binop.right);
 fprintf(out, ")");
 break;
 case BOP_EQ:
 if (expr_is_string(e->v.binop.left) ||
 expr_is_string(e->v.binop.right)) {
 fprintf(out, "(strcmp(");
 emit_expr(out, e->v.binop.left);
 fprintf(out, ", ");
 emit_expr(out, e->v.binop.right);
 fprintf(out, ") == 0)");
 } else {
 emit_expr(out, e->v.binop.left);
 fprintf(out, " == ");
 emit_expr(out, e->v.binop.right);
 }
 break;
 case BOP_NE:
 if (expr_is_string(e->v.binop.left) ||
 expr_is_string(e->v.binop.right)) {
 fprintf(out, "(strcmp(");
 emit_expr(out, e->v.binop.left);
 fprintf(out, ", ");
 emit_expr(out, e->v.binop.right);
 fprintf(out, ") != 0)");
 } else {
 emit_expr(out, e->v.binop.left);
 fprintf(out, " != ");
 emit_expr(out, e->v.binop.right);
 }
 break;
 case BOP_LT:
 if (expr_is_string(e->v.binop.left) ||
 expr_is_string(e->v.binop.right)) {
 fprintf(out, "(strcmp(");
 emit_expr(out, e->v.binop.left);
 fprintf(out, ", ");
 emit_expr(out, e->v.binop.right);
 fprintf(out, ") < 0)");
 } else {
 emit_expr(out, e->v.binop.left);
 fprintf(out, " < ");
 emit_expr(out, e->v.binop.right);
 }
 break;
 case BOP_GT:
 if (expr_is_string(e->v.binop.left) ||
 expr_is_string(e->v.binop.right)) {
 fprintf(out, "(strcmp(");
 emit_expr(out, e->v.binop.left);
 fprintf(out, ", ");
 emit_expr(out, e->v.binop.right);
 fprintf(out, ") > 0)");
 } else {
 emit_expr(out, e->v.binop.left);
 fprintf(out, " > ");
 emit_expr(out, e->v.binop.right);
 }
 break;
 case BOP_LE:
 emit_expr(out, e->v.binop.left);
 fprintf(out, " <= ");
 emit_expr(out, e->v.binop.right);
 break;
 case BOP_GE:
 emit_expr(out, e->v.binop.left);
 fprintf(out, " >= ");
 emit_expr(out, e->v.binop.right);
 break;
 case BOP_POW:
 fprintf(out, "pow(");
 emit_expr(out, e->v.binop.left);
 fprintf(out, ", ");
 emit_expr(out, e->v.binop.right);
 fprintf(out, ")");
 break;
 case BOP_AND:
 fprintf(out, "((");
 emit_expr(out, e->v.binop.left);
 fprintf(out, ") && (");
 emit_expr(out, e->v.binop.right);
 fprintf(out, "))");
 break;
 case BOP_OR:
 fprintf(out, "((");
 emit_expr(out, e->v.binop.left);
 fprintf(out, ") || (");
 emit_expr(out, e->v.binop.right);
 fprintf(out, "))");
 break;
 }
 fprintf(out, ")");
 break;

 case EXPR_UNOP:
 if (e->v.unop.op == UOP_NEG) {
 fprintf(out, "(-(");
 emit_expr(out, e->v.unop.operand);
 fprintf(out, "))");
 } else if (e->v.unop.op == UOP_NOT) {
 fprintf(out, "(!(");
 emit_expr(out, e->v.unop.operand);
 fprintf(out, "))");
 }
 break;

 case EXPR_FUNC_CALL:
 switch (e->v.func_call.func) {
 case FUNC_ABS:
 fprintf(out, "fabs(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_RND:
 fprintf(out, "((double)(rand() %% (int)(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")))");
 break;
 case FUNC_SIZE:
 fprintf(out, "65536L");
 break;
 case FUNC_SIN:
 fprintf(out, "sin(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_COS:
 fprintf(out, "cos(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_TAN:
 fprintf(out, "tan(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_ATN:
 fprintf(out, "atan(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_SQR:
 fprintf(out, "sqrt(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_LOG:
 fprintf(out, "log(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_EXP:
 fprintf(out, "exp(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_SGN:
 fprintf(out, "((");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")>0?1:((");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")<0?-1:0))");
 break;
 case FUNC_INT:
 fprintf(out, "floor(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_LEN:
 fprintf(out, "(double)strlen(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_ASC:
 fprintf(out, "(double)(unsigned char)(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")[0]");
 break;
 case FUNC_VAL:
 fprintf(out, "atof(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_CHR:
 fprintf(out, "bpp_chr((int)(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, "))");
 break;
 case FUNC_STR:
 fprintf(out, "bpp_str(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;

 case FUNC_LEFT:
 fprintf(out, "bpp_left(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ", (int)(");
 emit_expr(out, e->v.func_call.args[1]);
 fprintf(out, "))");
 break;
 case FUNC_LCASES:
 fprintf(out, "bpp_lcases(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_UCASES:
 fprintf(out, "bpp_ucases(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_TCASES:
 fprintf(out, "bpp_tcases(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_MCASES:
 fprintf(out, "bpp_mcases(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_ICASES:
 fprintf(out, "bpp_icases(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_TRIMS:
 fprintf(out, "bpp_trims(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_LTRIMS:
 fprintf(out, "bpp_ltrims(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_RTRIMS:
 fprintf(out, "bpp_rtrims(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 case FUNC_REPLACES:
 fprintf(out, "bpp_replaces(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ", ");
 emit_expr(out, e->v.func_call.args[1]);
 fprintf(out, ", ");
 emit_expr(out, e->v.func_call.args[2]);
 fprintf(out, ")");
 break;
 case FUNC_REVERSES:
 fprintf(out, "bpp_reverses(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;

 case FUNC_RIGHT:
 fprintf(out, "bpp_right(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ", (int)(");
 emit_expr(out, e->v.func_call.args[1]);
 fprintf(out, "))");
 break;
 case FUNC_MID:
 fprintf(out, "bpp_mid(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ", (int)(");
 emit_expr(out, e->v.func_call.args[1]);
 fprintf(out, "), (int)(");
 emit_expr(out, e->v.func_call.args[2]);
 fprintf(out, "))");
 break;
 case FUNC_TAB:
 fprintf(out, "bpp_tab((int)(");
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, "))");
 break;
 case FUNC_FN_USER:
 fprintf(out, "bpp_fn_%c(",
 e->v.func_call.fn_letter);
 emit_expr(out, e->v.func_call.args[0]);
 fprintf(out, ")");
 break;
 default:
 break;
 }
 break;
 }
}

static void emit_stmt_comment(FILE *out, AstStmt *s, const char *generic, int indent, AstStmt *line_stmts) {
    const char *built_in_remark = NULL;
    if (line_stmts) {
        AstStmt *curr = line_stmts;
        while (curr) {
            if (curr->type == STMT_REM) {
                built_in_remark = curr->v.rem.text;
                break;
            }
            curr = curr->next;
        }
    }

    int i;
    if (built_in_remark && built_in_remark[0] != '\0') {
        // Find the first non-REM statement on the line.
        AstStmt *first_non_rem = line_stmts;
        while (first_non_rem && first_non_rem->type == STMT_REM) {
            first_non_rem = first_non_rem->next;
        }
        if (s == first_non_rem) {
            for (i = 0; i < indent; i++) {
                fprintf(out, " ");
            }
            fprintf(out, "/* REM %s */\n", built_in_remark);
        }
    } else if (generic) {
        for (i = 0; i < indent; i++) {
            fprintf(out, " ");
        }
        fprintf(out, "/* %s */\n", generic);
    }
}

 // emit_stmt - Emit C code for a single statement.
static void emit_stmt(FILE *out, AstStmt *s, int indent, AstStmt *line_stmts, double ln, const char *filename)
{
 int i;

 if (!s) return;

 // Emit comment if applicable
 switch (s->type) {
     case STMT_IF:
         emit_stmt_comment(out, s, "BASIC: IF statement", indent, line_stmts);
         break;
     case STMT_GOTO:
         emit_stmt_comment(out, s, "BASIC: GOTO", indent, line_stmts);
         break;
     case STMT_GOSUB:
         emit_stmt_comment(out, s, "BASIC: GOSUB", indent, line_stmts);
         break;
     case STMT_RETURN:
         emit_stmt_comment(out, s, "BASIC: RETURN", indent, line_stmts);
         break;
     case STMT_FOR:
         emit_stmt_comment(out, s, "BASIC: FOR loop initialization", indent, line_stmts);
         break;
     case STMT_NEXT:
         emit_stmt_comment(out, s, "BASIC: NEXT loop step", indent, line_stmts);
         break;
     case STMT_DIM:
         emit_stmt_comment(out, s, "BASIC: DIM (Array Allocation)", indent, line_stmts);
         break;
     case STMT_DATA:
         emit_stmt_comment(out, s, "BASIC: DATA definition", indent, line_stmts);
         break;
     case STMT_READ:
         emit_stmt_comment(out, s, "BASIC: READ statement", indent, line_stmts);
         break;
     case STMT_RESTORE:
         emit_stmt_comment(out, s, "BASIC: RESTORE pointer", indent, line_stmts);
         break;
     case STMT_WHILE:
     case STMT_DO:
         emit_stmt_comment(out, s, "BASIC: loop control", indent, line_stmts);
         break;
     case STMT_ON_GOTO:
         emit_stmt_comment(out, s, "BASIC: ON GOTO branch", indent, line_stmts);
         break;
     default:
         // Prioritize built-in REM if present
         emit_stmt_comment(out, s, NULL, indent, line_stmts);
         break;
 }

 // Indentation
 for (i = 0; i < indent; i++) fprintf(out, " ");

 switch (s->type) {
 case STMT_PRINT:
 {
 int pi;
 for (pi = 0; pi < s->v.print.item_count; pi++) {
 AstPrintItem *item = &s->v.print.items[pi];
 if (item->is_hash_width) {
 fprintf(out, "bpp_print_width = (int)(");
 emit_expr(out, item->expr);
 fprintf(out, ");\n");
 for (i = 0; i < indent; i++)
 fprintf(out, " ");
 continue;
 }
 if (item->expr) {
 // Determine if string or numeric
 if (expr_is_string(item->expr)) {
 fprintf(out, "printf(\"%%s\", ");
 emit_expr(out, item->expr);
 fprintf(out, ");\n");
 } else if (item->expr->type == EXPR_FLOAT_LIT) {
 fprintf(out, "printf(\"%%g\", ");
 emit_expr(out, item->expr);
 fprintf(out, ");\n");
 } else {
 fprintf(out,
 "{ double _v = ");
 emit_expr(out, item->expr);
 fprintf(out,
 "; if (_v == (double)(long)_v)"
 " printf(\"%%*ld\", bpp_print_width,"
 " (long)_v);"
 " else printf(\"%%g\", _v); }\n");
 }
 if (pi < s->v.print.item_count - 1) {
 for (i = 0; i < indent; i++)
 fprintf(out, " ");
 }
 } else {
 // NULL expr = tab advance (comma separator)
 fprintf(out,
 "printf(\"%%*s\", bpp_print_width, \"\");\n");
 if (pi < s->v.print.item_count - 1) {
 for (i = 0; i < indent; i++)
 fprintf(out, " ");
 }
 }
 }
 if (!s->v.print.trailing_comma) {
 for (i = 0; i < indent; i++) fprintf(out, " ");
 fprintf(out, "printf(\"\\n\");\n");
 }
 }
 break;

 	case STMT_LET:
		{
			int idx;
			if (s->v.let.name[0] != '\0') {
				idx = get_numvar_index(s->v.let.name);
			} else {
				char nm[2] = { s->v.let.var_name, '\0' };
				idx = get_numvar_index(nm);
			}
			fprintf(out, "bpp_vars[%d] = ", idx);
		}
		emit_expr(out, s->v.let.value);
		fprintf(out, ";\n");
		break;

	case STMT_LET_STRVAR:
		{
			int idx;
			if (s->v.let_strvar.name[0] != '\0') {
				idx = get_strvar_index(s->v.let_strvar.name);
			} else {
				char nm[2] = { s->v.let_strvar.var_name, '\0' };
				idx = get_strvar_index(nm);
			}
			fprintf(out, "strncpy(bpp_strvars[%d], ", idx);
			emit_expr(out, s->v.let_strvar.value);
			fprintf(out, ", 255); bpp_strvars[%d][255]='\\0';\n", idx);
		}
		break;

 case STMT_LET_ARRAY_AT:
 fprintf(out, "bpp_at_array[(int)(");
 emit_expr(out, s->v.let_array_at.index);
 fprintf(out, ")] = ");
 emit_expr(out, s->v.let_array_at.value);
 fprintf(out, ";\n");
 break;

 case STMT_LET_DIM:
 {
 int is_str = (s->v.let_dim.name[1] == '$');
 if (is_str) {
 // String array: strncpy into bpp_strdim_ref(...)
 char nm[2];
 nm[0] = s->v.let_dim.name[0];
 nm[1] = '\0';
 fprintf(out, "strncpy(bpp_strdim_ref(\"%s\", (int)(",
 nm);
 emit_expr(out, s->v.let_dim.idx1);
 fprintf(out, "), ");
 if (s->v.let_dim.idx2) {
 fprintf(out, "(int)(");
 emit_expr(out, s->v.let_dim.idx2);
 fprintf(out, ")");
 } else {
 fprintf(out, "0");
 }
 fprintf(out, "), ");
 emit_expr(out, s->v.let_dim.value);
 fprintf(out, ", 255);\n");
 } else {
 // Numeric array
 fprintf(out, "*bpp_dim_ref(\"%s\", (int)(",
 s->v.let_dim.name);
 emit_expr(out, s->v.let_dim.idx1);
 fprintf(out, "), ");
 if (s->v.let_dim.idx2) {
 fprintf(out, "(int)(");
 emit_expr(out, s->v.let_dim.idx2);
 fprintf(out, ")");
 } else {
 fprintf(out, "0");
 }
 fprintf(out, ") = ");
 emit_expr(out, s->v.let_dim.value);
 fprintf(out, ";\n");
 }
 }
 break;

 case STMT_IF:
 fprintf(out, "if (");
 emit_expr(out, s->v.if_stmt.condition);
 fprintf(out, ") { ");
 emit_stmt(out, s->v.if_stmt.then_stmt, 0, line_stmts, ln, filename);
 fprintf(out, " }\n");
 break;

 case STMT_GOTO:
 fprintf(out, "goto L");
 // GOTO target must be a constant int for label
 if (s->v.goto_stmt.target &&
 s->v.goto_stmt.target->type == EXPR_INT_LIT) {
 fprintf(out, "%ld", s->v.goto_stmt.target->v.ival);
 } else {
 fprintf(out, "0 /* computed goto unsupported */");
 }
 fprintf(out, ";\n");
 break;

 case STMT_GOSUB:
 if (s->v.gosub.target &&
 s->v.gosub.target->type == EXPR_INT_LIT) {
 long tgt = s->v.gosub.target->v.ival;
 fprintf(out,
 "bpp_gosub_stack[bpp_gosub_sp++] = %d;"
 " goto L%ld;\n",
 0 /* placeholder - filled by return dispatch */,
 tgt);
 }
 break;

 case STMT_RETURN:
 fprintf(out,
 "{ int _ra = bpp_gosub_stack[--bpp_gosub_sp];"
 " switch(_ra) {\n");
 // Return targets are resolved in the main emit loop
 fprintf(out,
 " default: goto bpp_end;\n"
 " } }\n");
 break;

 	case STMT_FOR:
		{
			int idx;
			if (s->v.for_stmt.name[0] != '\0') {
				idx = get_numvar_index(s->v.for_stmt.name);
			} else {
				char nm[2] = { s->v.for_stmt.var_name, '\0' };
				idx = get_numvar_index(nm);
			}
			fprintf(out, "bpp_vars[%d] = ", idx);
		}
		emit_expr(out, s->v.for_stmt.init);
		fprintf(out, ";\n");
		break;

	case STMT_NEXT:
		if (s->v.next.name[0] != '\0') {
			fprintf(out, "/* NEXT %s - see loop structure */\n", s->v.next.name);
		} else {
			fprintf(out, "/* NEXT %c - see loop structure */\n",
				s->v.next.var_name ? s->v.next.var_name : '?');
		}
		break;

 	case STMT_INPUT:
	{
		int vi;
		for (vi = 0; vi < s->v.input.var_count; vi++) {
			for (i = 0; i < indent && vi > 0; i++)
				fprintf(out, " ");
			if (s->v.input.prompt && vi == 0) {
				fprintf(out, "printf(\"%%s\", ");
				emit_expr(out, s->v.input.prompt);
				fprintf(out, ");\n");
				for (i = 0; i < indent; i++)
					fprintf(out, " ");
			} else if (vi == 0) {
				fprintf(out, "printf(\"? \");\n");
				for (i = 0; i < indent; i++)
					fprintf(out, " ");
			}
			fprintf(out, "fflush(stdout);\n");
			for (i = 0; i < indent; i++)
				fprintf(out, " ");

			AstExpr *var = s->v.input.vars[vi];
			if (expr_is_string(var)) {
				// String input
				fprintf(out, "{ char _buf[256]; int _len;\n");
				for (i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "  fgets(_buf, 256, stdin); _len=(int)strlen(_buf);\n");
				for (i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "  while(_len>0 && (_buf[_len-1]=='\\n'||_buf[_len-1]=='\\r')) _buf[--_len]='\\0';\n");
				for (i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "  strncpy(");
				emit_expr(out, var);
				fprintf(out, ", _buf, 255); }\n");
			} else {
				// Numeric input
				fprintf(out, "{ char _buf[256]; double _v;\n");
				for (i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "  fgets(_buf, 256, stdin); _v = atof(_buf);\n");
				for (i = 0; i < indent; i++) fprintf(out, " ");
				fprintf(out, "  ");
				emit_expr(out, var);
				fprintf(out, " = _v; }\n");
			}
		}
	}
	break;

 case STMT_END:
 fprintf(out, "goto bpp_end;\n");
 break;

 case STMT_STOP:
 fprintf(out, "printf(\"STOPPED\\n\"); goto bpp_end;\n");
 break;

  case STMT_REM:
  {
      int has_prev_stmt = 0;
      if (line_stmts && line_stmts != s) {
          has_prev_stmt = 1;
      }
      if (!has_prev_stmt) {
          if (s->v.rem.text) {
              fprintf(out, "/* REM %s */\n", s->v.rem.text);
          } else {
              fprintf(out, "/* REM */\n");
          }
      }
  }
  break;

 case STMT_DIM:
 fprintf(out, "bpp_dim_alloc(\"%s\", (int)(",
 s->v.dim.name);
 emit_expr(out, s->v.dim.dim1);
 fprintf(out, "), ");
 if (s->v.dim.dim2) {
 fprintf(out, "(int)(");
 emit_expr(out, s->v.dim.dim2);
 fprintf(out, ")");
 } else {
 fprintf(out, "0");
 }
 fprintf(out, ");\n");
 break;

 case STMT_DATA:
 fprintf(out, "/* DATA - see initialization */\n");
 break;

 	case STMT_READ:
	{
		int ri;
		for (ri = 0; ri < s->v.read.var_count; ri++) {
			if (ri > 0) {
				for (i = 0; i < indent; i++)
					fprintf(out, " ");
			}
			if (s->v.read.var_types[ri] == 0) {
				// Simple numeric variable
				fprintf(out, "bpp_vars[%d] = bpp_read_num();\n",
					(int)(s->v.read.var_names[ri] - 'A'));
			} else if (s->v.read.var_types[ri] == 2) {
				// 1D Array element
				const char *dn = s->v.read.dim_names[ri];
				if (dn[0] == '\0') {
					char nm[2];
					nm[0] = s->v.read.var_names[ri]; nm[1] = '\0';
					dn = nm;
				}
				if (dn[strlen(dn)-1] == '$') {
					fprintf(out, "strncpy(bpp_strdim_ref(\"%s\", (int)(", dn);
					emit_expr(out, s->v.read.var_indices[ri]);
					fprintf(out, "), 0), bpp_read_str(), 255);\n");
				} else {
					fprintf(out, "*bpp_dim_ref(\"%s\", (int)(", dn);
					emit_expr(out, s->v.read.var_indices[ri]);
					fprintf(out, "), 0) = bpp_read_num();\n");
				}
			} else if (s->v.read.var_types[ri] == 3) {
				// 2D Array element
				const char *dn = s->v.read.dim_names[ri];
				if (dn[0] == '\0') {
					char nm[2];
					nm[0] = s->v.read.var_names[ri]; nm[1] = '\0';
					dn = nm;
				}
				if (dn[strlen(dn)-1] == '$') {
					fprintf(out, "strncpy(bpp_strdim_ref(\"%s\", (int)(", dn);
					emit_expr(out, s->v.read.var_indices[ri]);
					fprintf(out, "), (int)(");
					emit_expr(out, s->v.read.var_indices2[ri]);
					fprintf(out, ")), bpp_read_str(), 255);\n");
				} else {
					fprintf(out, "*bpp_dim_ref(\"%s\", (int)(", dn);
					emit_expr(out, s->v.read.var_indices[ri]);
					fprintf(out, "), (int)(");
					emit_expr(out, s->v.read.var_indices2[ri]);
					fprintf(out, ")) = bpp_read_num();\n");
				}
			} else {
				// String variable
				fprintf(out, "strncpy(bpp_strvars[%d], bpp_read_str(), 255);\n",
					(int)(s->v.read.var_names[ri] - 'A'));
			}
		}
	}
	break;

 case STMT_RESTORE:
 fprintf(out, "bpp_data_ptr = 0;\n");
 break;

 case STMT_WHILE:
 case STMT_WEND:
 case STMT_DO:
 case STMT_LOOP:
 fprintf(out, "/* %s - see loop structure */\n",
 s->type == STMT_WHILE ? "WHILE" :
 s->type == STMT_WEND ? "WEND" :
 s->type == STMT_DO ? "DO" : "LOOP");
 break;

 case STMT_ON_GOTO:
 {
 int oi;
 fprintf(out, "{ int _sel = (int)(");
 emit_expr(out, s->v.on_goto.selector);
 fprintf(out, ");\n");
 for (oi = 0; oi < s->v.on_goto.target_count; oi++) {
 for (i = 0; i < indent + 1; i++)
 fprintf(out, " ");
 if (s->v.on_goto.is_gosub) {
 fprintf(out, "if (_sel == %d) { bpp_gosub_stack[bpp_gosub_sp++] = 0; goto L%.0f; }\n",
 oi + 1, s->v.on_goto.targets[oi]);
 } else {
 fprintf(out, "if (_sel == %d) goto L%.0f;\n",
 oi + 1, s->v.on_goto.targets[oi]);
 }
 }
 for (i = 0; i < indent; i++)
 fprintf(out, " ");
 fprintf(out, "}\n");
 }
 break;

 case STMT_DEF_FN:
 // DEF FNA(X) = expr
 // We emit a static function at the statement location.
 // The function uses bpp_fn_param as the parameter. 
  fprintf(out, "/* DEF FN%c - defined as inline */\n",
  s->v.def_fn.func_name);
  break;

  case STMT_ASSERT:
  fprintf(out, "BPP_ASSERT(");
  emit_expr(out, s->v.assert_stmt.condition);
  fprintf(out, ", \"assertion failed\", ");
  if (s->v.assert_stmt.message) {
      emit_expr(out, s->v.assert_stmt.message);
  } else {
      fprintf(out, "NULL");
  }
  fprintf(out, ", %.0f, \"%s\");\n", ln, filename ? filename : "program.bas");
  break;

  case STMT_TRON:
  fprintf(out, "#ifdef BPP_DEBUG\n");
  fprintf(out, "bpp_tron = 1;\n");
  fprintf(out, "#endif\n");
  break;

  case STMT_TROFF:
  fprintf(out, "#ifdef BPP_DEBUG\n");
  fprintf(out, "bpp_tron = 0;\n");
  fprintf(out, "#endif\n");
  break;

  case STMT_BREAK:
  fprintf(out, "#ifdef BPP_DEBUG\n");
  fprintf(out, "bpp_breakpoint(%.0f, \"%s\");\n", ln, filename ? filename : "program.bas");
  fprintf(out, "#endif\n");
  break;

  case STMT_VARS:
  fprintf(out, "bpp_dump_vars();\n");
  break;
 default:
 break;
 }
}

// --- GOSUB/RETURN resolution ---
 // We need a two-pass approach:
 // Pass 1: Find all GOSUB sites and assign return IDs
 // Pass 2: Emit code with return dispatch in RETURN statements

typedef struct GosubSite {
 double line_num; // line containing the GOSUB
 int return_id; // unique return label ID
 double target_line; // GOSUB target
} GosubSite;

#define MAX_GOSUB_SITES 256

 // find_gosub_sites - Scan AST lines for GOSUB statements.
static int find_gosub_sites(AstLine *lines, int line_count,
 GosubSite *sites, int *site_count)
{
 int i;
 int id = 0;
 *site_count = 0;

 for (i = 0; i < line_count; i++) {
 AstStmt *s = lines[i].stmts;
 while (s) {
 if (s->type == STMT_GOSUB &&
 s->v.gosub.target &&
 s->v.gosub.target->type == EXPR_INT_LIT) {
 if (*site_count < MAX_GOSUB_SITES) {
 sites[*site_count].line_num = lines[i].line_number;
 sites[*site_count].return_id = id++;
 sites[*site_count].target_line =
 (int)s->v.gosub.target->v.ival;
 (*site_count)++;
 }
 }
 // Also check IF-then for nested GOSUB
 if (s->type == STMT_IF && s->v.if_stmt.then_stmt &&
 s->v.if_stmt.then_stmt->type == STMT_GOSUB) {
 AstStmt *gs = s->v.if_stmt.then_stmt;
 if (gs->v.gosub.target &&
 gs->v.gosub.target->type == EXPR_INT_LIT) {
 if (*site_count < MAX_GOSUB_SITES) {
 sites[*site_count].line_num =
 lines[i].line_number;
 sites[*site_count].return_id = id++;
 sites[*site_count].target_line =
 (int)gs->v.gosub.target->v.ival;
 (*site_count)++;
 }
 }
 }
 s = s->next;
 }
 }
 return id;
}

/// --- FOR/NEXT matching ---
typedef struct ForSite {
	double for_line; // line number of FOR statement
	char var_name; // loop variable
	char full_name[MAX_VAR_NAME_LEN + 1]; // full loop variable name
	AstExpr *limit; // limit expression
	AstExpr *step; // step expression (NULL = default 1)
	int loop_id; // unique label ID
} ForSite;

#define MAX_FOR_SITES 64

static int find_for_sites(AstLine *lines, int line_count,
	ForSite *sites, int *site_count)
{
	int i;
	int id = 0;
	*site_count = 0;

	for (i = 0; i < line_count; i++) {
		AstStmt *s = lines[i].stmts;
		while (s) {
			if (s->type == STMT_FOR && *site_count < MAX_FOR_SITES) {
				sites[*site_count].for_line = lines[i].line_number;
				sites[*site_count].var_name = s->v.for_stmt.var_name;
				strncpy(sites[*site_count].full_name, s->v.for_stmt.name, MAX_VAR_NAME_LEN);
				sites[*site_count].full_name[MAX_VAR_NAME_LEN] = '\0';
				sites[*site_count].limit = s->v.for_stmt.limit;
				sites[*site_count].step = s->v.for_stmt.step;
				sites[*site_count].loop_id = id++;
				(*site_count)++;
			}
			s = s->next;
		}
	}
	return id;
}

// Find the most recent FOR site for a given variable before line_num
static ForSite *find_for_for_next_named(ForSite *sites, int count,
	const char *full_name, char var, double next_line)
{
	int i;
	ForSite *best = NULL;
	// Find the FOR for this variable with the highest line_num
	// that is still <= next_line
	for (i = 0; i < count; i++) {
		int matched = 0;
		if (full_name && full_name[0] != '\0') {
			matched = (strcmp(sites[i].full_name, full_name) == 0);
		} else {
			matched = (sites[i].var_name == var);
		}
		if (matched && sites[i].for_line <= next_line) {
			best = &sites[i];
		}
	}
	return best;
}

// --- Label target collection ---
 // Scan AST for all GOTO/GOSUB/ON GOTO target line numbers
 // so we only emit labels for lines that are actually referenced.
#define MAX_GOTO_TARGETS 512

static void add_target(int *targets, int *count, int ln)
{
 int j;
 for (j = 0; j < *count; j++) {
 if (targets[j] == ln) return;
 }
 if (*count < MAX_GOTO_TARGETS) {
 targets[(*count)++] = ln;
 }
}

static void collect_goto_targets(AstLine *lines, int line_count,
 int *targets, int *target_count)
{
 int i;
 *target_count = 0;
 for (i = 0; i < line_count; i++) {
 AstStmt *s = lines[i].stmts;
 while (s) {
 if (s->type == STMT_GOTO && s->v.goto_stmt.target &&
 s->v.goto_stmt.target->type == EXPR_INT_LIT) {
 add_target(targets, target_count,
 (int)s->v.goto_stmt.target->v.ival);
 }
 if (s->type == STMT_GOSUB && s->v.gosub.target &&
 s->v.gosub.target->type == EXPR_INT_LIT) {
 add_target(targets, target_count,
 (int)s->v.gosub.target->v.ival);
 }
 if (s->type == STMT_ON_GOTO) {
 int oi;
 for (oi = 0; oi < s->v.on_goto.target_count; oi++) {
 add_target(targets, target_count,
 (int)s->v.on_goto.targets[oi]);
 }
 }
 if (s->type == STMT_IF && s->v.if_stmt.then_stmt) {
 AstStmt *ts = s->v.if_stmt.then_stmt;
 if (ts->type == STMT_GOTO && ts->v.goto_stmt.target &&
 ts->v.goto_stmt.target->type == EXPR_INT_LIT) {
 add_target(targets, target_count,
 (int)ts->v.goto_stmt.target->v.ival);
 }
 if (ts->type == STMT_GOSUB && ts->v.gosub.target &&
 ts->v.gosub.target->type == EXPR_INT_LIT) {
 add_target(targets, target_count,
 (int)ts->v.gosub.target->v.ival);
 }
 }
 s = s->next;
 }
 }
}

static int is_label_target(int *targets, int count, int ln)
{
 int j;
 for (j = 0; j < count; j++) {
 if (targets[j] == ln) return 1;
 }
 return 0;
}

// --- Main emit function ---

static void emit_data_init(FILE *out, ProgramStore *program)
{
	int i;
	int data_idx = 0;

	if (!program) return;

	for (i = 0; i < program->count; i++) {
		const char *line = program->lines[i].text;
		const char *p = line;
		int in_quotes = 0;
		const char *data_start = NULL;

		// Scan for DATA keyword outside quotes
		while (*p) {
			if (*p == '"') {
				in_quotes = !in_quotes;
			} else if (!in_quotes) {
				if ((p[0] == 'D' || p[0] == 'd') &&
					(p[1] == 'A' || p[1] == 'a') &&
					(p[2] == 'T' || p[2] == 't') &&
					(p[3] == 'A' || p[3] == 'a')) {
					int is_boundary = 0;
					if (p == line) is_boundary = 1;
					else {
						char prev = p[-1];
						if (isspace((unsigned char)prev) || prev == ':' || isdigit((unsigned char)prev)) {
							is_boundary = 1;
						}
					}
					if (is_boundary) {
						data_start = p + 4;
						break;
					}
				}
			}
			p++;
		}

		if (!data_start) continue;

		p = data_start;
		while (*p == ' ' || *p == '\t') p++;

		while (*p && *p != '\r' && *p != '\n' && *p != ':') {
			char item[1024];
			int item_len = 0;

			while (*p == ' ' || *p == '\t') p++;
			if (*p == '\0' || *p == '\r' || *p == '\n' || *p == ':') break;

			if (*p == '"') {
				p++; // skip starting quote
				while (*p && *p != '"' && *p != '\r' && *p != '\n') {
					if (item_len < 1023) item[item_len++] = *p;
					p++;
				}
				if (*p == '"') p++; // skip ending quote
			} else {
				while (*p && *p != ',' && *p != ':' && *p != '\r' && *p != '\n') {
					if (item_len < 1023) item[item_len++] = *p;
					p++;
				}
				// Strip trailing whitespace
				while (item_len > 0 && (item[item_len-1] == ' ' || item[item_len-1] == '\t')) {
					item_len--;
				}
			}
			item[item_len] = '\0';

			// Escape characters for C double quotes
			char escaped[2048];
			int esc_len = 0;
			for (int k = 0; k < item_len; k++) {
				if (item[k] == '"') {
					escaped[esc_len++] = '\\';
					escaped[esc_len++] = '"';
				} else if (item[k] == '\\') {
					escaped[esc_len++] = '\\';
					escaped[esc_len++] = '\\';
				} else if (item[k] == '\n') {
					escaped[esc_len++] = '\\';
					escaped[esc_len++] = 'n';
				} else if (item[k] == '\r') {
					escaped[esc_len++] = '\\';
					escaped[esc_len++] = 'r';
				} else {
					escaped[esc_len++] = item[k];
				}
			}
			escaped[esc_len] = '\0';

			fprintf(out, "  bpp_data_pool[%d] = \"%s\";\n", data_idx, escaped);
			data_idx++;

			while (*p == ' ' || *p == '\t') p++;
			if (*p == ',') {
				p++;
				continue;
			}
			break;
		}
	}

	fprintf(out, "  bpp_data_count = %d;\n", data_idx);
}

int codegen_emit(FILE *out, AstLine *lines, int line_count,
                 ProgramStore *program, const TargetConfig *target)
{
	num_vars_count = 26;
	str_vars_count = 26;

	int i;
 GosubSite gosub_sites[MAX_GOSUB_SITES];
 int gosub_count = 0;
 ForSite for_sites[MAX_FOR_SITES];
 int for_count = 0;
 int goto_targets[MAX_GOTO_TARGETS];
 int goto_target_count = 0;
 int has_end_or_stop = 0;

 // Pass 1: Find GOSUB, FOR, and GOTO target sites
 find_gosub_sites(lines, line_count, gosub_sites, &gosub_count);
 find_for_sites(lines, line_count, for_sites, &for_count);
 collect_goto_targets(lines, line_count,
                      goto_targets, &goto_target_count);

 // Check if program uses END or STOP (which need bpp_end)
 for (i = 0; i < line_count; i++) {
 AstStmt *s = lines[i].stmts;
 while (s) {
 if (s->type == STMT_END || s->type == STMT_STOP ||
 s->type == STMT_RETURN) {
 has_end_or_stop = 1;
 }
 s = s->next;
 }
 }

 // Emit runtime shim
 emit_runtime_shim(out, target);

 // Emit user-defined functions (DEF FN) as static functions
 for (i = 0; i < line_count; i++) {
 AstStmt *s = lines[i].stmts;
 while (s) {
 if (s->type == STMT_DEF_FN) {
 char fc = s->v.def_fn.func_name;
 char pn = s->v.def_fn.param_name;
 fprintf(out, "static double bpp_fn_%c(double %c) {\n",
 fc, pn);
 fprintf(out, " return ");
 emit_expr(out, s->v.def_fn.body);
 fprintf(out, ";\n}\n\n");
 }
 s = s->next;
 }
 }

 // Emit main function
 fprintf(out, "int main(void) {\n");

 // Emit DATA initialization
 emit_data_init(out, program);
 fprintf(out, "\n");


 // Emit each BASIC line as a labeled block
 for (i = 0; i < line_count; i++) {
 AstStmt *s = lines[i].stmts;
 double ln = lines[i].line_number;

  // Only emit label if this line is a GOTO/GOSUB target
  if (is_label_target(goto_targets, goto_target_count, (int)ln)) {
  fprintf(out, " L%.0f: /* Line %.0f */\n", ln, ln);
  } else {
  fprintf(out, " /* Line %.0f */\n", ln);
  }

  fprintf(out, "#line %.0f \"%s\"\n", ln, (program && program->filename[0] != '\0') ? program->filename : "program.bas");
  fprintf(out, "#ifdef BPP_DEBUG\n");
  fprintf(out, "  bpp_trace_statement(%.0f, \"%s\");\n", ln, (program && program->filename[0] != '\0') ? program->filename : "program.bas");
  fprintf(out, "#endif\n");

 while (s) {
 		// Special handling for FOR - emit init + loop label
		if (s->type == STMT_FOR) {
			emit_stmt_comment(out, s, "BASIC: FOR loop initialization", 1, lines[i].stmts);
			ForSite *fs = NULL;
			int fi;
			for (fi = 0; fi < for_count; fi++) {
				int matched = 0;
				if (s->v.for_stmt.name[0] != '\0') {
					matched = (strcmp(for_sites[fi].full_name, s->v.for_stmt.name) == 0);
				} else {
					matched = (for_sites[fi].var_name == s->v.for_stmt.var_name);
				}
				if (for_sites[fi].for_line == ln && matched) {
					fs = &for_sites[fi];
					break;
				}
			}
			int idx;
			if (s->v.for_stmt.name[0] != '\0') {
				idx = get_numvar_index(s->v.for_stmt.name);
			} else {
				char nm[2] = { s->v.for_stmt.var_name, '\0' };
				idx = get_numvar_index(nm);
			}
			fprintf(out, " bpp_vars[%d] = ", idx);
			emit_expr(out, s->v.for_stmt.init);
			fprintf(out, ";\n");
			if (fs) {
				fprintf(out, " bpp_for_%d:;\n", fs->loop_id);
			}
			s = s->next;
			continue;
		}

		// Special handling for NEXT - emit step + check + goto
		if (s->type == STMT_NEXT) {
			emit_stmt_comment(out, s, "BASIC: NEXT loop step", 1, lines[i].stmts);
			ForSite *fs = find_for_for_next_named(
				for_sites, for_count, s->v.next.name, s->v.next.var_name, ln);
			if (fs) {
				int vi;
				if (s->v.next.name[0] != '\0') {
					vi = get_numvar_index(s->v.next.name);
				} else {
					char nm[2] = { s->v.next.var_name, '\0' };
					vi = get_numvar_index(nm);
				}
				// Increment
				fprintf(out, " bpp_vars[%d] += ", vi);
				if (fs->step) {
					emit_expr(out, fs->step);
				} else {
					fprintf(out, "1");
				}
				fprintf(out, ";\n");
				// Check limit
				if (fs->step) {
					// Signed step: if step>0, check <= limit;
					// if step<0, check >= limit.
					//  Use runtime check for variable step. 
					fprintf(out, " if ((");
					emit_expr(out, fs->step);
					fprintf(out, " > 0 && bpp_vars[%d] <= ", vi);
					emit_expr(out, fs->limit);
					fprintf(out, ") || (");
					emit_expr(out, fs->step);
					fprintf(out, " < 0 && bpp_vars[%d] >= ", vi);
					emit_expr(out, fs->limit);
					fprintf(out, ")) goto bpp_for_%d;\n", fs->loop_id);
				} else {
					fprintf(out, " if (bpp_vars[%d] <= ", vi);
					emit_expr(out, fs->limit);
					fprintf(out, ") goto bpp_for_%d;\n", fs->loop_id);
				}
			} else {
				if (s->v.next.name[0] != '\0') {
					fprintf(out, " /* NEXT %s: no matching FOR */\n", s->v.next.name);
				} else {
					fprintf(out, " /* NEXT %c: no matching FOR */\n", s->v.next.var_name);
				}
			}
			s = s->next;
			continue;
		}

 // Special handling for GOSUB - emit return ID
 if (s->type == STMT_GOSUB &&
 s->v.gosub.target &&
 s->v.gosub.target->type == EXPR_INT_LIT) {
 emit_stmt_comment(out, s, "BASIC: GOSUB", 1, lines[i].stmts);
 int gi;
 long tgt = s->v.gosub.target->v.ival;
 for (gi = 0; gi < gosub_count; gi++) {
 if (gosub_sites[gi].line_num == ln &&
 gosub_sites[gi].target_line == (int)tgt) {
 fprintf(out,
 " bpp_gosub_stack[bpp_gosub_sp++]"
 " = %d; goto L%ld;\n",
 gosub_sites[gi].return_id, tgt);
 fprintf(out, " bpp_ret_%d:;\n",
 gosub_sites[gi].return_id);
 break;
 }
 }
 s = s->next;
 continue;
 }

 // Special handling for RETURN - emit dispatch
 if (s->type == STMT_RETURN) {
 emit_stmt_comment(out, s, "BASIC: RETURN", 1, lines[i].stmts);
 fprintf(out,
 " { int _ra ="
 " bpp_gosub_stack[--bpp_gosub_sp];\n"
 " switch(_ra) {\n");
 {
 int gi;
 for (gi = 0; gi < gosub_count; gi++) {
 fprintf(out,
 " case %d: goto bpp_ret_%d;\n",
 gosub_sites[gi].return_id,
 gosub_sites[gi].return_id);
 }
 }
 fprintf(out,
 " default: goto bpp_end;\n"
 " } }\n");
 s = s->next;
 continue;
 }

 // Special handling for IF-GOSUB
 if (s->type == STMT_IF && s->v.if_stmt.then_stmt &&
 s->v.if_stmt.then_stmt->type == STMT_GOSUB) {
 emit_stmt_comment(out, s, "BASIC: IF statement", 1, lines[i].stmts);
 AstStmt *gs = s->v.if_stmt.then_stmt;
 if (gs->v.gosub.target &&
 gs->v.gosub.target->type == EXPR_INT_LIT) {
 int gi;
 long tgt = gs->v.gosub.target->v.ival;
 fprintf(out, " if (");
 emit_expr(out, s->v.if_stmt.condition);
 fprintf(out, ") { ");
 for (gi = 0; gi < gosub_count; gi++) {
 if (gosub_sites[gi].line_num == ln &&
 gosub_sites[gi].target_line == (int)tgt) {
 fprintf(out,
 "bpp_gosub_stack[bpp_gosub_sp++]"
 " = %d; goto L%ld;",
 gosub_sites[gi].return_id, tgt);
 break;
 }
 }
 fprintf(out, " }\n");
 for (gi = 0; gi < gosub_count; gi++) {
 if (gosub_sites[gi].line_num == ln &&
 gosub_sites[gi].target_line == (int)tgt) {
 fprintf(out, " bpp_ret_%d:;\n",
 gosub_sites[gi].return_id);
 break;
 }
 }
 s = s->next;
 continue;
 }
 }

 // Normal statement
  emit_stmt(out, s, 1, lines[i].stmts, ln, program ? program->filename : "program.bas");
  s = s->next;
 }
 }

 // End label and return
 if (has_end_or_stop || gosub_count > 0) {
 fprintf(out, " bpp_end:\n");
 } else {
 fprintf(out, " /* bpp_end: (unused) */\n");
 }
 fprintf(out, " return 0;\n");
 fprintf(out, "}\n");

 return 0;
}

