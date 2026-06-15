/*
 * ---
 * BASIC++ Interpreter - parser_blockio.c
 * ---
 *
 * Block/random I/O & record field commands.
 *
 * FIELD, GET, PUT, LSET, RSET, SEEK, LOCK,
 * UNLOCK, IOCTL.
 *
 * ---
 */

#include "parser_internal.h"

/*
 * pi_parse_field - Handle FIELD command.
 */
void pi_parse_field(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * FIELD #n, width AS var$ [, width AS var$]
 * Map string variables to positions in
 * the random-access file record buffer.
 */
 {
 int chan;
 int offset = 0;
 FieldMap flds[MAX_FIELD_MAPS];
 int fcount = 0;

 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);

 /* Parse width AS var$ pairs */
 while (fcount < MAX_FIELD_MAPS &&
 lex->current.type != TOK_EOF &&
 lex->current.type != TOK_CR &&
 lex->current.type != TOK_COLON) {
 int w;
 w = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 /* Expect AS */
 if (!lexer_match_keyword(
 lex, KW_AS)) {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 lexer_next(lex);
 /* Get var name (string var) */
 if (lex->current.type ==
 TOK_STRING_VAR) {
 flds[fcount].name[0] =
 lex->current
 .value.var_name;
 flds[fcount].name_len = 1;
 } else if (lex->current.type ==
 TOK_NAMED_VAR) {
 int nl =
 lex->current.str_length;
 if (nl > 30) nl = 30;
 memcpy(flds[fcount].name,
 lex->current.str_start,
 (size_t)nl);
 flds[fcount].name_len = nl;
 } else {
 error_raise(ERR_WHAT,
 line_num);
 return;
 }
 lexer_next(lex);
 flds[fcount].offset = offset;
 flds[fcount].width = w;
 offset += w;
 fcount++;
 if (lex->current.type ==
 TOK_COMMA)
 lexer_next(lex);
 }
 fileio_set_field(chan, flds,
 fcount, line_num);
 }
 return;
}

/*
 * pi_parse_get - Handle GET command.
 */
void pi_parse_get(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * GET #n [, record]
 * Read a record from a random-access
 * file, or read bytes from BINARY.
 *
 * GET (x1,y1)-(x2,y2), arrayname
 * Capture a graphics screen region
 * (graphics stub).
 */
 if (lex->current.type == TOK_HASH ||
 lex->current.type == TOK_NUMBER) {
 int chan;
 long rec = 0;
 int got_rec = 0;
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 rec = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 got_rec = 1;
 }
 {
 int cmode =
 fileio_get_channel_mode(
 chan);
 if (cmode == FCHAN_RANDOM) {
 if (!got_rec) rec = 0;
 fileio_get_record(chan,
 rec > 0 ? rec : 1,
 line_num);
 if (!error_occurred()) {
 /* Copy fields to vars */
 int fi;
 for (fi = 0; fi < 26;
 fi++) {
 char nm[2];
 int flen;
 const char *fv;
 nm[0]=(char)('A'+fi);
 nm[1]='\0';
 fv =
 fileio_get_field_value(
 chan, nm, 1,
 &flen);
 if (fv) {
 char *p =
 strpool_store(
 &rt->strpool,
 fv, flen);
 runtime_set_string_var(
 rt,
 (char)('A'+fi),
 bval_string(
 p, flen));
 }
 }
 }
 } else if (cmode==FCHAN_BINARY){
 /* Binary: GET #n, pos */
 /* Reads 1 byte at pos */
 char bb;
 int nr;
 nr = fileio_get_binary(
 chan,
 got_rec ? rec : 0,
 &bb, 1, line_num);
 (void)nr;
 } else {
 error_raise(ERR_HOW,
 line_num);
 }
 }
 } else {
 /* GET (x,y)-(x,y), array: not supported */
 error_raise(ERR_HOW, line_num);
 return;
 }
 return;
}

/*
 * pi_parse_put - Handle PUT command.
 */
void pi_parse_put(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * PUT #n [, record]
 * Write a record to a random-access
 * file, or write bytes to BINARY.
 *
 * PUT (x,y), array (graphics stub)
 */
 if (lex->current.type == TOK_HASH ||
 lex->current.type == TOK_NUMBER) {
 int chan;
 long rec = 0;
 int got_rec = 0;
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 rec = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 got_rec = 1;
 }
 {
 int cmode =
 fileio_get_channel_mode(
 chan);
 if (cmode == FCHAN_RANDOM) {
 fileio_put_record(chan,
 rec > 0 ? rec : 1,
 line_num);
 } else if (cmode==FCHAN_BINARY){
 char bb = 0;
 fileio_put_binary(chan,
 got_rec ? rec : 0,
 &bb, 1, line_num);
 } else {
 error_raise(ERR_HOW,
 line_num);
 }
 }
 } else {
 /* PUT (x,y), array: not supported */
 error_raise(ERR_HOW, line_num);
 return;
 }
 return;
}

