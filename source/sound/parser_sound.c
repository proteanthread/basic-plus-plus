/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_sound.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Console attributes (CLS, LOCATE, WIDTH, colors) and sound/music statements (BEEP, SOUND, PLAY).
 *
 * 2. WHAT TO EXPECT:
 *    Uses ANSI escape sequences for text styling. Enforces console bounds checking.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Styling escape sequences, default frequencies, music tempo constraints.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Coordinate range checks, pitch calculation formulas.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure terminal color settings are correct. Verify sound timer routines correspond to system sleep functions.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_sound.c
 // ---
 //
 // Sound & music commands.
 //
 // BEEP, SOUND, PLAY.
 //
//
// HOW TO EXTEND:
//   To add a new statement or sub-command:
//   1. Add the keyword to lexer.h (KeywordId enum).
//   2. Add it to the keyword table in lexer.c.
//   3. Add a handler function in this file.
//   4. Wire it into parser.c's dispatch switch.
//
// TROUBLESHOOTING:
//   - 'WHAT?' on valid syntax: check dialect feature flags.
//   - Crash in expression: ensure error_occurred() is checked
//     after every parse_expression call.
 // ---

#include "parser_internal.h"

 // pi_parse_beep - Handle BEEP command.
void pi_parse_beep(Lexer *lex, RuntimeState *rt, int line_num)
{
  // BEEP - Emit audible bell.
  // BEEP ON - Enable error beep (default).
  // BEEP OFF - Disable error beep.
  // BEEP duration, pitch - Sinclair format.
  //   duration = seconds (float).
  //   pitch = semitones above middle C.
  //   Frequency = 440 * 2^((pitch-9)/12).
  //
  // Routes through vdev_beep() for simple bell
  // or vdev_sound() for frequency-mapped tone.
 if (lexer_match_keyword(lex, KW_ON)) {
  lexer_next(lex);
  error_set_beep(1);
 } else if (lex->current.type ==
  TOK_NAMED_VAR &&
  lex->current.str_length == 3 &&
  (lex->current.str_start[0] == 'O' ||
   lex->current.str_start[0] == 'o') &&
  (lex->current.str_start[1] == 'F' ||
   lex->current.str_start[1] == 'f') &&
  (lex->current.str_start[2] == 'F' ||
   lex->current.str_start[2] == 'f')) {
  lexer_next(lex);
  error_set_beep(0);
 } else if (lex->current.type == TOK_EOF ||
  lex->current.type == TOK_CR ||
  lex->current.type == TOK_COLON) {
  // Simple BEEP with no args
  vdev_beep();
 } else {
   // BEEP duration, pitch
   // duration in seconds, pitch in semitones.
   // Frequency = 440 * 2^((pitch-9)/12)
  double dur_sec, pitch, freq;
  int dur_ms, freq_hz;
  BValue dv;
  dv = parse_expression_bval(
   lex, rt, line_num);
  if (error_occurred()) return;
  dur_sec = bval_to_float(&dv);
  if (lex->current.type != TOK_COMMA) {
    // BEEP n: repeat n times
    {
     int bcount = (int)dur_sec;
     int bi;
     if (bcount < 1) bcount = 1;
     if (bcount > 100) bcount = 100;
     for (bi = 0; bi < bcount; bi++)
      vdev_beep();
    }
   return;
  }
  lexer_next(lex); // consume comma
  {
   BValue pv;
   pv = parse_expression_bval(
    lex, rt, line_num);
   if (error_occurred()) return;
   pitch = bval_to_float(&pv);
  }
  // Calculate frequency
  freq = 440.0 * pow(2.0,
   (pitch - 9.0) / 12.0);
  freq_hz = (int)(freq + 0.5);
  if (freq_hz < 37) freq_hz = 37;
  if (freq_hz > 32767) freq_hz = 32767;
  dur_ms = (int)(dur_sec * 1000.0);
  if (dur_ms < 1) dur_ms = 1;
  if (dur_ms > 30000) dur_ms = 30000;
  vdev_sound(freq_hz, dur_ms);
 }
 return;
}

 // pi_parse_sound - Handle SOUND command.
void pi_parse_sound(Lexer *lex, RuntimeState *rt, int line_num)
{
 // SOUND freq, duration
 //
 // GW-BASIC: freq is Hz (37-32767).
 // duration is in clock ticks at 18.2
 // ticks per second. We convert to ms:
 // ms = ticks * 1000 / 18.2
 // = ticks * 55 (approximately)
 //
 // SOUND 0,n is a pause (silence).
 {
 int freq, dur_ticks, dur_ms;
 freq = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type != TOK_COMMA) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 dur_ticks = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 dur_ms = dur_ticks * 55;
 if (freq <= 0) {
 // Silence: just pause
 vdev_sleep(dur_ms);
 } else {
 vdev_sound(freq, dur_ms);
 }
 }
 return;
}

 // pi_parse_play - Handle PLAY command.
