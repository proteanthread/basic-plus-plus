/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: format_using_core.c
 * Subsystem: Formatted PRINT USING Formatter
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Formats numbers and strings based on templates (PRINT USING).
 *
 * 2. WHAT TO EXPECT:
 *    Renders padded values and adds decimal signs.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Currency signs, digit punctuation delimiters.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Scientific format formatting math.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If text overflows, verify formatting mask width.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE FORMAT USING SUBSYSTEM
 * File: format_using_core.c
 * ===================================================================== */

#include "format_using_core.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

 // format_using_emit_char - Write a single character to stream.
void format_using_emit_char(FILE *fp, char ch)
{
 fputc(ch, fp);
}

 // format_using_emit_str - Write a string to stream.
void format_using_emit_str(FILE *fp, const char *s, int len)
{
 if (s && len > 0)
  fwrite(s, 1, (size_t)len, fp);
}

 // format_using_emit_fill - Write n copies of a character.
void format_using_emit_fill(FILE *fp, char ch, int n)
{
 while (n-- > 0)
  fputc(ch, fp);
}

// ============================================================
 // ESCAPE SEQUENCE PROCESSING
 // ============================================================ 

 // format_using_escape - Process a backslash escape in format.
 //
 // Returns the number of format chars consumed (including the \).
 // Emits the escape character to the stream.
int format_using_escape(FILE *fp, const char *fmt,
          int pos, int flen)
{
 char ch;

 if (pos + 1 >= flen) {
  // Bare backslash at end
  format_using_emit_char(fp, '\\');
  return 1;
 }

 ch = fmt[pos + 1];
 switch (ch) {
 case 'n':  format_using_emit_char(fp, '\n'); return 2;
 case 't':  format_using_emit_char(fp, '\t'); return 2;
 case 'b':  format_using_emit_char(fp, '\b'); return 2;
 case 'r':  format_using_emit_char(fp, '\r'); return 2;
 case 'c':  /* inhibit CR a" no-op */          return 2;
 case 'a':  format_using_emit_char(fp, '\a'); return 2;
 case 'f':  format_using_emit_char(fp, '\f'); return 2;
 case 'v':  format_using_emit_char(fp, '\v'); return 2;
 case '\\': format_using_emit_char(fp, '\\'); return 2;
 case '\'': format_using_emit_char(fp, '\''); return 2;
 case '"':  format_using_emit_char(fp, '"');  return 2;
 case '?':  format_using_emit_char(fp, '?');  return 2;
 case '0':  format_using_emit_char(fp, '\0'); return 2;
 case 'o': {
  // Octal character: \oNNN
  int val = 0, i;
  int consumed = 2;
  for (i = 0; i < 3 && pos + 2 + i < flen; i++) {
   char d = fmt[pos + 2 + i];
   if (d >= '0' && d <= '7') {
    val = val * 8 + (d - '0');
    consumed++;
   } else break;
  }
  format_using_emit_char(fp, (char)val);
  return consumed;
 }
 case 'x': {
  // Hex character: \xHH
  int val = 0, i;
  int consumed = 2;
  for (i = 0; i < 2 && pos + 2 + i < flen; i++) {
   char d = fmt[pos + 2 + i];
   if (d >= '0' && d <= '9') {
    val = val * 16 + (d - '0');
    consumed++;
   } else if (d >= 'A' && d <= 'F') {
    val = val * 16 + (d - 'A' + 10);
    consumed++;
   } else if (d >= 'a' && d <= 'f') {
    val = val * 16 + (d - 'a' + 10);
    consumed++;
   } else break;
  }
  format_using_emit_char(fp, (char)val);
  return consumed;
 }
 default:
  // Unknown escape: print literally
  format_using_emit_char(fp, '\\');
  format_using_emit_char(fp, ch);
  return 2;
 }
}

// ============================================================
 // NUMERIC FORMATTING
 // ============================================================ 

 // format_insert_commas - Insert commas into a digit string.
 //
 // Takes a right-justified digit string (spaces/zeros + digits),
 // inserts commas every 3 digits from the right. Writes result
 // into outbuf and returns the length.
