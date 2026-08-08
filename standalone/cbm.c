#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#else
#define _POSIX_C_SOURCE 199309L
#include <unistd.h>
#endif

/* -- Lexer Definitions & Constants -- */
enum Token {
    TOK_END = 128, TOK_FOR, TOK_NEXT, TOK_DATA, TOK_INPUT, TOK_DIM, TOK_READ,
    TOK_LET, TOK_GOTO, TOK_RUN, TOK_IF, TOK_RESTORE, TOK_GOSUB, TOK_RETURN,
    TOK_REM, TOK_STOP, TOK_ON, TOK_WAIT, TOK_LOAD, TOK_SAVE, TOK_VERIFY,
    TOK_DEF, TOK_POKE, TOK_PRINT, TOK_CONT, TOK_LIST, TOK_CLR, TOK_CMD,
    TOK_SYS, TOK_OPEN, TOK_CLOSE, TOK_GET, TOK_NEW, TOK_TAB, TOK_TO,
    TOK_FN, TOK_SPC, TOK_THEN, TOK_NOT, TOK_STEP, TOK_PLUS, TOK_MINUS,
    TOK_MUL, TOK_DIV, TOK_POW, TOK_AND, TOK_OR, TOK_GT, TOK_EQ, TOK_LT,
    TOK_SGN, TOK_INT, TOK_ABS, TOK_USR, TOK_FRE, TOK_POS, TOK_SQR, TOK_RND,
    TOK_LOG, TOK_EXP, TOK_COS, TOK_SIN, TOK_TAN, TOK_ATN, TOK_PEEK, TOK_LEN,
    TOK_STR, TOK_VAL, TOK_ASC, TOK_CHR, TOK_LEFT, TOK_RIGHT, TOK_MID,
    TOK_GO, TOK_SLEEP
};

const char *keywords[] = {
    "END", "FOR", "NEXT", "DATA", "INPUT", "DIM", "READ", "LET", "GOTO", "RUN", "IF", "RESTORE",
    "GOSUB", "RETURN", "REM", "STOP", "ON", "WAIT", "LOAD", "SAVE", "VERIFY", "DEF", "POKE",
    "PRINT", "CONT", "LIST", "CLR", "CMD", "SYS", "OPEN", "CLOSE", "GET", "NEW", "TAB", "TO",
    "FN", "SPC", "THEN", "NOT", "STEP", "+", "-", "*", "/", "^", "AND", "OR", ">", "=", "<",
    "SGN", "INT", "ABS", "USR", "FRE", "POS", "SQR", "RND", "LOG", "EXP", "COS", "SIN", "TAN",
    "ATN", "PEEK", "LEN", "STR$", "VAL", "ASC", "CHR$", "LEFT$", "RIGHT$", "MID$", "GO", "SLEEP",
    NULL
};

/* -- Data Structures -- */
typedef struct Value {
    int type; /* 0 = num, 1 = str */
    double num;
    char *str;
} Value;

typedef struct Var {
    char name[4];
    Value val;
    struct Var *next;
} Var;

typedef struct Line {
    int num;
    char *text;
    struct Line *next;
} Line;

typedef struct ForLoop {
    char var_name[4];
    double to_val;
    double step_val;
    Line *line;
    char *ptr;
    struct ForLoop *next;
} ForLoop;

typedef struct Gosub {
    Line *line;
    char *ptr;
    struct Gosub *next;
} Gosub;

/* -- Globals -- */
Line *prog = NULL;
Var *vars = NULL;
ForLoop *fors = NULL;
Gosub *gosubs = NULL;
Line *curr_line = NULL;
char *curr_ptr = NULL;
int end_prog = 0;

/* -- Utilities -- */
char* my_strdup(const char *s) {
    char *d;
    if (!s) return NULL;
    d = (char*)malloc(strlen(s) + 1);
    if (d) strcpy(d, s);
    return d;
}

int my_strncasecmp(const char *s1, const char *s2, int n) {
    int i;
    int c1;
    int c2;
    for (i = 0; i < n; i++) {
        c1 = toupper((unsigned char)s1[i]);
        c2 = toupper((unsigned char)s2[i]);
        if (c1 != c2) return c1 - c2;
        if (c1 == '\0') return 0;
    }
    return 0;
}