void pi_parse_play(Lexer *lex, RuntimeState *rt, int line_num)
{
 // PLAY string$ - Music macro language.
 //
 // GW-BASIC music commands:
 // C D E F G A B - notes
 // # or + - sharp
 // - - flat
 // O0-O6 - set octave (default 4)
 // > - octave up
 // < - octave down
 // L1-L64 - set note length (default 4)
 // T32-T255 - set tempo (default 120)
 // Pn - pause for length n
 // Nn - play note by number (0-84)
 // . - dot (extend 1.5x)
 // MN/ML/MS - music normal/legato/staccato
 // (ignored - we play full length)
 {
 // Note frequency table: semitone
 // frequencies for octave 4 (middle).
 // Index: 0=C 1=C# 2=D 3=D# 4=E 5=F
 // 6=F# 7=G 8=G# 9=A 10=A# 11=B
 static const int note_freq4[] = {
 262, 277, 294, 311, 330, 349,
 370, 392, 415, 440, 466, 494
 };
 // Map note letter to semitone index.
 // C=0 D=2 E=4 F=5 G=7 A=9 B=11
 static const int note_semi[] = {
 9, 11, 0, 2, 4, 5, 7
 }; // A B C D E F G

 const char *s;
 int slen, si;
 int octave = 4;
 int note_len = 4; // quarter note
 int tempo = 120; // BPM

 // Get the music string
 if (lex->current.type != TOK_STRING) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 s = lex->current.str_start;
 slen = lex->current.str_length;
 lexer_next(lex);

 // Parse the music string
 for (si = 0; si < slen; si++) {
 char ch;
 ch = s[si];
 // Uppercase
 if (ch >= 'a' && ch <= 'z')
 ch = (char)(ch - 32);

 if (ch >= 'A' && ch <= 'G') {
 // Note
 int semi, freq, dur;
 int this_len = note_len;
 int dotted = 0;
 semi = note_semi[ch - 'A'];

 // Check for sharp/flat
 if (si+1 < slen &&
 (s[si+1] == '#' ||
 s[si+1] == '+')) {
 semi++;
 if (semi > 11) semi = 0;
 si++;
 } else if (si+1 < slen &&
 s[si+1] == '-') {
 semi--;
 if (semi < 0) semi = 11;
 si++;
 }

 // Check for length suffix
 if (si+1 < slen &&
 s[si+1] >= '1' &&
 s[si+1] <= '9') {
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num >= 1 && num <= 64)
 this_len = num;
 }

 // Check for dot
 if (si+1 < slen &&
 s[si+1] == '.') {
 dotted = 1;
 si++;
 }

 // Calculate frequency
 freq = note_freq4[semi];
 if (octave < 4) {
 int sh;
 for (sh = 0;
 sh < 4-octave;
 sh++)
 freq /= 2;
 } else if (octave > 4) {
 int sh;
 for (sh = 0;
 sh < octave-4;
 sh++)
 freq *= 2;
 }

 // Calculate duration ms
 // quarter = 60000/tempo ms
 dur = (240000 /
 (tempo * this_len));
 if (dotted)
 dur = dur + dur / 2;

 vdev_sound(freq, dur);

 } else if (ch == 'O') {
 // Set octave
 if (si+1 < slen &&
 s[si+1] >= '0' &&
 s[si+1] <= '6') {
 octave = s[si+1] - '0';
 si++;
 }
 } else if (ch == '>') {
 if (octave < 6) octave++;
 } else if (ch == '<') {
 if (octave > 0) octave--;
 } else if (ch == 'L') {
 // Set default note length
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num >= 1 && num <= 64)
 note_len = num;
 } else if (ch == 'T') {
 // Set tempo
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num >= 32 && num <= 255)
 tempo = num;
 } else if (ch == 'P') {
 // Pause
 int plen = note_len;
 int dur;
 if (si+1 < slen &&
 s[si+1] >= '1' &&
 s[si+1] <= '9') {
 int num = 0;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num >= 1 && num <= 64)
 plen = num;
 }
 dur = 240000 /
 (tempo * plen);
 vdev_sleep(dur);
 } else if (ch == 'N') {
 // Note by number 0-84
 int num = 0;
 int freq, oct, semi, dur;
 si++;
 while (si < slen &&
 s[si] >= '0' &&
 s[si] <= '9') {
 num = num*10 +
 (s[si] - '0');
 si++;
 }
 si--;
 if (num == 0) {
 // N0 = rest
 dur = 240000 /
 (tempo * note_len);
 vdev_sleep(dur);
 } else if (num <= 84) {
 num--;
 oct = num / 12;
 semi = num % 12;
 freq = note_freq4[semi];
 if (oct < 4) {
 int sh;
 for (sh = 0;
 sh < 4-oct;
 sh++)
 freq /= 2;
 } else if (oct > 4) {
 int sh;
 for (sh = 0;
 sh < oct-4;
 sh++)
 freq *= 2;
 }
 dur = 240000 /
 (tempo * note_len);
 vdev_sound(freq, dur);
 }
 }
 // M, space, etc: skip
 }
 }
 return;
}