static int format_insert_commas(const char *digits, int dlen,
    char *outbuf, int outmax)
{
 int i, j, count, start;

 // Find where digits start (skip spaces/signs)
 start = 0;
 while (start < dlen && (digits[start] == ' ' ||
        digits[start] == '+' || digits[start] == '-' ||
        digits[start] == '$' || digits[start] == '*'))
  start++;

 // Count actual digits
 count = dlen - start;
 if (count <= 3 || outmax < dlen + count / 3 + 1) {
  // No commas needed or buffer too small
  memcpy(outbuf, digits, (size_t)dlen);
  return dlen;
 }

 // Copy prefix (spaces/signs)
 j = 0;
 for (i = 0; i < start && j < outmax - 1; i++)
  outbuf[j++] = digits[i];

 // Insert commas from right: figure out first group size
 {
  int first_group = count % 3;
  int pos = 0;

  if (first_group == 0) first_group = 3;

  for (i = start; i < dlen && j < outmax - 1; i++) {
   outbuf[j++] = digits[i];
   pos++;
   if (pos == first_group && i + 1 < dlen &&
       j < outmax - 1) {
    outbuf[j++] = ',';
    first_group = 999; // reset
    pos = 0;
   } else if (pos > 0 && pos % 3 == 0 &&
       i + 1 < dlen && j < outmax - 1 &&
       first_group == 999) {
    outbuf[j++] = ',';
    pos = 0;
   }
  }
 }
 outbuf[j] = '\0';
 return j;
}

 // format_using_numeric - Format a single numeric value.
 //
 // Parses the format specifier starting at fmt[*pos] and
 // formats the double value. Writes output to fp.
 // Advances *pos past the consumed format specifiers.
 //
 // Returns 1 if a value was consumed, 0 if not.
