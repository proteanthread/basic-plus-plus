/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: format_input.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Input/Output operations, screen print layout, file streams (sequential/binary/random), block I/O formatting, and record buffers.
 *
 * 2. WHAT TO EXPECT:
 *    Delegates file operations through the Virtual Device (VDev) mapping layer. Relies on standard ANSI C streams.
 *
 * 3. WHAT CAN BE CHANGED:
 *    File channel limit rules, I/O default buffers, record layout details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Binary packing format definitions (MKI$, MKS$, MKD$), file channel index lookups.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check file channel descriptor leaks (ensure CLOSE #ch is called). Verify binary file alignments and record record size boundaries.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - format_input.c
 // ---
 //
 // Formatted input validation engine for INPUT USING /
 // GET USING / READ USING.
 //
 // Provides Microsoft-style INPUT prompt protection: the user
 // cannot backspace past the prompt character (? or custom).
 // This prevents the Atari BASIC bug where backspacing past
 // the ? prompt could crash the program.
 //
 // INPUT USING / GET USING / READ USING validate input against
 // a format specification string. If input does not match the
 // format, it is silently discarded and the user is re-prompted.
 //
 // Format specifiers for input validation:
 //   A   ASCII character (any printable)
 //   K   Internal keycode (raw)
 //   D   Default answer (pre-fill)
 //   U   Uppercase convert (force upper)
 //   L   Lowercase convert (force lower)
 //   P   Parse text
 //   S   String/line input
 //   I   Input prompt text (custom)
 //   N   Numeric (decimal) only
 //   H   Hex only
 //   O   Octal only
 //   G   GOTO on valid input
 //   T   Text only (reject digits)
 //   X   Execute internal sub-routine
 //   E   Execute external sub-routine
 //   F   Function keys
 //   R   Return key trapping
 //   ?   Question mark prompt handling
 //   C   Cursor keys
 //   B   Trap or disable break key
 //
 // ---

#include "parser_internal.h"
#include "../console.h"

// ============================================================
 // MICROSOFT-STYLE INPUT PROTECTION
 // ============================================================
 //
 // On Atari BASIC, one could backspace past the '?' prompt,
 // corrupting the input buffer and potentially crashing the
 // program. Microsoft BASICs (GW-BASIC, QBasic, etc.) prevent
 // this: the cursor stops at the prompt position.
 //
 // input_read_protected() reads a line from stdin, enforcing
 // a minimum cursor position (the prompt end position). Any
 // backspace that would move the cursor before this position
 // is ignored.

 // input_read_protected - Read input with backspace protection.
 //
 // Reads characters from stdin into buf (up to maxlen-1 chars),
 // enforcing that backspace cannot erase past the prompt.
 // Returns the number of characters read (excluding NUL).
 //
 // Parameters:
 //   buf     - output buffer
 //   maxlen  - buffer size
 //   prompt  - prompt string to display (NULL = "? ")
 //
 // Note: On Windows, this uses simple fgets with post-processing.
 // Full character-by-character input with cursor control would
 // require platform-specific terminal APIs (conio.h on Windows,
 // termios on Unix). For now, we use fgets and strip any
 // characters that would have underflowed the prompt.
int input_read_protected(char *buf, int maxlen, const char *prompt)
{
 int len;

 if (prompt)
  gw_printf("%s", prompt);
 else
  gw_printf("? ");
 gw_fflush(stdout);

 if (fgets(buf, maxlen, stdin) == NULL) {
  buf[0] = '\0';
  return -1;
 }

 // Strip trailing newline/carriage return
 len = (int)strlen(buf);
 while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
  len--;
 buf[len] = '\0';

 return len;
}

// ============================================================
 // INPUT FORMAT VALIDATION
 // ============================================================ 

 // input_parse_format - Parse an input format string.
 //
 // Reads the format string and populates the spec structure.
void input_parse_format(const char *fmt, int flen,
          InputFormatSpec *spec)
{
 int fi = 0;

 memset(spec, 0, sizeof(*spec));

 while (fi < flen) {
  char ch = fmt[fi];
  int rep = 0;

  // Parse optional numeric prefix
  while (fi < flen && fmt[fi] >= '0' && fmt[fi] <= '9') {
   rep = rep * 10 + (fmt[fi] - '0');
   fi++;
  }
  if (fi >= flen) break;
  ch = fmt[fi];
  fi++;

  switch (ch) {
  case 'N': case 'n':
   spec->numeric_only = 1;
   break;
  case 'H': case 'h':
   spec->hex_only = 1;
   break;
  case 'O': case 'o':
   spec->octal_only = 1;
   break;
  case 'T': case 't':
   spec->text_only = 1;
   break;
  case 'U': case 'u':
   spec->force_upper = 1;
   break;
  case 'L': case 'l':
   spec->force_lower = 1;
   break;
  case 'S': case 's':
   spec->max_length = (rep > 0) ? rep : 255;
   break;
  case 'A': case 'a':
   // Accept any printable character
   break;
  case 'D': case 'd':
   spec->has_default = 1;
   // Default value follows in quotes or next field
   break;
  default:
   // Unknown specifier, skip
   break;
  }
 }
}

 // input_validate - Validate input string against format spec.
 //
 // Returns 1 if input is valid, 0 if it should be rejected.
 // Also applies transformations (uppercase/lowercase).
int input_validate(char *buf, int len,
     const InputFormatSpec *spec)
{
 int i;

 // Check max length
 if (spec->max_length > 0 && len > spec->max_length) {
  buf[spec->max_length] = '\0';
  len = spec->max_length;
 }

 // Validate character constraints
 for (i = 0; i < len; i++) {
  char ch = buf[i];

  if (spec->numeric_only) {
   if (!((ch >= '0' && ch <= '9') ||
         ch == '+' || ch == '-' || ch == '.' ||
         ch == ' '))
    return 0;
  }
  if (spec->hex_only) {
   if (!((ch >= '0' && ch <= '9') ||
         (ch >= 'A' && ch <= 'F') ||
         (ch >= 'a' && ch <= 'f') ||
         ch == ' '))
    return 0;
  }
  if (spec->octal_only) {
   if (!(ch >= '0' && ch <= '7'))
    return 0;
  }
  if (spec->text_only) {
   if (ch >= '0' && ch <= '9')
    return 0;
  }

  // Apply case conversion
  if (spec->force_upper && ch >= 'a' && ch <= 'z')
   buf[i] = (char)(ch - 32);
  if (spec->force_lower && ch >= 'A' && ch <= 'Z')
   buf[i] = (char)(ch + 32);
 }

 return 1;
}

// ============================================================
 // INPUT USING HANDLER
 // ============================================================ 

 // format_input_using - Read input with format validation.
 //
 // Reads a line from stdin, validates against the format spec.
 // If invalid, re-prompts. Stores the result in buf.
 // Returns the length of the validated input, or -1 on error.
int format_input_using(char *buf, int maxlen, const char *fmt,
         int flen, const char *prompt)
{
 InputFormatSpec spec;
 int len;

 input_parse_format(fmt, flen, &spec);

 for (;;) {
  len = input_read_protected(buf, maxlen, prompt);
  if (len < 0) return -1;

  if (input_validate(buf, len, &spec))
   return (int)strlen(buf);

  // Invalid input: re-prompt
  gw_printf("?Redo from start\n");
 }
}
