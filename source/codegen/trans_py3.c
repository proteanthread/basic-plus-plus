/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: trans_py3.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Python 3 transpiler backend for BASIC++.
 *    Translates AST lines into a Python 3 match/case GOTO state-machine loop.
 * ===================================================================== */

#include "trans.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#ifndef MAX_FOR_SITES
#define MAX_FOR_SITES 64
#endif

typedef struct {
    double for_line;     // line number containing FOR
    char var_name;       // loop variable
    char full_name[64];  // full loop variable name
    AstExpr *limit;      // limit expression
    AstExpr *step;       // step expression (NULL = default 1)
    int loop_id;         // unique label ID
} PyForSite;

static int py_find_for_sites(AstLine *lines, int line_count,
                             PyForSite *sites, int *site_count)
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
                strncpy(sites[*site_count].full_name, s->v.for_stmt.name, 63);
                sites[*site_count].full_name[63] = '\0';
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

static PyForSite *py_find_for_for_next_named(PyForSite *sites, int count,
                                            const char *full_name, char var, double next_line)
{
    int i;
    PyForSite *best = NULL;
    for (i = 0; i < count; i++) {
        int matched = 0;
        if (full_name && full_name[0] != '\0') {
            matched = (strcmp(sites[i].full_name, full_name) == 0);
        } else {
            matched = (sites[i].var_name == var);
        }
        if (matched && sites[i].for_line <= next_line) {
            if (!best || sites[i].for_line > best->for_line) {
                best = &sites[i];
            }
        }
    }
    return best;
}

static int expr_is_string(AstExpr *e) {
    if (!e) return 0;
    if (e->type == EXPR_STRING_LIT || e->type == EXPR_STRING_VAR) return 1;
    if (e->type == EXPR_NAMED_VAR) {
        size_t len = strlen(e->v.named.name);
        if (len > 0 && e->v.named.name[len-1] == '$') return 1;
    }
    if (e->type == EXPR_DIM_ACCESS) {
        size_t len = strlen(e->v.dim_access.name);
        if (len > 0 && e->v.dim_access.name[len-1] == '$') return 1;
    }
    if (e->type == EXPR_FUNC_CALL) {
        if (e->v.func_call.func == FUNC_CHR ||
            e->v.func_call.func == FUNC_STR ||
            e->v.func_call.func == FUNC_LEFT ||
            e->v.func_call.func == FUNC_RIGHT ||
            e->v.func_call.func == FUNC_MID ||
            e->v.func_call.func == FUNC_MEMMAP ||
            e->v.func_call.func == FUNC_VPATH ||
            e->v.func_call.func == FUNC_CWD) {
            return 1;
        }
    }
    if (e->type == EXPR_BINOP && e->v.binop.op == BOP_CONCAT) return 1;
    return 0;
}

static void emit_py3_var_name(FILE *out, char var_char, int is_string) {
    fprintf(out, "%c", tolower((unsigned char)var_char));
    if (is_string) {
        fprintf(out, "_str");
    }
}

static void emit_py3_named_var(FILE *out, const char *name) {
    size_t len = strlen(name);
    for (size_t i = 0; i < len; i++) {
        if (name[i] == '$') {
            fprintf(out, "_str");
        } else if (name[i] == '!' || name[i] == '#' || name[i] == '%' || name[i] == '&') {
            /* Skip type suffixes in Python */
        } else {
            fprintf(out, "%c", tolower((unsigned char)name[i]));
        }
    }
}

static void emit_py3_expr(FILE *out, AstExpr *e);