int format_using_numeric(FILE *fp, const char *fmt, int flen,
    int *pos, double value)
{
 int fi = *pos;
 int whole = 0, frac = 0, has_dot = 0;
 int has_comma = 0;
 int has_leading_plus = 0, has_trailing_plus = 0;
 int has_trailing_minus = 0;
 int has_dollar = 0, has_float_dollar = 0;
 int has_star_fill = 0;
 int has_exp = 0;
 int has_zero_fill = 0;
 int fw;
 char fill_char = ' ';
 char nbuf[128];
 int nlen;
 int is_neg;

 // Check for leading +
 if (fi < flen && fmt[fi] == '+') {
  has_leading_plus = 1;
  fi++;
 }

 // Check for ** (asterisk fill)
 if (fi + 1 < flen && fmt[fi] == '*' && fmt[fi + 1] == '*') {
  has_star_fill = 1;
  fill_char = '*';
  whole += 2;
  fi += 2;
  // Check for **$
  if (fi < flen && fmt[fi] == '$') {
   has_dollar = 1;
   fi++;
  }
 }
 // Check for $$ (floating dollar)
 else if (fi + 1 < flen && fmt[fi] == '$' && fmt[fi + 1] == '$') {
  has_float_dollar = 1;
  has_dollar = 1;
  whole += 1; // $$ gives one digit position
  fi += 2;
 }

 // Count # digits before decimal point
 while (fi < flen && (fmt[fi] == '#' || fmt[fi] == 'D' ||
        fmt[fi] == 'd' || fmt[fi] == '0' ||
        fmt[fi] == 'Z' || fmt[fi] == 'z')) {
  if (fmt[fi] == '0' || fmt[fi] == 'Z' || fmt[fi] == 'z')
   has_zero_fill = 1;
  whole++;
  fi++;
 }

 // Check for comma (grouping)
 if (fi < flen && fmt[fi] == ',') {
  has_comma = 1;
  fi++;
  // Continue counting digits after comma
  while (fi < flen && (fmt[fi] == '#' || fmt[fi] == 'D' ||
         fmt[fi] == 'd' || fmt[fi] == ',' ||
         fmt[fi] == '0' || fmt[fi] == 'Z' ||
         fmt[fi] == 'z')) {
   if (fmt[fi] == ',') {
    fi++;
    continue;
   }
   if (fmt[fi] == '0' || fmt[fi] == 'Z' ||
       fmt[fi] == 'z')
    has_zero_fill = 1;
   whole++;
   fi++;
  }
 }

 // Check for decimal point
 if (fi < flen && fmt[fi] == '.') {
  has_dot = 1;
  fi++;
  // Count fractional digits
  while (fi < flen && (fmt[fi] == '#' || fmt[fi] == 'D' ||
         fmt[fi] == 'd' || fmt[fi] == '0' ||
         fmt[fi] == 'Z' || fmt[fi] == 'z')) {
   frac++;
   fi++;
  }
 }

 // If no digits found at all, not a numeric format
 if (whole == 0 && frac == 0 && !has_leading_plus &&
     !has_star_fill && !has_float_dollar) {
  return 0;
 }

 // Check for ^^^^ (exponential)
 if (fi + 3 < flen && fmt[fi] == '^' && fmt[fi + 1] == '^' &&
     fmt[fi + 2] == '^' && fmt[fi + 3] == '^') {
  has_exp = 1;
  fi += 4;
 }

 // Check for trailing sign
 if (fi < flen && fmt[fi] == '-') {
  has_trailing_minus = 1;
  fi++;
 } else if (fi < flen && fmt[fi] == '+') {
  has_trailing_plus = 1;
  fi++;
 }

 // Now format the value
 is_neg = (value < 0.0);
 if (is_neg) value = -value;

 if (has_zero_fill) fill_char = '0';

 if (has_exp) {
  // Exponential format
  int total_digits = whole + frac;
  if (total_digits < 1) total_digits = 1;
  sprintf(nbuf, "%*.*E",
   whole + (has_dot ? 1 + frac : 0) + 5,
   frac > 0 ? frac : (total_digits - 1),
   is_neg ? -value : value);
  // Override sign handling for exp
  if (has_leading_plus && !is_neg) {
   fprintf(fp, "+");
  } else if (has_leading_plus && is_neg) {
   fprintf(fp, "-");
  }
  fprintf(fp, "%s", nbuf);
  if (has_trailing_minus) {
   fprintf(fp, "%c", is_neg ? '-' : ' ');
  } else if (has_trailing_plus) {
   fprintf(fp, "%c", is_neg ? '-' : '+');
  }
 } else {
  // Fixed-point format
  fw = whole + (has_dot ? 1 + frac : 0);

  if (has_dot) {
   sprintf(nbuf, "%*.*f", fw, frac, value);
  } else {
   sprintf(nbuf, "%*ld", whole,
    (long)(value + 0.5));
  }
  nlen = (int)strlen(nbuf);

  // Check for overflow (value too wide)
  if (nlen > fw && fw > 0) {
   // GW-BASIC prints % then the overflowed value
   fprintf(fp, "%%");
   fprintf(fp, "%s", nbuf);
   if (has_trailing_minus)
    fprintf(fp, "%c", is_neg ? '-' : ' ');
   else if (has_trailing_plus)
    fprintf(fp, "%c", is_neg ? '-' : '+');
  } else {
   // Apply fill character
   if (fill_char != ' ') {
    int i;
    for (i = 0; i < nlen; i++) {
     if (nbuf[i] == ' ')
      nbuf[i] = fill_char;
     else
      break;
    }
   }

   // Apply comma grouping
   if (has_comma) {
    // Find the integer part
    char *dot = strchr(nbuf, '.');
    int int_len = dot ? (int)(dot - nbuf) :
          nlen;
    char comma_buf[128];
    int clen;

    clen = format_insert_commas(nbuf, int_len,
           comma_buf, 126);
    comma_buf[clen] = '\0';

    // Insert sign
    if (has_leading_plus) {
     fprintf(fp, "%c",
      is_neg ? '-' : '+');
    } else if (is_neg) {
     // Find first space/star and
      // replace with - 
     int placed = 0;
     int i;
     for (i = 0; i < clen; i++) {
      if (comma_buf[i] == ' ' ||
          comma_buf[i] ==
          fill_char) {
       continue;
      }
      if (i > 0) {
       comma_buf[i-1] = '-';
       placed = 1;
      }
      break;
     }
     if (!placed)
      fprintf(fp, "-");
    }

    // Floating dollar
    if (has_dollar) {
     // Insert $ before first digit
     int i;
     for (i = 0; i < clen; i++) {
      if (comma_buf[i] >= '0' &&
          comma_buf[i] <= '9') {
       if (i > 0)
        comma_buf[i-1]
            = '$';
       else
        fprintf(fp,
         "$");
       break;
      }
     }
    }

    fprintf(fp, "%s", comma_buf);
    if (dot)
     fprintf(fp, "%s", dot);
   } else {
    // No commas
    // Insert sign
    if (has_leading_plus) {
     fprintf(fp, "%c",
      is_neg ? '-' : '+');
    } else if (is_neg) {
     // Replace leading space with -
     int i, placed = 0;
     for (i = 0; i < nlen; i++) {
      if (nbuf[i] == ' ' ||
          nbuf[i] == fill_char) {
       continue;
      }
      if (i > 0) {
       nbuf[i-1] = '-';
       placed = 1;
      }
      break;
     }
     if (!placed) {
      fprintf(fp, "-");
     }
    }

    // Floating dollar sign
    if (has_dollar) {
     int i;
     for (i = 0; i < nlen; i++) {
      if (nbuf[i] >= '0' &&
          nbuf[i] <= '9') {
       if (i > 0)
        nbuf[i-1]
            = '$';
       else
        fprintf(fp,
         "$");
       break;
      }
     }
    }

    fprintf(fp, "%s", nbuf);
   }

   // Trailing sign
   if (has_trailing_minus) {
    fprintf(fp, "%c", is_neg ? '-' : ' ');
   } else if (has_trailing_plus) {
    fprintf(fp, "%c", is_neg ? '-' : '+');
   }
  }
 }

 *pos = fi;
 return 1;
}

