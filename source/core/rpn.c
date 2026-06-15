/*
 * ---
 * BASIC++ Interpreter - rpn.c
 * ---
 *
 * RPN (Reverse Polish Notation) calculator implementation.
 * Forth-like stack evaluator. Activated by OPTION RPN.
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "rpn.h"

/* --- Stack Operations --- */

static int rpn_push(RpnState *s, double val)
{
 if (s->top >= RPN_STACK_SIZE) {
  printf("RPN: stack overflow\n");
  return 0;
 }
 s->stack[s->top++] = val;
 return 1;
}

static int rpn_pop(RpnState *s, double *val)
{
 if (s->top <= 0) {
  printf("RPN: stack underflow\n");
  return 0;
 }
 *val = s->stack[--s->top];
 return 1;
}

static int rpn_peek(RpnState *s, double *val)
{
 if (s->top <= 0) {
  printf("RPN: stack empty\n");
  return 0;
 }
 *val = s->stack[s->top - 1];
 return 1;
}

/* --- Case-insensitive compare --- */
static int rpn_streq(const char *a, const char *b)
{
 while (*a && *b) {
  char ca = *a, cb = *b;
  if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
  if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
  if (ca != cb) return 0;
  a++; b++;
 }
 return (*a == '\0' && *b == '\0');
}

/* --- Public API --- */

void rpn_init(RpnState *s)
{
 s->top = 0;
 s->active = 0;
}

int rpn_is_active(RpnState *s)
{
 return s->active;
}

void rpn_set_active(RpnState *s, int on)
{
 s->active = on;
 if (on) {
  printf("RPN mode: ON (Forth-style stack calculator)\n");
  printf(" Enter numbers to push, operators to compute.\n");
  printf(" .  = pop and print    .S = show stack\n");
  printf(" Type OPTION RPN OFF to return to BASIC.\n");
 } else {
  printf("RPN mode: OFF\n");
 }
}

/*
 * rpn_print_number - Print a number, integer-style if no fraction.
 */
static void rpn_print_number(double val)
{
 if (val == floor(val) && fabs(val) < 1e15) {
  printf(" %ld", (long)val);
 } else {
  printf(" %G", val);
 }
}