char* tokenize(const char *in) {
    char *out;
    int i, j, k;
    int matched, in_str, len;
    
    out = (char*)malloc(strlen(in) + 1);
    i = 0; 
    j = 0; 
    in_str = 0;
    
    while (in[i]) {
        if (in[i] == '"') {
            in_str = !in_str;
            out[j++] = in[i++];
            continue;
        }
        if (in_str) {
            out[j++] = in[i++];
            continue;
        }
        if (in[i] == '?') {
            out[j++] = (char)TOK_PRINT;
            i++;
            continue;
        }
        matched = 0;
        for (k = 0; keywords[k]; k++) {
            len = strlen(keywords[k]);
            if (my_strncasecmp(in + i, keywords[k], len) == 0) {
                out[j++] = (char)(128 + k);
                i += len;
                matched = 1;
                /* If REM or DATA, bypass space restrictions safely */
                if (128 + k == TOK_REM || 128 + k == TOK_DATA) {
                    while (in[i]) out[j++] = in[i++];
                }
                break;
            }
        }
        if (!matched) {
            out[j++] = toupper((unsigned char)in[i++]);
        }
    }
    out[j] = '\0';
    return out;
}

void read_var(char **p, char *out) {
    int i;
    char *q;
    
    i = 0;
    q = *p;
    while (*q == ' ') q++;
    if (*q >= 'A' && *q <= 'Z') {
        out[i++] = *q++;
        while (*q == ' ') q++;
        if ((*q >= 'A' && *q <= 'Z') || (*q >= '0' && *q <= '9')) {
            out[i++] = *q++;
        }
        /* Bypass truncated unneeded extended variable chars cleanly */
        while (1) {
            while (*q == ' ') q++;
            if ((*q >= 'A' && *q <= 'Z') || (*q >= '0' && *q <= '9')) {
                q++;
            } else {
                break;
            }
        }
        if (*q == '$' || *q == '%') {
            out[i++] = *q++;
        }
    }
    out[i] = '\0';
    *p = q;
}

Value get_var(const char *name) {
    Var *v;
    Value res;
    
    v = vars;
    while (v) {
        if (strcmp(v->name, name) == 0) {
            res.type = v->val.type;
            res.num = v->val.num;
            res.str = (res.type == 1 && v->val.str) ? my_strdup(v->val.str) : NULL;
            return res;
        }
        v = v->next;
    }
    res.type = strchr(name, '$') ? 1 : 0;
    res.num = 0.0;
    res.str = res.type ? my_strdup("") : NULL;
    return res;
}

void set_var(const char *name, Value val) {
    Var *v;
    v = vars;
    while (v) {
        if (strcmp(v->name, name) == 0) {
            if (v->val.type == 1 && v->val.str) free(v->val.str);
            v->val.type = val.type;
            v->val.num = strchr(name, '%') ? (int)val.num : val.num;
            v->val.str = (val.type == 1 && val.str) ? my_strdup(val.str) : NULL;
            return;
        }
        v = v->next;
    }
    v = (Var*)malloc(sizeof(Var));
    strcpy(v->name, name);
    v->val.type = val.type;
    v->val.num = strchr(name, '%') ? (int)val.num : val.num;
    v->val.str = (val.type == 1 && val.str) ? my_strdup(val.str) : NULL;
    v->next = vars;
    vars = v;
}