// ============================================================
 // SPECIAL FORMAT: INTEGER, OCTAL, HEX, BINARY
 // ============================================================ 

 // format_using_radix - Format value in given radix (I/O/H/B).
 //
 // Returns 1 if format was consumed, 0 otherwise.
int format_using_radix(FILE *fp, const char *fmt, int flen,
         int *pos, double value, int rep)
{
 char spec = fmt[*pos];
 long ival = (long)value;
 char buf[68]; // enough for 64-bit binary

 (*pos)++;

 switch (spec) {
 case 'I': case 'i':
  // Integer format
  if (rep > 0)
   fprintf(fp, "%*ld", rep, ival);
  else
   fprintf(fp, "%ld", ival);
  return 1;
 case 'O': case 'o':
  // Octal format
  if (rep > 0)
   fprintf(fp, "%*lo", rep, (unsigned long)ival);
  else
   fprintf(fp, "%lo", (unsigned long)ival);
  return 1;
 case 'H': case 'h':
  // Hexadecimal format
  if (rep > 0)
   fprintf(fp, "%*lX", rep, (unsigned long)ival);
  else
   fprintf(fp, "%lX", (unsigned long)ival);
  return 1;
 case 'B': case 'b': {
  // Binary format
  unsigned long uval = (unsigned long)ival;
  int bi = 0, i;
  if (uval == 0) {
   buf[bi++] = '0';
  } else {
   // Build binary string in reverse
   while (uval > 0 && bi < 64) {
    buf[bi++] = (char)('0' + (uval & 1));
    uval >>= 1;
   }
  }
  // Pad to rep width
  if (rep > 0) {
   int pad = rep - bi;
   while (pad-- > 0)
    fputc('0', fp);
  }
  // Print reversed
  for (i = bi - 1; i >= 0; i--)
   fputc(buf[i], fp);
  return 1;
 }
 case 'E': case 'e':
  // Exponential format
  if (rep > 0)
   fprintf(fp, "%*.*E", rep + 7, rep > 1 ? rep - 1 : 1,
    value);
  else
   fprintf(fp, "%E", value);
  return 1;
 case 'S': case 's':
  // Single precision
  fprintf(fp, "%.7G", value);
  return 1;
 default:
  return 0;
 }
}

// ============================================================
 // STRING FORMATTING
 // ============================================================ 

 // format_using_string_field - Format a string value.
 //
 // Handles !, \ \, &, A format specifiers.
 // Returns 1 if a string value was consumed, 0 otherwise.
int format_using_string_field(FILE *fp, const char *fmt, int flen,
         int *pos, const char *str, int slen)
{
 int fi = *pos;

 if (fi >= flen) return 0;

 switch (fmt[fi]) {
 case '!':
  // First character of string
  fi++;
  if (str && slen > 0)
   fputc(str[0], fp);
  else
   fputc(' ', fp);
  *pos = fi;
  return 1;

 case '\\': {
  // String field: \ \ = width is distance + 2
  int width = 2;
  int pad;
  fi++; // skip first backslash
  while (fi < flen && fmt[fi] != '\\') {
   width++;
   fi++;
  }
  if (fi < flen) fi++; // skip closing backslash

  if (str) {
   int plen = (slen > width) ? width : slen;
   format_using_emit_str(fp, str, plen);
   pad = width - plen;
  } else {
   pad = width;
  }
  format_using_emit_fill(fp, ' ', pad);
  *pos = fi;
  return 1;
 }

 case '&':
  // Entire string, variable width
  fi++;
  if (str && slen > 0)
   format_using_emit_str(fp, str, slen);
  *pos = fi;
  return 1;

 default:
  return 0;
 }
}

// ============================================================
 // MAIN FORMAT PROCESSOR
 // ============================================================ 

 // format_using_process - Process a format string with values.
 //
 // This is the main entry point called by PRINT USING, LPRINT USING,
 // and PRINT#c USING.
 //
 // Parameters:
 //   fp      - output stream (stdout, stderr, or file)
 //   fmt     - format string
 //   flen    - length of format string
 //   lex     - lexer (to parse value expressions)
 //   rt      - runtime state
 //   line_num - current line number (for error reporting)