/*
 * pi_parse_lset - Handle LSET command.
 */
void pi_parse_lset(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * LSET var$ = string$
 * Left-justify string in field buffer.
 * If var is a FIELD variable, pads with
 * spaces to field width. Otherwise acts
 * as plain assignment.
 */
 if (lex->current.type ==
 TOK_STRING_VAR) {
 char sv = lex->current
 .value.var_name;
 BValue val;
 char vn[2];
 int ch;
 lexer_next(lex);
 if (lex->current.type != TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 /* Try all channels for field match */
 vn[0] = sv; vn[1] = '\0';
 for (ch = 1; ch <= 8; ch++) {
 if (fileio_get_channel_mode(ch)
 == FCHAN_RANDOM) {
 int r;
 r = fileio_set_field_value(
 ch, vn, 1,
 bval_is_string(&val) ?
 val.v.sval.data : "",
 bval_is_string(&val) ?
 val.v.sval.length : 0,
 0, line_num);
 if (r == 0) return;
 }
 }
 /* Not a field - plain assignment */
 runtime_set_string_var(rt, sv, val);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;
}

/*
 * pi_parse_rset - Handle RSET command.
 */
void pi_parse_rset(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * RSET var$ = string$
 * Right-justify string in field buffer.
 */
 if (lex->current.type ==
 TOK_STRING_VAR) {
 char sv = lex->current
 .value.var_name;
 BValue val;
 char vn[2];
 int ch;
 lexer_next(lex);
 if (lex->current.type != TOK_EQUALS) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);
 val = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 vn[0] = sv; vn[1] = '\0';
 for (ch = 1; ch <= 8; ch++) {
 if (fileio_get_channel_mode(ch)
 == FCHAN_RANDOM) {
 int r;
 r = fileio_set_field_value(
 ch, vn, 1,
 bval_is_string(&val) ?
 val.v.sval.data : "",
 bval_is_string(&val) ?
 val.v.sval.length : 0,
 1, line_num);
 if (r == 0) return;
 }
 }
 runtime_set_string_var(rt, sv, val);
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 return;
}

/*
 * pi_parse_seek - Handle SEEK command.
 */
void pi_parse_seek(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * SEEK #n, position
 * Set the file position for channel n.
 */
 {
 int chan;
 long pos;
 FILE *fp;

 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
 lexer_next(lex);
 pos = parse_expression(lex, rt,
 line_num);
 if (error_occurred()) return;

 fp = fileio_get_fp(chan);
 if (fp == NULL) {
 error_raise(ERR_HOW, line_num);
 return;
 }
 fseek(fp, pos - 1, SEEK_SET);
 }
 return;
}

/*
 * pi_parse_lock - Handle LOCK command.
 */
void pi_parse_lock(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * LOCK/UNLOCK #n [, record [TO record]]
 * OS-level file locking.
 */
 {
 int is_lock = 1; /* LOCK */
 int chan;
 long s = 1, e = 1;
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 s = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 e = s;
 }
 if (lex->current.type == TOK_KEYWORD
 && lex->current.value.keyword
 == KW_TO) {
 lexer_next(lex);
 e = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (is_lock)
 fileio_lock(chan, s, e,
 line_num);
 else
 fileio_unlock(chan, s, e,
 line_num);
 }
 return;
}

/*
 * pi_parse_unlock - Handle UNLOCK command.
 */
void pi_parse_unlock(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
 * LOCK/UNLOCK #n [, record [TO record]]
 * OS-level file locking.
 */
 {
 int is_lock = 0; /* UNLOCK */
 int chan;
 long s = 1, e = 1;
 if (lex->current.type == TOK_HASH)
 lexer_next(lex);
 chan = (int)parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA) {
 lexer_next(lex);
 s = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 e = s;
 }
 if (lex->current.type == TOK_KEYWORD
 && lex->current.value.keyword
 == KW_TO) {
 lexer_next(lex);
 e = parse_expression(
 lex, rt, line_num);
 if (error_occurred()) return;
 }
 if (is_lock)
 fileio_lock(chan, s, e,
 line_num);
 else
 fileio_unlock(chan, s, e,
 line_num);
 }
 return;
}

/*
 * pi_parse_ioctl - Handle IOCTL command.
 *
 * IOCTL #ch, cmd$             - send command to device
 * IOCTL #ch, cmd$, arg$       - send command with argument
 *
 * Routes the command to the channel's VDev ioctl handler.
 * For FujiNet N: device, maps string commands to FNIO_*:
 *   "JSON_PARSE"    -> FNIO_JSON_PARSE
 *   "JSON_QUERY"    -> FNIO_JSON_QUERY
 *   "HTTP_POST"     -> FNIO_HTTP_POST
 *   "HTTP_PUT"      -> FNIO_HTTP_PUT
 *   "HTTP_DELETE"   -> FNIO_HTTP_DELETE
 *   "TRANSLATION"   -> FNIO_SET_TRANSLATION
 *   "HEADER"        -> FNIO_HTTP_SET_HEADER
 *   "SEARCH"        -> VDIO_ENUMERATE (for UPNP:)
 *   "RESET"         -> VDIO_RESET
 */
void pi_parse_ioctl(Lexer *lex, RuntimeState *rt, int line_num)
{
 int chan;
 BValue cmd_val;
 BValue arg_val;
 int has_arg = 0;
 const char *cmd;
 VDev *dev;

 if (lex->current.type == TOK_HASH)
  lexer_next(lex);
 chan = (int)parse_expression(lex, rt, line_num);
 if (error_occurred()) return;
 if (lex->current.type == TOK_COMMA)
  lexer_next(lex);

 /* Parse command string */
 cmd_val = parse_expression_bval(lex, rt, line_num);
 if (error_occurred()) return;

 /* Optional argument */
 if (lex->current.type == TOK_COMMA) {
  lexer_next(lex);
  arg_val = parse_expression_bval(lex, rt, line_num);
  if (error_occurred()) return;
  has_arg = 1;
 }

 if (!bval_is_string(&cmd_val)) {
  error_raise(ERR_WHAT, line_num);
  return;
 }
 cmd = cmd_val.v.sval.data;

 /* Get channel VDev */
 dev = fileio_get_channel_vdev(chan);
 if (dev == NULL) {
  /* No VDev on this channel - accept silently
   * (GW-BASIC compatibility: IOCTL is a no-op
   * for standard file channels). */
  return;
 }

 if (dev->dev_ioctl == NULL) {
  /* Device doesn't support ioctl */
  return;
 }

 /* Map command string to ioctl code and dispatch */
 {
 int ioctl_cmd = VDIO_USER;
 void *ioctl_arg = NULL;
 char arg_buf[512];

 /* Prepare arg buffer */
 if (has_arg && bval_is_string(&arg_val)) {
  int alen = arg_val.v.sval.length;
  if (alen > 511) alen = 511;
  memcpy(arg_buf, arg_val.v.sval.data,
   (size_t)alen);
  arg_buf[alen] = '\0';
  ioctl_arg = arg_buf;
 } else if (has_arg) {
  int nval = bval_to_int(&arg_val);
  ioctl_arg = (void *)(long)nval;
 }

 /* Map string commands */
 if (pi_str_case_equal(cmd, "SEARCH") ||
     pi_str_case_equal(cmd, "ENUMERATE"))
  ioctl_cmd = VDIO_ENUMERATE;
 else if (pi_str_case_equal(cmd, "RESET"))
  ioctl_cmd = VDIO_RESET;
 else if (pi_str_case_equal(cmd, "JSON_PARSE"))
  ioctl_cmd = 256;  /* FNIO_JSON_PARSE */
 else if (pi_str_case_equal(cmd, "JSON_QUERY"))
  ioctl_cmd = 257;  /* FNIO_JSON_QUERY */
 else if (pi_str_case_equal(cmd, "CHANNEL_MODE"))
  ioctl_cmd = 258;  /* FNIO_SET_CHANNEL_MODE */
 else if (pi_str_case_equal(cmd, "TRANSLATION"))
  ioctl_cmd = 259;  /* FNIO_SET_TRANSLATION */
 else if (pi_str_case_equal(cmd, "HEADER") ||
          pi_str_case_equal(cmd, "HTTP_HEADER"))
  ioctl_cmd = 263;  /* FNIO_HTTP_SET_HEADER */
 else if (pi_str_case_equal(cmd, "HTTP_POST"))
  ioctl_cmd = 265;  /* FNIO_HTTP_POST */
 else if (pi_str_case_equal(cmd, "HTTP_PUT"))
  ioctl_cmd = 266;  /* FNIO_HTTP_PUT */
 else if (pi_str_case_equal(cmd, "HTTP_DELETE"))
  ioctl_cmd = 267;  /* FNIO_HTTP_DELETE */

 dev->dev_ioctl(dev, ioctl_cmd, ioctl_arg);
 }
 return;
}