void add_line(int num, const char *text) {
    Line *nl;
    Line *curr;
    Line *prev;
    
    nl = (Line *)malloc(sizeof(Line));
    nl->num = num;
    nl->text = tokenize(text);
    nl->next = NULL;
    
    if (!prog || prog->num > num) {
        nl->next = prog;
        prog = nl;
        return;
    }
    
    curr = prog;
    prev = NULL;
    while (curr && curr->num <= num) {
        if (curr->num == num) {
            free(curr->text);
            curr->text = nl->text;
            free(nl);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    prev->next = nl;
    nl->next = curr;
}

/* -- Recursive Descent Parsers -- */
Value eval_expr(char **p);
Value eval_pow(char **p);
Value eval_unary(char **p);
Value eval_factor(char **p);

Value eval_factor(char **p) {
    Value res, arg1, arg2, arg3;
    char *q;
    char *start;
    char buf[32];
    char vname[4];
    int len, tok, n, start_pos, take;
    
    res.type = 0; res.num = 0; res.str = NULL;
    q = *p;
    while (*q == ' ') q++;
    
    if (*q == '(') {
        q++;
        res = eval_expr(&q);
        while (*q == ' ') q++;
        if (*q == ')') q++;
        *p = q;
        return res;
    }
    if (*q == '"') {
        q++;
        start = q;
        while (*q && *q != '"') q++;
        len = q - start;
        res.type = 1;
        res.str = (char*)malloc(len + 1);
        strncpy(res.str, start, len);
        res.str[len] = '\0';
        if (*q == '"') q++;
        *p = q;
        return res;
    }
    
    /* Standard and scientific notation parsing handled inherently by C strtod */
    if (isdigit((unsigned char)*q) || *q == '.') {
        res.type = 0;
        res.num = strtod(q, &q);
        *p = q;
        return res;
    }
    
    if ((unsigned char)*q >= 128) {
        tok = (unsigned char)*q;
        q++;
        
        if (tok == TOK_CHR || tok == TOK_STR || tok == TOK_LEN || tok == TOK_INT || 
            tok == TOK_RND || tok == TOK_VAL || tok == TOK_ASC || tok == TOK_ABS || 
            tok == TOK_SQR || tok == TOK_SIN || tok == TOK_COS || tok == TOK_TAN || 
            tok == TOK_ATN || tok == TOK_EXP || tok == TOK_LOG || tok == TOK_SGN) {
            
            while (*q == ' ') q++;
            if (*q == '(') q++;
            arg1 = eval_expr(&q);
            while (*q == ' ') q++;
            if (*q == ')') q++;
            
            if (tok == TOK_CHR) {
                res.type = 1;
                res.str = (char*)malloc(2);
                res.str[0] = (char)(int)arg1.num;
                res.str[1] = '\0';
            } else if (tok == TOK_STR) {
                res.type = 1;
                sprintf(buf, "%g", arg1.num);
                res.str = my_strdup(buf);
            } else if (tok == TOK_LEN) {
                res.type = 0;
                res.num = (arg1.type == 1 && arg1.str) ? strlen(arg1.str) : 0;
            } else if (tok == TOK_INT) {
                res.type = 0;
                res.num = floor(arg1.num); /* CBM Native behavior maps to floor() */
            } else if (tok == TOK_RND) {
                res.type = 0;
                if (arg1.num < 0) srand((unsigned int)(-arg1.num));
                res.num = (double)rand() / ((double)RAND_MAX + 1.0); /* Guaranteed < 1.0 */
            } else if (tok == TOK_VAL) {
                res.type = 0;
                res.num = (arg1.type == 1 && arg1.str) ? atof(arg1.str) : 0;
            } else if (tok == TOK_ASC) {
                res.type = 0;
                res.num = (arg1.type == 1 && arg1.str && arg1.str[0]) ? (unsigned char)arg1.str[0] : 0;
            } else if (tok == TOK_ABS) {
                res.type = 0;
                res.num = fabs(arg1.num);
            } else if (tok == TOK_SQR) {
                res.type = 0;
                if (arg1.num < 0.0) {
                    printf("\n?ILLEGAL QUANTITY ERROR IN %d\n", curr_line ? curr_line->num : 0);
                    exit(1);
                }
                res.num = sqrt(arg1.num);
                
            /* -- Properly Added Execution logic for Floating Point Functions -- */
            } else if (tok == TOK_SIN) {
                res.type = 0;
                res.num = sin(arg1.num);
            } else if (tok == TOK_COS) {
                res.type = 0;
                res.num = cos(arg1.num);
            } else if (tok == TOK_TAN) {
                res.type = 0;
                res.num = tan(arg1.num);
            } else if (tok == TOK_ATN) {
                res.type = 0;
                res.num = atan(arg1.num);
            } else if (tok == TOK_EXP) {
                res.type = 0;
                res.num = exp(arg1.num);
            } else if (tok == TOK_LOG) {
                res.type = 0;
                if (arg1.num <= 0.0) {
                    printf("\n?ILLEGAL QUANTITY ERROR IN %d\n", curr_line ? curr_line->num : 0);
                    exit(1);
                }
                res.num = log(arg1.num); /* C Standard log() maps precisely to natural logarithm */
            } else if (tok == TOK_SGN) {
                res.type = 0;
                res.num = (arg1.num > 0.0) ? 1.0 : ((arg1.num < 0.0) ? -1.0 : 0.0);
            }
            
            if (arg1.type == 1 && arg1.str) free(arg1.str);
            *p = q;
            return res;
        }
        
        if (tok == TOK_LEFT || tok == TOK_RIGHT || tok == TOK_MID) {
            while (*q == ' ') q++;
            if (*q == '(') q++;
            arg1 = eval_expr(&q);
            while (*q == ' ') q++;
            if (*q == ',') q++;
            arg2 = eval_expr(&q);
            arg3.type = 0; arg3.num = -1; arg3.str = NULL;
            while (*q == ' ') q++;
            if (tok == TOK_MID && *q == ',') {
                q++;
                arg3 = eval_expr(&q);
            }
            while (*q == ' ') q++;
            if (*q == ')') q++;
            
            res.type = 1;
            res.str = my_strdup("");
            if (arg1.type == 1 && arg1.str) {
                len = strlen(arg1.str);
                n = (int)arg2.num;
                if (n < 0) n = 0;
                free(res.str);
                if (tok == TOK_LEFT) {
                    if (n > len) n = len;
                    res.str = (char*)malloc(n + 1);
                    strncpy(res.str, arg1.str, n);
                    res.str[n] = '\0';
                } else if (tok == TOK_RIGHT) {
                    if (n > len) n = len;
                    res.str = (char*)malloc(n + 1);
                    strncpy(res.str, arg1.str + len - n, n);
                    res.str[n] = '\0';
                } else if (tok == TOK_MID) {
                    start_pos = n - 1;
                    take = (arg3.num >= 0) ? (int)arg3.num : len;
                    if (start_pos < 0) start_pos = 0;
                    if (start_pos > len) start_pos = len;
                    if (take < 0) take = 0;
                    if (start_pos + take > len) take = len - start_pos;
                    res.str = (char*)malloc(take + 1);
                    strncpy(res.str, arg1.str + start_pos, take);
                    res.str[take] = '\0';
                }
            }
            if (arg1.type == 1 && arg1.str) free(arg1.str);
            if (arg2.type == 1 && arg2.str) free(arg2.str);
            if (arg3.type == 1 && arg3.str) free(arg3.str);
            *p = q;
            return res;
        }
    }
    
    if (*q >= 'A' && *q <= 'Z') {
        read_var(&q, vname);
        res = get_var(vname);
        *p = q;
        return res;
    }
    
    return res;
}

/* Reordered AST precedence: Power evaluations happen PRIOR to Negations */
Value eval_pow(char **p) {
    Value a, b;
    char *q;
    
    a = eval_factor(p);
    q = *p;
    while (*q == ' ') q++;
    while ((unsigned char)*q == TOK_POW || *q == '^') {
        q++;
        b = eval_unary(&q);
        a.num = pow(a.num, b.num);
        while (*q == ' ') q++;
    }
    *p = q;
    return a;
}

Value eval_unary(char **p) {
    Value res;
    char *q;
    
    q = *p;
    while (*q == ' ') q++;
    if ((unsigned char)*q == TOK_MINUS || *q == '-') {
        q++;
        res = eval_unary(&q);
        res.num = -res.num;
        *p = q;
        return res;
    }
    if ((unsigned char)*q == TOK_PLUS || *q == '+') {
        q++;
        res = eval_unary(&q);
        *p = q;
        return res;
    }
    /* Fixed Operator Precedence to match CBM Basic: Unary ties down into Power */
    return eval_pow(p);
}

Value eval_muldiv(char **p) {
    Value a, b;
    char *q;
    int op;
    
    a = eval_unary(p);
    q = *p;
    while (*q == ' ') q++;
    while ((unsigned char)*q == TOK_MUL || *q == '*' || (unsigned char)*q == TOK_DIV || *q == '/') {
        op = (unsigned char)*q;
        q++;
        b = eval_unary(&q);
        if (op == TOK_MUL || op == '*') {
            a.num *= b.num;
        } else {
            if (b.num == 0) {
                printf("\n?DIVISION BY ZERO ERROR IN %d\n", curr_line ? curr_line->num : 0);
                exit(1);
            }
            a.num /= b.num;
        }
        while (*q == ' ') q++;
    }
    *p = q;
    return a;
}

Value eval_addsub(char **p) {
    Value a, b;
    char *q;
    int op;
    char *ns;
    
    a = eval_muldiv(p);
    q = *p;
    while (*q == ' ') q++;
    while ((unsigned char)*q == TOK_PLUS || *q == '+' || (unsigned char)*q == TOK_MINUS || *q == '-') {
        op = (unsigned char)*q;
        q++;
        b = eval_muldiv(&q);
        
        if (op == TOK_PLUS || op == '+') {
            if (a.type == 1 && b.type == 1) {
                ns = (char*)malloc((a.str ? strlen(a.str) : 0) + (b.str ? strlen(b.str) : 0) + 1);
                ns[0] = '\0';
                if (a.str) strcat(ns, a.str);
                if (b.str) strcat(ns, b.str);
                if (a.str) free(a.str);
                if (b.str) free(b.str);
                a.str = ns;
            } else {
                a.num += b.num;
            }
        } else {
            a.num -= b.num;
        }
        while (*q == ' ') q++;
    }
    *p = q;
    return a;
}

Value eval_rel(char **p) {
    Value a, b;
    char *q;
    int op, truth, cmp;
    
    a = eval_addsub(p);
    q = *p;
    while (*q == ' ') q++;
    
    while (1) {
        op = 0;
        truth = 0;
        if ((unsigned char)*q == TOK_EQ || *q == '=') { op = 1; q++; }
        else if ((unsigned char)*q == TOK_LT || *q == '<') {
            q++;
            if ((unsigned char)*q == TOK_GT || *q == '>') { op = 6; q++; }
            else if ((unsigned char)*q == TOK_EQ || *q == '=') { op = 4; q++; }
            else op = 2;
        } else if ((unsigned char)*q == TOK_GT || *q == '>') {
            q++;
            if ((unsigned char)*q == TOK_LT || *q == '<') { op = 6; q++; }
            else if ((unsigned char)*q == TOK_EQ || *q == '=') { op = 5; q++; }
            else op = 3;
        } else break;
        
        b = eval_addsub(&q);
        if (a.type == 1 && b.type == 1) {
            cmp = strcmp(a.str ? a.str : "", b.str ? b.str : "");
            if (op == 1 && cmp == 0) truth = 1;
            if (op == 2 && cmp < 0) truth = 1;
            if (op == 3 && cmp > 0) truth = 1;
            if (op == 4 && cmp <= 0) truth = 1;
            if (op == 5 && cmp >= 0) truth = 1;
            if (op == 6 && cmp != 0) truth = 1;
        } else {
            if (op == 1 && a.num == b.num) truth = 1;
            if (op == 2 && a.num < b.num) truth = 1;
            if (op == 3 && a.num > b.num) truth = 1;
            if (op == 4 && a.num <= b.num) truth = 1;
            if (op == 5 && a.num >= b.num) truth = 1;
            if (op == 6 && a.num != b.num) truth = 1;
        }
        
        if (a.type == 1 && a.str) { free(a.str); a.str = NULL; a.type = 0; }
        if (b.type == 1 && b.str) { free(b.str); }
        /* CBM Native Evaluation Logic Bounds (True equates to -1) */
        a.num = truth ? -1.0 : 0.0;
        while (*q == ' ') q++;
    }
    *p = q;
    return a;
}

Value eval_not(char **p) {
    Value res;
    char *q;
    
    q = *p;
    while (*q == ' ') q++;
    if ((unsigned char)*q == TOK_NOT) {
        q++;
        res = eval_rel(&q);
        res.num = (double)(~(int)res.num);
        *p = q;
        return res;
    }
    return eval_rel(p);
}

Value eval_and(char **p) {
    Value a, b;
    char *q;
    
    a = eval_not(p);
    q = *p;
    while (*q == ' ') q++;
    while ((unsigned char)*q == TOK_AND) {
        q++;
        b = eval_not(&q);
        a.num = (double)((int)a.num & (int)b.num);
        while (*q == ' ') q++;
    }
    *p = q;
    return a;
}

Value eval_expr(char **p) {
    Value a, b;
    char *q;
    
    a = eval_and(p);
    q = *p;
    while (*q == ' ') q++;
    while ((unsigned char)*q == TOK_OR) {
        q++;
        b = eval_and(&q);
        a.num = (double)((int)a.num | (int)b.num);
        while (*q == ' ') q++;
    }
    *p = q;
    return a;
}

/* -- Execution Instruction Engine -- */
void execute_statement(void) {
    int tok;
    
    while (*curr_ptr == ' ') curr_ptr++;
    if (!*curr_ptr || *curr_ptr == ':') return;
    
    tok = (unsigned char)*curr_ptr;
    
    if (tok == TOK_REM || tok == TOK_DATA) {
        while (*curr_ptr) curr_ptr++;
        return;
    }
    if (tok == TOK_DIM) {
        while (*curr_ptr && *curr_ptr != ':') curr_ptr++;
        return;
    }
    
    if (tok == TOK_PRINT) {
        int newline, i, n;
        Value arg, val;
        
        newline = 1;
        curr_ptr++;
        
        while (*curr_ptr && *curr_ptr != ':') {
            while (*curr_ptr == ' ') curr_ptr++;
            if (!*curr_ptr || *curr_ptr == ':') break;
            if (*curr_ptr == ';' || *curr_ptr == ',') {
                if (*curr_ptr == ',') printf("\t");
                newline = 0;
                curr_ptr++;
            } else if ((unsigned char)*curr_ptr == TOK_SPC || (unsigned char)*curr_ptr == TOK_TAB) {
                curr_ptr++;
                while (*curr_ptr == ' ') curr_ptr++;
                if (*curr_ptr == '(') curr_ptr++;
                arg = eval_expr(&curr_ptr);
                while (*curr_ptr == ' ') curr_ptr++;
                if (*curr_ptr == ')') curr_ptr++;
                
                n = (int)arg.num;
                if (n < 0) n = 0;
                for (i = 0; i < n; i++) printf(" ");
                newline = 0;
            } else {
                val = eval_expr(&curr_ptr);
                if (val.type == 1) {
                    if (val.str) printf("%s", val.str);
                } else {
                    if (val.num >= 0) printf(" ");
                    printf("%g ", val.num);
                }
                if (val.type == 1 && val.str) free(val.str);
                newline = 1;
            }
        }
        if (newline) printf("\n");
        return;
    }
    
    if (tok == TOK_GOTO || tok == TOK_GO) {
        Value val;
        Line *target;
        
        curr_ptr++;
        while (*curr_ptr == ' ') curr_ptr++;
        if (tok == TOK_GO && (unsigned char)*curr_ptr == TOK_TO) curr_ptr++;
        val = eval_expr(&curr_ptr);
        
        target = prog;
        while (target) {
            if (target->num == (int)val.num) break;
            target = target->next;
        }
        if (!target) {
            printf("\n?UNDEF'D STATEMENT ERROR IN %d\n", curr_line->num);
            end_prog = 1;
            return;
        }
        curr_line = target;
        curr_ptr = target->text;
        return;
    }
    
    if (tok == TOK_GOSUB) {
        Value val;
        Line *target;
        Gosub *g;
        
        curr_ptr++;
        val = eval_expr(&curr_ptr);
        
        target = prog;
        while (target) {
            if (target->num == (int)val.num) break;
            target = target->next;
        }
        if (!target) {
            printf("\n?UNDEF'D STATEMENT ERROR IN %d\n", curr_line->num);
            end_prog = 1;
            return;
        }
        
        g = (Gosub*)malloc(sizeof(Gosub));
        g->line = curr_line;
        g->ptr = curr_ptr;
        g->next = gosubs;
        gosubs = g;
        
        curr_line = target;
        curr_ptr = target->text;
        return;
    }
    
    if (tok == TOK_RETURN) {
        Gosub *g;
        
        curr_ptr++;
        if (!gosubs) {
            printf("\n?RETURN WITHOUT GOSUB ERROR IN %d\n", curr_line->num);
            end_prog = 1;
            return;
        }
        
        g = gosubs;
        curr_line = g->line;
        curr_ptr = g->ptr;
        gosubs = g->next;
        free(g);
        return;
    }
    
    if (tok == TOK_IF) {
        Value cond, val;
        Line *target;
        int truth;
        
        curr_ptr++;
        cond = eval_expr(&curr_ptr);
        truth = (cond.num != 0.0);
        
        while (*curr_ptr == ' ') curr_ptr++;
        if ((unsigned char)*curr_ptr == TOK_THEN || (unsigned char)*curr_ptr == TOK_GOTO) {
            if ((unsigned char)*curr_ptr == TOK_THEN) curr_ptr++;
        }
        
        if (truth) {
            while (*curr_ptr == ' ') curr_ptr++;
            if (isdigit((unsigned char)*curr_ptr)) {
                val = eval_expr(&curr_ptr);
                target = prog;
                while (target) {
                    if (target->num == (int)val.num) break;
                    target = target->next;
                }
                if (!target) {
                    printf("\n?UNDEF'D STATEMENT ERROR IN %d\n", curr_line->num);
                    end_prog = 1;
                    return;
                }
                curr_line = target;
                curr_ptr = target->text;
            }
        } else {
            while (*curr_ptr) curr_ptr++;
        }
        return;
    }
    
    if (tok == TOK_FOR) {
        char vname[4];
        Value start, limit, step;
        ForLoop *f;
        
        curr_ptr++;
        read_var(&curr_ptr, vname);
        while (*curr_ptr == ' ') curr_ptr++;
        if ((unsigned char)*curr_ptr == TOK_EQ || *curr_ptr == '=') curr_ptr++;
        
        start = eval_expr(&curr_ptr);
        set_var(vname, start);
        if (start.type == 1 && start.str) free(start.str);
        
        limit.num = 0; 
        step.num = 1;
        while (*curr_ptr == ' ') curr_ptr++;
        if ((unsigned char)*curr_ptr == TOK_TO) {
            curr_ptr++;
            limit = eval_expr(&curr_ptr);
        }
        while (*curr_ptr == ' ') curr_ptr++;
        if ((unsigned char)*curr_ptr == TOK_STEP) {
            curr_ptr++;
            step = eval_expr(&curr_ptr);
        }
        
        f = (ForLoop*)malloc(sizeof(ForLoop));
        strcpy(f->var_name, vname);
        f->to_val = limit.num;
        f->step_val = step.num;
        f->line = curr_line;
        f->ptr = curr_ptr;
        f->next = fors;
        fors = f;
        return;
    }
    
    if (tok == TOK_NEXT) {
        char vname[4];
        ForLoop *f;
        ForLoop *prev;
        Value val;
        
        curr_ptr++;
        while (*curr_ptr == ' ') curr_ptr++;
        vname[0] = '\0';
        if (isalpha((unsigned char)*curr_ptr)) read_var(&curr_ptr, vname);
        
        f = fors;
        prev = NULL;
        if (vname[0] != '\0') {
            while (f) {
                if (strcmp(f->var_name, vname) == 0) break;
                prev = f;
                f = f->next;
            }
        }
        if (!f) {
            printf("\n?NEXT WITHOUT FOR ERROR IN %d\n", curr_line->num);
            end_prog = 1;
            return;
        }
        
        val = get_var(f->var_name);
        val.num += f->step_val;
        set_var(f->var_name, val);
        if (val.type == 1 && val.str) free(val.str);
        
        if ((f->step_val > 0 && val.num <= f->to_val) || (f->step_val < 0 && val.num >= f->to_val)) {
            curr_line = f->line;
            curr_ptr = f->ptr;
        } else {
            if (prev) prev->next = f->next;
            else fors = f->next;
            free(f);
        }
        return;
    }
    
    if (tok == TOK_END || tok == TOK_STOP) {
        end_prog = 1;
        return;
    }
    
    if (tok == TOK_SLEEP) {
        Value ticks;
        double ms;
        
        curr_ptr++;
        ticks = eval_expr(&curr_ptr);
        ms = ticks.num * (1000.0 / 60.0);
        
#if defined(_WIN32)
        Sleep((DWORD)ms);
#else
        {
            struct timespec ts;
            ts.tv_sec = (time_t)(ms / 1000.0);
            ts.tv_nsec = (long)(fmod(ms, 1000.0) * 1000000.0);
            nanosleep(&ts, NULL);
        }
#endif
        return;
    }
    
    if (tok == TOK_INPUT) {
        char vname[4];
        char buf[256];
        char *nl;
        Value val;
        
        curr_ptr++;
        while (*curr_ptr == ' ') curr_ptr++;
        if (*curr_ptr == '"') {
            curr_ptr++;
            while (*curr_ptr && *curr_ptr != '"') {
                putchar(*curr_ptr);
                curr_ptr++;
            }
            if (*curr_ptr == '"') curr_ptr++;
            while (*curr_ptr == ' ') curr_ptr++;
            if (*curr_ptr == ';' || *curr_ptr == ',') curr_ptr++;
        } else {
            printf("? ");
        }
        
        read_var(&curr_ptr, vname);
        if (fgets(buf, sizeof(buf), stdin)) {
            nl = strchr(buf, '\n');
            if (nl) *nl = '\0';
            nl = strchr(buf, '\r');
            if (nl) *nl = '\0';
            
            val.type = strchr(vname, '$') ? 1 : 0;
            if (val.type == 1) {
                val.str = my_strdup(buf);
                val.num = 0.0;
            } else {
                val.str = NULL;
                val.num = atof(buf);
            }
            set_var(vname, val);
            if (val.type == 1 && val.str) free(val.str);
        }
        return;
    }
    
    if (tok == TOK_LET || isalpha((unsigned char)*curr_ptr)) {
        char vname[4];
        Value val;
        
        if (tok == TOK_LET) curr_ptr++;
        read_var(&curr_ptr, vname);
        while (*curr_ptr == ' ') curr_ptr++;
        if ((unsigned char)*curr_ptr == TOK_EQ || *curr_ptr == '=') {
            curr_ptr++;
        } else {
            printf("\n?SYNTAX ERROR IN %d\n", curr_line->num);
            end_prog = 1;
            return;
        }
        
        val = eval_expr(&curr_ptr);
        set_var(vname, val);
        if (val.type == 1 && val.str) free(val.str);
        return;
    }
    
    printf("\n?SYNTAX ERROR IN %d\n", curr_line->num);
    end_prog = 1;
}

/* -- Standard Interpreter Init -- */
void load_program(const char *filename) {
    FILE *f;
    char buf[1024];
    char *p;
    char *nl_ptr;
    int num;
    
    f = fopen(filename, "r");
    if (!f) {
        printf("?FILE NOT FOUND\n");
        exit(1);
    }
    
    while (fgets(buf, sizeof(buf), f)) {
        p = buf;
        while (*p == ' ' || *p == '\t') p++;
        if (isdigit((unsigned char)*p)) {
            num = atoi(p);
            while (isdigit((unsigned char)*p)) p++;
            while (*p == ' ' || *p == '\t') p++;
            
            nl_ptr = strchr(p, '\n');
            if (nl_ptr) *nl_ptr = '\0';
            nl_ptr = strchr(p, '\r');
            if (nl_ptr) *nl_ptr = '\0';
            
            add_line(num, p);
        }
    }
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("USAGE: cbmbasic <script.bas>\n");
        return 1;
    }
    
    srand((unsigned int)time(NULL));
    load_program(argv[1]);
    
    curr_line = prog;
    if (curr_line) curr_ptr = curr_line->text;
    
    while (curr_line && !end_prog) {
        while (*curr_ptr && !end_prog) {
            execute_statement();
            if (end_prog) break;
            
            if (curr_line && *curr_ptr == ':') {
                curr_ptr++;
            } else if (curr_line && !*curr_ptr) {
                curr_line = curr_line->next;
                if (curr_line) curr_ptr = curr_line->text;
                break;
            }
        }
    }
    
    return 0;
}