int rpn_eval_line(RpnState *s, const char *line)
{
 char token[64];
 int pos = 0;
 int len = (int)strlen(line);

 while (pos < len) {
  int tlen = 0;
  double a, b;

  /* Skip whitespace */
  while (pos < len && (line[pos] == ' ' || line[pos] == '\t'))
   pos++;
  if (pos >= len) break;

  /* Extract token */
  while (pos < len && tlen < 63 &&
   line[pos] != ' ' && line[pos] != '\t' &&
   line[pos] != '\r' && line[pos] != '\n') {
   token[tlen++] = line[pos++];
  }
  token[tlen] = '\0';
  if (tlen == 0) break;

  /* Check for OPTION RPN OFF */
  if (rpn_streq(token, "OPTION")) {
   /* Check next tokens for "RPN OFF" */
   char t2[64], t3[64];
   int t2l = 0, t3l = 0;

   while (pos < len && (line[pos] == ' ' || line[pos] == '\t'))
    pos++;
   while (pos < len && t2l < 63 &&
    line[pos] != ' ' && line[pos] != '\t' &&
    line[pos] != '\r' && line[pos] != '\n') {
    t2[t2l++] = line[pos++];
   }
   t2[t2l] = '\0';

   while (pos < len && (line[pos] == ' ' || line[pos] == '\t'))
    pos++;
   while (pos < len && t3l < 63 &&
    line[pos] != ' ' && line[pos] != '\t' &&
    line[pos] != '\r' && line[pos] != '\n') {
    t3[t3l++] = line[pos++];
   }
   t3[t3l] = '\0';

   if (rpn_streq(t2, "RPN") && rpn_streq(t3, "OFF")) {
    rpn_set_active(s, 0);
    return 1;
   }
   printf("RPN: unknown OPTION\n");
   return 1;
  }

  /* Try number (including negative numbers and hex) */
  if ((token[0] >= '0' && token[0] <= '9') ||
   (token[0] == '-' && tlen > 1 && token[1] >= '0' && token[1] <= '9') ||
   (token[0] == '.' && tlen > 1)) {
   char *endptr;
   double val = strtod(token, &endptr);
   if (*endptr == '\0') {
    rpn_push(s, val);
    continue;
   }
  }

  /* Hex literals: &Hnn */
  if (tlen > 2 && token[0] == '&' &&
   (token[1] == 'H' || token[1] == 'h')) {
   long hval = strtol(token + 2, NULL, 16);
   rpn_push(s, (double)hval);
   continue;
  }

  /* Single-char operators */
  if (tlen == 1) {
   switch (token[0]) {
   case '+':
    if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
    rpn_push(s, a + b);
    continue;
   case '-':
    if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
    rpn_push(s, a - b);
    continue;
   case '*':
    if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
    rpn_push(s, a * b);
    continue;
   case '/':
    if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
    if (b == 0.0) { printf("RPN: division by zero\n"); return 1; }
    rpn_push(s, a / b);
    continue;
   case '.':
    if (!rpn_pop(s, &a)) return 1;
    rpn_print_number(a);
    printf("\n");
    continue;
   case '=':
    if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
    rpn_push(s, (a == b) ? -1.0 : 0.0);
    continue;
   case '<':
    if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
    rpn_push(s, (a < b) ? -1.0 : 0.0);
    continue;
   case '>':
    if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
    rpn_push(s, (a > b) ? -1.0 : 0.0);
    continue;
   default:
    break;
   }
  }

  /* Multi-char operators / words */
  if (rpn_streq(token, "DUP")) {
   if (!rpn_peek(s, &a)) return 1;
   rpn_push(s, a);
  } else if (rpn_streq(token, "DROP")) {
   if (!rpn_pop(s, &a)) return 1;
  } else if (rpn_streq(token, "SWAP")) {
   if (s->top < 2) { printf("RPN: stack underflow\n"); return 1; }
   a = s->stack[s->top - 1];
   s->stack[s->top - 1] = s->stack[s->top - 2];
   s->stack[s->top - 2] = a;
  } else if (rpn_streq(token, "OVER")) {
   if (s->top < 2) { printf("RPN: stack underflow\n"); return 1; }
   rpn_push(s, s->stack[s->top - 2]);
  } else if (rpn_streq(token, "ROT")) {
   if (s->top < 3) { printf("RPN: stack underflow\n"); return 1; }
   a = s->stack[s->top - 3];
   s->stack[s->top - 3] = s->stack[s->top - 2];
   s->stack[s->top - 2] = s->stack[s->top - 1];
   s->stack[s->top - 1] = a;
  } else if (rpn_streq(token, "DEPTH")) {
   rpn_push(s, (double)s->top);
  } else if (rpn_streq(token, "CLEAR")) {
   s->top = 0;
  } else if (rpn_streq(token, ".S")) {
   int i;
   printf("<%d>", s->top);
   for (i = 0; i < s->top; i++) {
    rpn_print_number(s->stack[i]);
   }
   printf("\n");
  } else if (rpn_streq(token, "MOD")) {
   if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
   if (b == 0.0) { printf("RPN: division by zero\n"); return 1; }
   rpn_push(s, (double)((long)a % (long)b));
  } else if (rpn_streq(token, "AND")) {
   if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
   rpn_push(s, (double)((long)a & (long)b));
  } else if (rpn_streq(token, "OR")) {
   if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
   rpn_push(s, (double)((long)a | (long)b));
  } else if (rpn_streq(token, "XOR")) {
   if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
   rpn_push(s, (double)((long)a ^ (long)b));
  } else if (rpn_streq(token, "NOT")) {
   if (!rpn_pop(s, &a)) return 1;
   rpn_push(s, (a == 0.0) ? -1.0 : 0.0);
  } else if (rpn_streq(token, "NEGATE")) {
   if (!rpn_pop(s, &a)) return 1;
   rpn_push(s, -a);
  } else if (rpn_streq(token, "ABS")) {
   if (!rpn_pop(s, &a)) return 1;
   rpn_push(s, fabs(a));
  } else if (rpn_streq(token, "SQRT") || rpn_streq(token, "SQR")) {
   if (!rpn_pop(s, &a)) return 1;
   if (a < 0.0) { printf("RPN: negative sqrt\n"); return 1; }
   rpn_push(s, sqrt(a));
  } else if (rpn_streq(token, "SIN")) {
   if (!rpn_pop(s, &a)) return 1;
   rpn_push(s, sin(a));
  } else if (rpn_streq(token, "COS")) {
   if (!rpn_pop(s, &a)) return 1;
   rpn_push(s, cos(a));
  } else if (rpn_streq(token, "TAN")) {
   if (!rpn_pop(s, &a)) return 1;
   rpn_push(s, tan(a));
  } else if (rpn_streq(token, "LOG") || rpn_streq(token, "LN")) {
   if (!rpn_pop(s, &a)) return 1;
   if (a <= 0.0) { printf("RPN: log of non-positive\n"); return 1; }
   rpn_push(s, log(a));
  } else if (rpn_streq(token, "EXP")) {
   if (!rpn_pop(s, &a)) return 1;
   rpn_push(s, exp(a));
  } else if (rpn_streq(token, "POW") || rpn_streq(token, "**")) {
   if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
   rpn_push(s, pow(a, b));
  } else if (rpn_streq(token, "PI")) {
   rpn_push(s, 3.14159265358979323846);
  } else if (rpn_streq(token, "E")) {
   rpn_push(s, 2.71828182845904523536);
  } else if (rpn_streq(token, "<>")) {
   if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
   rpn_push(s, (a != b) ? -1.0 : 0.0);
  } else if (rpn_streq(token, "<=")) {
   if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
   rpn_push(s, (a <= b) ? -1.0 : 0.0);
  } else if (rpn_streq(token, ">=")) {
   if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
   rpn_push(s, (a >= b) ? -1.0 : 0.0);
  } else if (rpn_streq(token, "MIN")) {
   if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
   rpn_push(s, (a < b) ? a : b);
  } else if (rpn_streq(token, "MAX")) {
   if (!rpn_pop(s, &b) || !rpn_pop(s, &a)) return 1;
   rpn_push(s, (a > b) ? a : b);
  } else if (rpn_streq(token, "BYE") || rpn_streq(token, "QUIT")) {
   rpn_set_active(s, 0);
   return 1;
  } else {
   printf("RPN: unknown word '%s'\n", token);
  }
 }

 return 1;
}