static void emit_py3_expr(FILE *out, AstExpr *e) {
    if (!e) return;
    switch (e->type) {
        case EXPR_INT_LIT:
            fprintf(out, "%ld", e->v.ival);
            break;
        case EXPR_FLOAT_LIT:
            fprintf(out, "%g", e->v.fval);
            break;
        case EXPR_STRING_LIT: {
            fprintf(out, "\"");
            for (int i = 0; i < e->v.sval.length; i++) {
                char c = e->v.sval.data[i];
                if (c == '"') fprintf(out, "\\\"");
                else if (c == '\\') fprintf(out, "\\\\");
                else if (c == '\n') fprintf(out, "\\n");
                else if (c == '\r') fprintf(out, "\\r");
                else fprintf(out, "%c", c);
            }
            fprintf(out, "\"");
            break;
        }
        case EXPR_VAR:
            emit_py3_var_name(out, e->v.var_name, 0);
            break;
        case EXPR_STRING_VAR:
            emit_py3_var_name(out, e->v.var_name, 1);
            break;
        case EXPR_NAMED_VAR:
            emit_py3_named_var(out, e->v.named.name);
            break;
        case EXPR_ARRAY_AT:
            fprintf(out, "at_array[int(");
            emit_py3_expr(out, e->v.array_at.index);
            fprintf(out, ")]");
            break;
        case EXPR_DIM_ACCESS:
            emit_py3_named_var(out, e->v.dim_access.name);
            fprintf(out, "[int(");
            emit_py3_expr(out, e->v.dim_access.idx1);
            fprintf(out, ")]");
            if (e->v.dim_access.idx2) {
                fprintf(out, "[int(");
                emit_py3_expr(out, e->v.dim_access.idx2);
                fprintf(out, ")]");
            }
            break;
        case EXPR_BINOP:
            if (e->v.binop.op == BOP_CONCAT) {
                fprintf(out, "str(");
                emit_py3_expr(out, e->v.binop.left);
                fprintf(out, ") + str(");
                emit_py3_expr(out, e->v.binop.right);
                fprintf(out, ")");
            } else {
                fprintf(out, "(");
                emit_py3_expr(out, e->v.binop.left);
                switch (e->v.binop.op) {
                    case BOP_ADD: fprintf(out, " + "); break;
                    case BOP_SUB: fprintf(out, " - "); break;
                    case BOP_MUL: fprintf(out, " * "); break;
                    case BOP_DIV: fprintf(out, " / "); break;
                    case BOP_MOD: fprintf(out, " %% "); break;
                    case BOP_POW: fprintf(out, " ** "); break;
                    case BOP_EQ:  fprintf(out, " == "); break;
                    case BOP_NE:  fprintf(out, " != "); break;
                    case BOP_LT:  fprintf(out, " < "); break;
                    case BOP_GT:  fprintf(out, " > "); break;
                    case BOP_LE:  fprintf(out, " <= "); break;
                    case BOP_GE:  fprintf(out, " >= "); break;
                    case BOP_AND: fprintf(out, " and "); break;
                    case BOP_OR:  fprintf(out, " or "); break;
                    default: break;
                }
                emit_py3_expr(out, e->v.binop.right);
                fprintf(out, ")");
            }
            break;
        case EXPR_UNOP:
            if (e->v.unop.op == UOP_NEG) {
                fprintf(out, "(-(");
                emit_py3_expr(out, e->v.unop.operand);
                fprintf(out, "))");
            } else if (e->v.unop.op == UOP_NOT) {
                fprintf(out, "(not (");
                emit_py3_expr(out, e->v.unop.operand);
                fprintf(out, "))");
            }
            break;
        case EXPR_FUNC_CALL: {
            switch (e->v.func_call.func) {
                case FUNC_ABS:
                    fprintf(out, "abs(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_RND:
                    // RND(1) or RND(n)
                    fprintf(out, "bpp_rnd(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_SIN:
                    fprintf(out, "math.sin(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_COS:
                    fprintf(out, "math.cos(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_TAN:
                    fprintf(out, "math.tan(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_ATN:
                    fprintf(out, "math.atan(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_SQR:
                    fprintf(out, "math.sqrt(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_LOG:
                    fprintf(out, "math.log(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_EXP:
                    fprintf(out, "math.exp(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_SGN:
                    fprintf(out, "bpp_sgn(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_INT:
                    fprintf(out, "math.floor(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_LEN:
                    fprintf(out, "len(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_ASC:
                    fprintf(out, "bpp_asc(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_VAL:
                    fprintf(out, "bpp_val(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_CHR:
                    fprintf(out, "chr(int(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, "))");
                    break;
                case FUNC_STR:
                    fprintf(out, "bpp_str(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                case FUNC_LEFT:
                    fprintf(out, "bpp_left(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ", ");
                    emit_py3_expr(out, e->v.func_call.args[1]);
                    fprintf(out, ")");
                    break;
                case FUNC_RIGHT:
                    fprintf(out, "bpp_right(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ", ");
                    emit_py3_expr(out, e->v.func_call.args[1]);
                    fprintf(out, ")");
                    break;
                case FUNC_MID:
                    fprintf(out, "bpp_mid(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ", ");
                    emit_py3_expr(out, e->v.func_call.args[1]);
                    if (e->v.func_call.arg_count > 2) {
                        fprintf(out, ", ");
                        emit_py3_expr(out, e->v.func_call.args[2]);
                    }
                    fprintf(out, ")");
                    break;
                case FUNC_TAB:
                    fprintf(out, "bpp_tab(");
                    emit_py3_expr(out, e->v.func_call.args[0]);
                    fprintf(out, ")");
                    break;
                default:
                    fprintf(out, "0");
                    break;
            }
            break;
        }
    }
}

// Simple variable collection pass to initialize them and avoid NameError in Python
typedef struct VarNode {
    char name[64];
    int is_str;
    struct VarNode *next;
} VarNode;

static void collect_expr_vars(AstExpr *e, VarNode **head) {
    if (!e) return;
    if (e->type == EXPR_VAR) {
        char nm[2] = { (char)tolower((unsigned char)e->v.var_name), '\0' };
        for (VarNode *curr = *head; curr; curr = curr->next) {
            if (strcmp(curr->name, nm) == 0 && curr->is_str == 0) return;
        }
        VarNode *n = malloc(sizeof(VarNode));
        strcpy(n->name, nm);
        n->is_str = 0;
        n->next = *head;
        *head = n;
    } else if (e->type == EXPR_STRING_VAR) {
        char nm[6] = { (char)tolower((unsigned char)e->v.var_name), '_', 's', 't', 'r', '\0' };
        for (VarNode *curr = *head; curr; curr = curr->next) {
            if (strcmp(curr->name, nm) == 0 && curr->is_str == 1) return;
        }
        VarNode *n = malloc(sizeof(VarNode));
        strcpy(n->name, nm);
        n->is_str = 1;
        n->next = *head;
        *head = n;
    } else if (e->type == EXPR_NAMED_VAR) {
        char nm[64] = {0};
        size_t len = strlen(e->v.named.name);
        int is_str = 0;
        size_t out_idx = 0;
        for (size_t i = 0; i < len; i++) {
            if (e->v.named.name[i] == '$') {
                is_str = 1;
                strcpy(&nm[out_idx], "_str");
                out_idx += 4;
            } else if (e->v.named.name[i] == '!' || e->v.named.name[i] == '#' || e->v.named.name[i] == '%' || e->v.named.name[i] == '&') {
                /* Skip type suffixes */
            } else {
                nm[out_idx++] = (char)tolower((unsigned char)e->v.named.name[i]);
            }
        }
        nm[out_idx] = '\0';
        for (VarNode *curr = *head; curr; curr = curr->next) {
            if (strcmp(curr->name, nm) == 0) return;
        }
        VarNode *n = malloc(sizeof(VarNode));
        strcpy(n->name, nm);
        n->is_str = is_str;
        n->next = *head;
        *head = n;
    } else if (e->type == EXPR_ARRAY_AT) {
        collect_expr_vars(e->v.array_at.index, head);
    } else if (e->type == EXPR_DIM_ACCESS) {
        collect_expr_vars(e->v.dim_access.idx1, head);
        collect_expr_vars(e->v.dim_access.idx2, head);
    } else if (e->type == EXPR_BINOP) {
        collect_expr_vars(e->v.binop.left, head);
        collect_expr_vars(e->v.binop.right, head);
    } else if (e->type == EXPR_UNOP) {
        collect_expr_vars(e->v.unop.operand, head);
    } else if (e->type == EXPR_FUNC_CALL) {
        for (int i = 0; i < e->v.func_call.arg_count; i++) {
            collect_expr_vars(e->v.func_call.args[i], head);
        }
    }
}

static void collect_stmt_vars(AstStmt *s, VarNode **head) {
    while (s) {
        switch (s->type) {
            case STMT_LET:
                {
                    char nm[64];
                    if (s->v.let.name[0] != '\0') {
                        size_t l = strlen(s->v.let.name);
                        size_t k;
                        size_t out_idx = 0;
                        for (k = 0; k < l; k++) {
                            char c = s->v.let.name[k];
                            if (c == '!' || c == '#' || c == '%' || c == '&') {
                                /* Skip */
                            } else {
                                nm[out_idx++] = (char)tolower((unsigned char)c);
                            }
                        }
                        nm[out_idx] = '\0';
                    } else {
                        nm[0] = (char)tolower((unsigned char)s->v.let.var_name);
                        nm[1] = '\0';
                    }
                    int found = 0;
                    for (VarNode *curr = *head; curr; curr = curr->next) {
                        if (strcmp(curr->name, nm) == 0) { found = 1; break; }
                    }
                    if (!found) {
                        VarNode *n = malloc(sizeof(VarNode));
                        strcpy(n->name, nm);
                        n->is_str = 0;
                        n->next = *head;
                        *head = n;
                    }
                }
                collect_expr_vars(s->v.let.value, head);
                break;
            case STMT_LET_STRVAR:
                {
                    char nm[64];
                    if (s->v.let_strvar.name[0] != '\0') {
                        size_t l = strlen(s->v.let_strvar.name);
                        size_t k = 0;
                        size_t out_idx = 0;
                        for (; k < l; k++) {
                            char c = s->v.let_strvar.name[k];
                            if (c == '$') {
                                strcpy(&nm[out_idx], "_str");
                                out_idx += 4;
                                break;
                            } else if (c == '!' || c == '#' || c == '%' || c == '&') {
                                /* Skip */
                            } else {
                                nm[out_idx++] = (char)tolower((unsigned char)c);
                            }
                        }
                        nm[out_idx] = '\0';
                    } else {
                        nm[0] = (char)tolower((unsigned char)s->v.let_strvar.var_name);
                        nm[1] = '_';
                        nm[2] = 's';
                        nm[3] = 't';
                        nm[4] = 'r';
                        nm[5] = '\0';
                    }
                    int found = 0;
                    for (VarNode *curr = *head; curr; curr = curr->next) {
                        if (strcmp(curr->name, nm) == 0) { found = 1; break; }
                    }
                    if (!found) {
                        VarNode *n = malloc(sizeof(VarNode));
                        strcpy(n->name, nm);
                        n->is_str = 1;
                        n->next = *head;
                        *head = n;
                    }
                }
                collect_expr_vars(s->v.let_strvar.value, head);
                break;
            case STMT_LET_DIM:
                collect_expr_vars(s->v.let_dim.idx1, head);
                collect_expr_vars(s->v.let_dim.idx2, head);
                collect_expr_vars(s->v.let_dim.value, head);
                break;
            case STMT_PRINT:
                for (int i = 0; i < s->v.print.item_count; i++) {
                    collect_expr_vars(s->v.print.items[i].expr, head);
                }
                break;
            case STMT_IF:
                collect_expr_vars(s->v.if_stmt.condition, head);
                collect_stmt_vars(s->v.if_stmt.then_stmt, head);
                break;
            case STMT_ASSERT:
                collect_expr_vars(s->v.assert_stmt.condition, head);
                if (s->v.assert_stmt.message) {
                    collect_expr_vars(s->v.assert_stmt.message, head);
                }
                break;
            case STMT_GOTO:
                collect_expr_vars(s->v.goto_stmt.target, head);
                break;
            case STMT_GOSUB:
                collect_expr_vars(s->v.gosub.target, head);
                break;
            case STMT_FOR:
                {
                    char nm[64];
                    if (s->v.for_stmt.name[0] != '\0') {
                        size_t l = strlen(s->v.for_stmt.name);
                        size_t k;
                        size_t out_idx = 0;
                        for (k = 0; k < l; k++) {
                            char c = s->v.for_stmt.name[k];
                            if (c == '!' || c == '#' || c == '%' || c == '&') {
                                /* Skip */
                            } else {
                                nm[out_idx++] = (char)tolower((unsigned char)c);
                            }
                        }
                        nm[out_idx] = '\0';
                    } else {
                        nm[0] = (char)tolower((unsigned char)s->v.for_stmt.var_name);
                        nm[1] = '\0';
                    }
                    int found = 0;
                    for (VarNode *curr = *head; curr; curr = curr->next) {
                        if (strcmp(curr->name, nm) == 0) { found = 1; break; }
                    }
                    if (!found) {
                        VarNode *n = malloc(sizeof(VarNode));
                        strcpy(n->name, nm);
                        n->is_str = 0;
                        n->next = *head;
                        *head = n;
                    }
                }
                collect_expr_vars(s->v.for_stmt.init, head);
                collect_expr_vars(s->v.for_stmt.limit, head);
                collect_expr_vars(s->v.for_stmt.step, head);
                break;
            case STMT_NEXT:
                break;
            case STMT_INPUT:
                collect_expr_vars(s->v.input.prompt, head);
                for (int i = 0; i < s->v.input.var_count; i++) {
                    collect_expr_vars(s->v.input.vars[i], head);
                }
                break;
            case STMT_WHILE:
                collect_expr_vars(s->v.while_stmt.left, head);
                collect_expr_vars(s->v.while_stmt.right, head);
                break;
            default:
                break;
        }
        s = s->next;
    }
}

static void emit_py3_stmt(FILE *out, AstStmt *s, int indent, AstLine *lines, int line_count, int current_idx, PyForSite *for_sites, int for_count) {
    while (s) {
        if (s->type == STMT_FOR) {
            PyForSite *fs = py_find_for_for_next_named(for_sites, for_count, s->v.for_stmt.name, s->v.for_stmt.var_name, lines[current_idx].line_number);
            if (fs) {
                for (int i = 0; i < indent; i++) fprintf(out, " ");
                fprintf(out, "if not loop_jumped_%d:\n", fs->loop_id);

                for (int i = 0; i < indent + 4; i++) fprintf(out, " ");
                if (s->v.for_stmt.name[0] != '\0') {
                    emit_py3_named_var(out, s->v.for_stmt.name);
                } else {
                    emit_py3_var_name(out, s->v.for_stmt.var_name, 0);
                }
                fprintf(out, " = ");
                emit_py3_expr(out, s->v.for_stmt.init);
                fprintf(out, "\n");

                for (int i = 0; i < indent; i++) fprintf(out, " ");
                fprintf(out, "loop_jumped_%d = False\n", fs->loop_id);
            }
            s = s->next;
            continue;
        }

        if (s->type == STMT_NEXT) {
            PyForSite *fs = py_find_for_for_next_named(for_sites, for_count, s->v.next.name, s->v.next.var_name, lines[current_idx].line_number);
            if (fs) {
                for (int i = 0; i < indent; i++) fprintf(out, " ");
                if (s->v.next.name[0] != '\0') {
                    emit_py3_named_var(out, s->v.next.name);
                } else {
                    emit_py3_var_name(out, s->v.next.var_name, 0);
                }
                fprintf(out, " += ");
                if (fs->step) {
                    emit_py3_expr(out, fs->step);
                } else {
                    fprintf(out, "1.0");
                }
                fprintf(out, "\n");

                for (int i = 0; i < indent; i++) fprintf(out, " ");
                fprintf(out, "if (");
                if (fs->step) {
                    fprintf(out, "(");
                    emit_py3_expr(out, fs->step);
                    fprintf(out, " > 0 and ");
                    if (s->v.next.name[0] != '\0') {
                        emit_py3_named_var(out, s->v.next.name);
                    } else {
                        emit_py3_var_name(out, s->v.next.var_name, 0);
                    }
                    fprintf(out, " <= ");
                    emit_py3_expr(out, fs->limit);
                    fprintf(out, ") or (");
                    emit_py3_expr(out, fs->step);
                    fprintf(out, " < 0 and ");
                    if (s->v.next.name[0] != '\0') {
                        emit_py3_named_var(out, s->v.next.name);
                    } else {
                        emit_py3_var_name(out, s->v.next.var_name, 0);
                    }
                    fprintf(out, " >= ");
                    emit_py3_expr(out, fs->limit);
                    fprintf(out, ")");
                } else {
                    if (s->v.next.name[0] != '\0') {
                        emit_py3_named_var(out, s->v.next.name);
                    } else {
                        emit_py3_var_name(out, s->v.next.var_name, 0);
                    }
                    fprintf(out, " <= ");
                    emit_py3_expr(out, fs->limit);
                }
                fprintf(out, "):\n");

                for (int i = 0; i < indent + 4; i++) fprintf(out, " ");
                fprintf(out, "loop_jumped_%d = True\n", fs->loop_id);

                for (int i = 0; i < indent + 4; i++) fprintf(out, " ");
                fprintf(out, "state = %.0f; continue\n", fs->for_line);
            } else {
                for (int i = 0; i < indent; i++) fprintf(out, " ");
                fprintf(out, "pass\n");
            }
            s = s->next;
            continue;
        }

        for (int i = 0; i < indent; i++) fprintf(out, " ");
        switch (s->type) {
            case STMT_PRINT: {
                if (s->v.print.item_count == 0) {
                    fprintf(out, "bpp_print_nl()\n");
                } else {
                    for (int pi = 0; pi < s->v.print.item_count; pi++) {
                        AstPrintItem *item = &s->v.print.items[pi];
                        if (item->expr) {
                            int is_str = expr_is_string(item->expr);
                            if (item->expr->type == EXPR_FUNC_CALL && item->expr->v.func_call.func == FUNC_TAB) {
                                emit_py3_expr(out, item->expr);
                                fprintf(out, "\n");
                            } else {
                                fprintf(out, "bpp_print_val(");
                                emit_py3_expr(out, item->expr);
                                fprintf(out, ", is_string=%s)\n", is_str ? "True" : "False");
                            }
                        } else {
                            // Separator comma
                            fprintf(out, "bpp_print_tab()\n");
                        }
                        if (pi < s->v.print.item_count - 1) {
                            for (int i = 0; i < indent; i++) fprintf(out, " ");
                        }
                    }
                    if (!s->v.print.trailing_comma) {
                        for (int i = 0; i < indent; i++) fprintf(out, " ");
                        fprintf(out, "bpp_print_nl()\n");
                    }
                }
                break;
            }
            case STMT_LET:
                if (s->v.let.name[0] != '\0') {
                    emit_py3_named_var(out, s->v.let.name);
                } else {
                    emit_py3_var_name(out, s->v.let.var_name, 0);
                }
                fprintf(out, " = ");
                emit_py3_expr(out, s->v.let.value);
                fprintf(out, "\n");
                break;
            case STMT_LET_STRVAR:
                if (s->v.let_strvar.name[0] != '\0') {
                    emit_py3_named_var(out, s->v.let_strvar.name);
                } else {
                    emit_py3_var_name(out, s->v.let_strvar.var_name, 1);
                }
                fprintf(out, " = ");
                emit_py3_expr(out, s->v.let_strvar.value);
                fprintf(out, "\n");
                break;
            case STMT_LET_ARRAY_AT:
                fprintf(out, "at_array[int(");
                emit_py3_expr(out, s->v.let_array_at.index);
                fprintf(out, ")] = ");
                emit_py3_expr(out, s->v.let_array_at.value);
                fprintf(out, "\n");
                break;
            case STMT_LET_DIM:
                emit_py3_named_var(out, s->v.let_dim.name);
                fprintf(out, "[int(");
                emit_py3_expr(out, s->v.let_dim.idx1);
                fprintf(out, ")]");
                if (s->v.let_dim.idx2) {
                    fprintf(out, "[int(");
                    emit_py3_expr(out, s->v.let_dim.idx2);
                    fprintf(out, ")]");
                }
                fprintf(out, " = ");
                emit_py3_expr(out, s->v.let_dim.value);
                fprintf(out, "\n");
                break;
            case STMT_ASSERT:
                for (int i = 0; i < indent; i++) fprintf(out, " ");
                fprintf(out, "assert ");
                emit_py3_expr(out, s->v.assert_stmt.condition);
                if (s->v.assert_stmt.message) {
                    fprintf(out, ", ");
                    emit_py3_expr(out, s->v.assert_stmt.message);
                }
                fprintf(out, "\n");
                break;
            case STMT_TRON:
            case STMT_TROFF:
            case STMT_BREAK:
                break;
            case STMT_VARS:
                for (int i = 0; i < indent; i++) fprintf(out, " ");
                fprintf(out, "print('--- Active Variables (Python) ---')\n");
                break;
            case STMT_IF:
                fprintf(out, "if ");
                emit_py3_expr(out, s->v.if_stmt.condition);
                fprintf(out, ":\n");
                emit_py3_stmt(out, s->v.if_stmt.then_stmt, indent + 4, lines, line_count, current_idx, for_sites, for_count);
                break;
            case STMT_GOTO:
                fprintf(out, "state = int(");
                emit_py3_expr(out, s->v.goto_stmt.target);
                fprintf(out, "); continue\n");
                break;
            case STMT_GOSUB: {
                double next_line = 0.0;
                if (current_idx + 1 < line_count) {
                    next_line = lines[current_idx + 1].line_number;
                }
                fprintf(out, "gosub_stack.append(%.0f)\n", next_line);
                for (int i = 0; i < indent; i++) fprintf(out, " ");
                fprintf(out, "state = int(");
                emit_py3_expr(out, s->v.gosub.target);
                fprintf(out, "); continue\n");
                break;
            }
            case STMT_RETURN:
                fprintf(out, "if not gosub_stack:\n");
                for (int i = 0; i < indent + 4; i++) fprintf(out, " ");
                fprintf(out, "print(\"RETURN WITHOUT GOSUB\", file=sys.stderr); sys.exit(1)\n");
                for (int i = 0; i < indent; i++) fprintf(out, " ");
                fprintf(out, "state = gosub_stack.pop(); continue\n");
                break;
            case STMT_FOR:
                // Init loop variable
                if (s->v.for_stmt.name[0] != '\0') {
                    emit_py3_named_var(out, s->v.for_stmt.name);
                } else {
                    emit_py3_var_name(out, s->v.for_stmt.var_name, 0);
                }
                fprintf(out, " = ");
                emit_py3_expr(out, s->v.for_stmt.init);
                fprintf(out, "\n");
                break;
            case STMT_NEXT: {
                if (s->v.next.name[0] != '\0') {
                    fprintf(out, "# NEXT %s\n", s->v.next.name);
                    for (int i = 0; i < indent; i++) fprintf(out, " ");
                    emit_py3_named_var(out, s->v.next.name);
                } else {
                    char nvar = s->v.next.var_name;
                    fprintf(out, "# NEXT %c\n", nvar);
                    for (int i = 0; i < indent; i++) fprintf(out, " ");
                    emit_py3_var_name(out, nvar, 0);
                }
                fprintf(out, " += 1\n");
                break;
            }
            case STMT_INPUT: {
                if (s->v.input.prompt) {
                    fprintf(out, "print(");
                    emit_py3_expr(out, s->v.input.prompt);
                    fprintf(out, ", end='')\n");
                    for (int i = 0; i < indent; i++) fprintf(out, " ");
                }
                for (int vi = 0; vi < s->v.input.var_count; vi++) {
                    AstExpr *var = s->v.input.vars[vi];
                    emit_py3_expr(out, var);
                    if (expr_is_string(var)) {
                        fprintf(out, " = bpp_input_str()\n");
                    } else {
                        fprintf(out, " = bpp_input_num()\n");
                    }
                    if (vi < s->v.input.var_count - 1) {
                        for (int i = 0; i < indent; i++) fprintf(out, " ");
                    }
                }
                break;
            }
            case STMT_DIM:
                emit_py3_named_var(out, s->v.dim.name);
                fprintf(out, " = ");
                if (s->v.dim.name[strlen(s->v.dim.name)-1] == '$') {
                    if (s->v.dim.dim2) {
                        fprintf(out, "[[\"\"] * (int(");
                        emit_py3_expr(out, s->v.dim.dim2);
                        fprintf(out, ") + 1) for _ in range(int(");
                        emit_py3_expr(out, s->v.dim.dim1);
                        fprintf(out, ") + 1)]\n");
                    } else {
                        fprintf(out, "[\"\"] * (int(");
                        emit_py3_expr(out, s->v.dim.dim1);
                        fprintf(out, ") + 1)\n");
                    }
                } else {
                    if (s->v.dim.dim2) {
                        fprintf(out, "[[0.0] * (int(");
                        emit_py3_expr(out, s->v.dim.dim2);
                        fprintf(out, ") + 1) for _ in range(int(");
                        emit_py3_expr(out, s->v.dim.dim1);
                        fprintf(out, ") + 1)]\n");
                    } else {
                        fprintf(out, "[0.0] * (int(");
                        emit_py3_expr(out, s->v.dim.dim1);
                        fprintf(out, ") + 1)\n");
                    }
                }
                break;
            case STMT_DATA:
                fprintf(out, "pass # DATA gathered globally\n");
                break;
            case STMT_READ: {
                for (int vi = 0; vi < s->v.read.var_count; vi++) {
                    if (vi > 0) {
                        for (int i = 0; i < indent; i++) fprintf(out, " ");
                    }
                    if (s->v.read.var_types[vi] == 0) {
                        // Simple numeric variable
                        emit_py3_var_name(out, s->v.read.var_names[vi], 0);
                        fprintf(out, " = bpp_read_num()\n");
                    } else if (s->v.read.var_types[vi] == 2) {
                        // 1D Array element
                        const char *dn = s->v.read.dim_names[vi];
                        if (dn[0] == '\0') {
                            char nm[2] = { s->v.read.var_names[vi], '\0' };
                            emit_py3_named_var(out, nm);
                        } else {
                            emit_py3_named_var(out, dn);
                        }
                        fprintf(out, "[int(");
                        emit_py3_expr(out, s->v.read.var_indices[vi]);
                        if (dn[0] && dn[strlen(dn)-1] == '$') {
                            fprintf(out, ")] = bpp_read_str()\n");
                        } else {
                            fprintf(out, ")] = bpp_read_num()\n");
                        }
                    } else if (s->v.read.var_types[vi] == 3) {
                        // 2D Array element
                        const char *dn = s->v.read.dim_names[vi];
                        if (dn[0] == '\0') {
                            char nm[2] = { s->v.read.var_names[vi], '\0' };
                            emit_py3_named_var(out, nm);
                        } else {
                            emit_py3_named_var(out, dn);
                        }
                        fprintf(out, "[int(");
                        emit_py3_expr(out, s->v.read.var_indices[vi]);
                        fprintf(out, ")][int(");
                        emit_py3_expr(out, s->v.read.var_indices2[vi]);
                        if (dn[0] && dn[strlen(dn)-1] == '$') {
                            fprintf(out, "))] = bpp_read_str()\n");
                        } else {
                            fprintf(out, "))] = bpp_read_num()\n");
                        }
                    } else {
                        // Simple string variable
                        emit_py3_var_name(out, s->v.read.var_names[vi], 1);
                        fprintf(out, " = bpp_read_str()\n");
                    }
                }
                break;
            }
            case STMT_RESTORE:
                fprintf(out, "bpp_restore()\n");
                break;
            case STMT_END:
            case STMT_STOP:
                fprintf(out, "sys.exit(0)\n");
                break;
            case STMT_REM:
                fprintf(out, "# %s\n", s->v.rem.text ? s->v.rem.text : "");
                break;
            case STMT_ON_GOTO: {
                fprintf(out, "on_idx = int(");
                emit_py3_expr(out, s->v.on_goto.selector);
                fprintf(out, ") - 1\n");
                for (int i = 0; i < indent; i++) fprintf(out, " ");
                fprintf(out, "on_targets = [");
                for (int ti = 0; ti < s->v.on_goto.target_count; ti++) {
                    fprintf(out, "%.0f", s->v.on_goto.targets[ti]);
                    if (ti < s->v.on_goto.target_count - 1) fprintf(out, ", ");
                }
                fprintf(out, "]\n");
                for (int i = 0; i < indent; i++) fprintf(out, " ");
                fprintf(out, "if 0 <= on_idx < len(on_targets):\n");
                for (int i = 0; i < indent + 4; i++) fprintf(out, " ");
                if (s->v.on_goto.is_gosub) {
                    double next_line = 0.0;
                    if (current_idx + 1 < line_count) {
                        next_line = lines[current_idx + 1].line_number;
                    }
                    fprintf(out, "gosub_stack.append(%.0f)\n", next_line);
                    for (int i = 0; i < indent + 4; i++) fprintf(out, " ");
                }
                fprintf(out, "state = on_targets[on_idx]; continue\n");
                break;
            }
            default:
                fprintf(out, "pass\n");
                break;
        }
        s = s->next;
    }
}

// Global DATA statements collector
static void emit_global_data(FILE *out, ProgramStore *program) {
    fprintf(out, "# === DATA statements ===\n");
    fprintf(out, "bpp_data_pool = [");
    int first = 1;
    if (program) {
        for (int i = 0; i < program->count; i++) {
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

                // Escape characters for Python double quotes
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

                if (!first) fprintf(out, ", ");
                first = 0;
                fprintf(out, "\"%s\"", escaped);

                while (*p == ' ' || *p == '\t') p++;
                if (*p == ',') {
                    p++;
                    continue;
                }
                break;
            }
        }
    }
    fprintf(out, "]\n\n");
}

int trans_emit_py3(FILE *out, AstLine *lines, int line_count, ProgramStore *program, const TargetConfig *target)
{
    (void)target;
    PyForSite for_sites[MAX_FOR_SITES];
    int for_count = 0;
    py_find_for_sites(lines, line_count, for_sites, &for_count);
    fprintf(out, "#!/usr/bin/env python3\n");
    fprintf(out,
        "# =====================================================================\n"
        "# BASIC++ AUTO-GENERATED PYTHON 3 SOURCE FILE\n"
        "# =====================================================================\n"
        "# 1. SUBSYSTEMS EXPLAINED:\n"
        "#    - Runtime Helpers: Mock functions for printing, input routines, stack operations,\n"
        "#      and array indexing using standard Python built-ins.\n"
        "#    - Global Variables: Scalar and array states tracked within function/global scope.\n"
        "#    - Control Flow: A state machine utilizing a while-loop and match/case block\n"
        "#      to emulate BASIC's arbitrary GOTO line jumping.\n"
        "#\n"
        "# 2. PORTABILITY CONCERNS:\n"
        "#    - Pure Python 3 compatible, using only core built-ins and standard sys/random/math modules.\n"
        "#    - Runs on any operating system supporting Python 3.x interpreter.\n"
        "#\n"
        "# 3. MEMORY MANAGEMENT:\n"
        "#    - Uses Python's native automatic garbage collection and dynamic object references.\n"
        "#    - Pre-allocated list structures model fixed-size legacy BASIC memory structures.\n"
        "#\n"
        "# 4. PARSER & LEXER BEHAVIOR:\n"
        "#    - Walks transpiler AST node hierarchy and outputs corresponding Python statements.\n"
        "#    - Correctly handles semantic differences like standard division and array sizing.\n"
        "#\n"
        "# 5. RUNTIME BEHAVIOR:\n"
        "#    - Runs sequentially via the GOTO state loop, modifying the 'state' variable to branch.\n"
        "#    - Errors trigger standard Python tracebacks unless intercepted.\n"
        "#\n"
        "# 6. FUTURE EXPANSION POINTS:\n"
        "#    - Can be extended to support custom Python wrappers around graphical/serial peripherals.\n"
        "#    - New target configuration mappings can map BASIC functions directly to pip modules.\n"
        "#\n"
        "# 7. WHAT CAN BE CHANGED:\n"
        "#    - Helper functions (bpp_print_val, etc.) or default print width constants.\n"
        "#    - Insertion of arbitrary Python logic or external package imports.\n"
        "#\n"
        "# 8. WHAT CANNOT BE CHANGED:\n"
        "#    - GOTO emulation structure utilizing the 'state' loop and match/case structure.\n"
        "#    - Variable naming conventions mapped from BASIC variables to lowercase strings.\n"
        "#\n"
        "# 9. WHAT TO EXPECT:\n"
        "#    - Consistent script execution conforming exactly to standard Python 3 interpreter behavior.\n"
        "#\n"
        "# 10. TROUBLESHOOTING & FAILURE MODES:\n"
        "#     - Indentation errors: Ensure correct spaces/tabs when editing the transpiled code.\n"
        "#     - ValueError/TypeError: Check if string/numeric variable types are mixed incorrectly.\n"
        "# =====================================================================\n\n");
    fprintf(out, "# Auto-generated by BASIC++ Transpiler (Python 3 target)\n");
    fprintf(out, "import sys\nimport math\nimport random\nimport time\n\n");

    fprintf(out, "# === Runtime Helpers ===\n");
    fprintf(out, "bpp_print_width = 14\n");
    fprintf(out, "bpp_data_ptr = 0\n");
    fprintf(out, "gosub_stack = []\n");
    fprintf(out, "at_array = [0.0] * 4096\n\n");

    fprintf(out, "def bpp_print_val(val, is_string=False):\n");
    fprintf(out, "    if is_string:\n");
    fprintf(out, "        sys.stdout.write(str(val))\n");
    fprintf(out, "    else:\n");
    fprintf(out, "        try:\n");
    fprintf(out, "            f = float(val)\n");
    fprintf(out, "            if f.is_integer():\n");
    fprintf(out, "                sys.stdout.write(f\"{int(f):>{bpp_print_width}}\")\n");
    fprintf(out, "            else:\n");
    fprintf(out, "                sys.stdout.write(f\"{f:>{bpp_print_width}g}\")\n");
    fprintf(out, "        except ValueError:\n");
    fprintf(out, "            sys.stdout.write(f\"{str(val):>{bpp_print_width}}\")\n");
    fprintf(out, "    sys.stdout.flush()\n\n");

    fprintf(out, "def bpp_print_tab():\n");
    fprintf(out, "    sys.stdout.write(\" \" * bpp_print_width)\n");
    fprintf(out, "    sys.stdout.flush()\n\n");

    fprintf(out, "def bpp_print_nl():\n");
    fprintf(out, "    sys.stdout.write(\"\\n\")\n");
    fprintf(out, "    sys.stdout.flush()\n\n");

    fprintf(out, "def bpp_input_num():\n");
    fprintf(out, "    while True:\n");
    fprintf(out, "        line = sys.stdin.readline()\n");
    fprintf(out, "        if not line:\n");
    fprintf(out, "            sys.exit(0)\n");
    fprintf(out, "        try:\n");
    fprintf(out, "            return float(line.strip())\n");
    fprintf(out, "        except ValueError:\n");
    fprintf(out, "            print(\"?Redo from start\")\n\n");

    fprintf(out, "def bpp_input_str():\n");
    fprintf(out, "    line = sys.stdin.readline()\n");
    fprintf(out, "    if not line:\n");
    fprintf(out, "        sys.exit(0)\n");
    fprintf(out, "    return line.strip('\\r\\n')\n\n");

    fprintf(out, "def bpp_rnd(arg):\n");
    fprintf(out, "    if arg <= 0:\n");
    fprintf(out, "        return random.random()\n");
    fprintf(out, "    else:\n");
    fprintf(out, "        return random.randint(1, int(arg))\n\n");

    fprintf(out, "def bpp_sgn(arg):\n");
    fprintf(out, "    return 1.0 if arg > 0 else (-1.0 if arg < 0 else 0.0)\n\n");

    fprintf(out, "def bpp_asc(arg):\n");
    fprintf(out, "    return ord(arg[0]) if arg else 0\n\n");

    fprintf(out, "def bpp_val(arg):\n");
    fprintf(out, "    try:\n");
    fprintf(out, "        return float(arg)\n");
    fprintf(out, "    except ValueError:\n");
    fprintf(out, "        return 0.0\n\n");

    fprintf(out, "def bpp_str(arg):\n");
    fprintf(out, "    return str(arg)\n\n");

    fprintf(out, "def bpp_left(s, n):\n");
    fprintf(out, "    return s[:max(0, int(n))]\n\n");

    fprintf(out, "def bpp_right(s, n):\n");
    fprintf(out, "    n_val = max(0, int(n))\n");
    fprintf(out, "    return s[-n_val:] if n_val > 0 else \"\"\n\n");

    fprintf(out, "def bpp_mid(s, start, length=None):\n");
    fprintf(out, "    st = max(0, int(start) - 1)\n");
    fprintf(out, "    if length is None:\n");
    fprintf(out, "        return s[st:]\n");
    fprintf(out, "    return s[st:st + max(0, int(length))]\n\n");

    fprintf(out, "def bpp_tab(n):\n");
    fprintf(out, "    sys.stdout.write(\" \" * max(0, int(n)))\n");
    fprintf(out, "    sys.stdout.flush()\n");
    fprintf(out, "    return \"\"\n\n");

    fprintf(out, "def bpp_read_num():\n");
    fprintf(out, "    global bpp_data_ptr\n");
    fprintf(out, "    if bpp_data_ptr >= len(bpp_data_pool):\n");
    fprintf(out, "        print(\"OUT OF DATA\", file=sys.stderr)\n");
    fprintf(out, "        sys.exit(1)\n");
    fprintf(out, "    val = bpp_data_pool[bpp_data_ptr]\n");
    fprintf(out, "    bpp_data_ptr += 1\n");
    fprintf(out, "    try:\n");
    fprintf(out, "        return float(val)\n");
    fprintf(out, "    except ValueError:\n");
    fprintf(out, "        return 0.0\n\n");

    fprintf(out, "def bpp_read_str():\n");
    fprintf(out, "    global bpp_data_ptr\n");
    fprintf(out, "    if bpp_data_ptr >= len(bpp_data_pool):\n");
    fprintf(out, "        print(\"OUT OF DATA\", file=sys.stderr)\n");
    fprintf(out, "        sys.exit(1)\n");
    fprintf(out, "    val = bpp_data_pool[bpp_data_ptr]\n");
    fprintf(out, "    bpp_data_ptr += 1\n");
    fprintf(out, "    return str(val)\n\n");

    fprintf(out, "def bpp_restore():\n");
    fprintf(out, "    global bpp_data_ptr\n");
    fprintf(out, "    bpp_data_ptr = 0\n\n");

    // Collect and declare all variables used in the program
    VarNode *var_head = NULL;
    for (int i = 0; i < line_count; i++) {
        collect_stmt_vars(lines[i].stmts, &var_head);
    }
    
    fprintf(out, "# === Variable Initializations ===\n");
    VarNode *curr = var_head;
    while (curr) {
        if (curr->is_str) {
            fprintf(out, "%s = \"\"\n", curr->name);
        } else {
            fprintf(out, "%s = 0.0\n", curr->name);
        }
        VarNode *next = curr->next;
        free(curr);
        curr = next;
    }
    fprintf(out, "\n");

    fprintf(out, "# === Loop Control Flags ===\n");
    for (int fi = 0; fi < for_count; fi++) {
        fprintf(out, "loop_jumped_%d = False\n", for_sites[fi].loop_id);
    }
    fprintf(out, "\n");

    // Emit global DATA statements
    emit_global_data(out, program);

    // Emit user functions
    for (int i = 0; i < line_count; i++) {
        AstStmt *s = lines[i].stmts;
        while (s) {
            if (s->type == STMT_DEF_FN) {
                fprintf(out, "def bpp_fn_%c(", tolower((unsigned char)s->v.def_fn.func_name));
                fprintf(out, "%c):\n", tolower((unsigned char)s->v.def_fn.param_name));
                fprintf(out, "    return ");
                emit_py3_expr(out, s->v.def_fn.body);
                fprintf(out, "\n\n");
            }
            s = s->next;
        }
    }

    if (line_count > 0) {
        fprintf(out, "state = %.0f\n", lines[0].line_number);
    } else {
        fprintf(out, "state = 0\n");
    }

    fprintf(out, "while True:\n");
    fprintf(out, "    match state:\n");

    for (int i = 0; i < line_count; i++) {
        fprintf(out, "        case %.0f:\n", lines[i].line_number);
        emit_py3_stmt(out, lines[i].stmts, 12, lines, line_count, i, for_sites, for_count);
        // If not a jump or stop, fallthrough to the next sequential line
        int is_jump = 0;
        AstStmt *s = lines[i].stmts;
        while (s) {
            if (s->type == STMT_GOTO || s->type == STMT_GOSUB || s->type == STMT_RETURN || 
                s->type == STMT_END || s->type == STMT_STOP || s->type == STMT_ON_GOTO) {
                is_jump = 1;
            }
            s = s->next;
        }
        if (!is_jump) {
            if (i + 1 < line_count) {
                fprintf(out, "            state = %.0f\n", lines[i + 1].line_number);
            } else {
                fprintf(out, "            sys.exit(0)\n");
            }
        }
    }

    fprintf(out, "        case _:\n");
    fprintf(out, "            print(f\"UNDEF LINE {state}\", file=sys.stderr)\n");
    fprintf(out, "            sys.exit(1)\n");

    return 0;
}
