/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_filemgmt.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    File system directory operations (FILES, DIR, KILL, SCRATCH, COPY, MOVE, PWD, CHDIR, MKDIR, RMDIR).
 *
 * 2. WHAT TO EXPECT:
 *    Translates BASIC system actions into platform-specific directory routines safely.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Default file filter patterns, directory listing layouts.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Path separators validations, file deletion security gates.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Verify that host OS permissions permit write/delete access. Ensure path lengths do not exceed compile constants.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_filemgmt.c
 // ---
 //
 // File management & directory navigation.
 //
 // FILES, DIR, KILL, SCRATCH, UNSAVE, COPY, MOVE,
 // PWD, CHDIR, MKDIR, RMDIR, NAME, RENAME, ERASE.
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

 // pi_parse_files - Handle FILES command.
void pi_parse_files(Lexer *lex, RuntimeState *rt, int line_num)
{
 // FILES [pattern] - List directory (detailed).
 //
 // Native dir scanning -- no system() call.
 // Shows filename, size, and <DIR> markers.
 // With no argument, lists current dir.
 // With a string argument, uses it as a
 // wildcard pattern (e.g. FILES "*.BAS")
 {
 char pattern[280];
 int have_pat = 0;
 if (lex->current.type == TOK_STRING
 && lex->current.str_start != NULL
 && lex->current.str_length > 0) {
 int sl = lex->current.str_length;
 if (sl > 259) sl = 259;
 memcpy(pattern,
 lex->current.str_start,
 (size_t)sl);
 pattern[sl] = '\0';
 have_pat = 1;
 lexer_next(lex);
 }
 printf("\n");
#ifdef _WIN32
 {
 WIN32_FIND_DATAA fd;
 HANDLE hFind;
 int col = 0;
 if (!have_pat) strcpy(pattern, "*");
 hFind = FindFirstFileA(pattern, &fd);
 if (hFind == INVALID_HANDLE_VALUE) {
 printf("No files found.\n");
 } else {
 do {
 if (fd.dwFileAttributes &
 FILE_ATTRIBUTE_DIRECTORY) {
 printf("%-14s <DIR>  ", fd.cFileName);
 } else {
 printf("%-14s %7lu  ",
 fd.cFileName,
 (unsigned long)fd.nFileSizeLow);
 }
 col++;
 if (col >= 3) {
 printf("\n");
 col = 0;
 }
 } while (FindNextFileA(hFind, &fd));
 FindClose(hFind);
 if (col > 0) printf("\n");
 }
 }
#else
 {
 DIR *dp;
 struct dirent *ep;
 struct stat st;
 int col = 0;
 char dirpath[280];
 if (!have_pat) strcpy(pattern, ".");
 // Extract directory from pattern
 strcpy(dirpath, pattern);
 // Simple: if pattern is just "*" or "."
 // or has no path separator, use "." 
 {
 int has_sep = 0, pi;
 for (pi = 0; dirpath[pi]; pi++) {
 if (dirpath[pi] == '/')
 has_sep = 1;
 }
 if (!has_sep) strcpy(dirpath, ".");
 }
 dp = opendir(dirpath);
 if (dp == NULL) {
 printf("No files found.\n");
 } else {
 while ((ep = readdir(dp)) != NULL) {
 char fullpath[560];
 // Skip . and ..
 if (ep->d_name[0] == '.' &&
 (ep->d_name[1] == '\0' ||
 (ep->d_name[1] == '.' &&
 ep->d_name[2] == '\0')))
 continue;
 sprintf(fullpath, "%s/%s",
 dirpath, ep->d_name);
 if (stat(fullpath, &st) == 0) {
 if (S_ISDIR(st.st_mode)) {
 printf("%-14s <DIR>  ",
 ep->d_name);
 } else {
 printf("%-14s %7lu  ",
 ep->d_name,
 (unsigned long)st.st_size);
 }
 } else {
 printf("%-14s        ",
 ep->d_name);
 }
 col++;
 if (col >= 3) {
 printf("\n");
 col = 0;
 }
 }
 closedir(dp);
 if (col > 0) printf("\n");
 }
 }
#endif
 printf("\n");
 }
 return;
}

 // pi_parse_dir - Handle DIR command.
void pi_parse_dir(Lexer *lex, RuntimeState *rt, int line_num)
{
 // DIR [pattern] - List filenames only.
 //
 // Native dir scanning -- no system() call.
 // Shows one filename per line, no sizes.
 {
 char pattern[280];
 int have_pat = 0;
 if (lex->current.type == TOK_STRING
 && lex->current.str_start != NULL
 && lex->current.str_length > 0) {
 int sl = lex->current.str_length;
 if (sl > 259) sl = 259;
 memcpy(pattern,
 lex->current.str_start,
 (size_t)sl);
 pattern[sl] = '\0';
 have_pat = 1;
 lexer_next(lex);
 }
 printf("\n");
#ifdef _WIN32
 {
 WIN32_FIND_DATAA fd;
 HANDLE hFind;
 if (!have_pat) strcpy(pattern, "*");
 hFind = FindFirstFileA(pattern, &fd);
 if (hFind == INVALID_HANDLE_VALUE) {
 printf("No files found.\n");
 } else {
 do {
 printf("%s\n", fd.cFileName);
 } while (FindNextFileA(hFind, &fd));
 FindClose(hFind);
 }
 }
#else
 {
 DIR *dp;
 struct dirent *ep;
 char dirpath[280];
 if (!have_pat) strcpy(pattern, ".");
 strcpy(dirpath, pattern);
 {
 int has_sep = 0, pi;
 for (pi = 0; dirpath[pi]; pi++) {
 if (dirpath[pi] == '/')
 has_sep = 1;
 }
 if (!has_sep) strcpy(dirpath, ".");
 }
 dp = opendir(dirpath);
 if (dp == NULL) {
 printf("No files found.\n");
 } else {
 while ((ep = readdir(dp)) != NULL) {
 if (ep->d_name[0] == '.' &&
 (ep->d_name[1] == '\0' ||
 (ep->d_name[1] == '.' &&
 ep->d_name[2] == '\0')))
 continue;
 printf("%s\n", ep->d_name);
 }
 closedir(dp);
 }
 }
#endif
 printf("\n");
 }
 return;
}

 // pi_parse_kill - Handle KILL command.
void pi_parse_kill(Lexer *lex, RuntimeState *rt, int line_num)
{
 // KILL "filename" - Delete a file (GW-BASIC).
 // Auto-appends .BAS if no extension given.
 {
 char fname[260];
 int flen;

 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 flen = lex->current.str_length;
 if (flen > 255) flen = 255;
 memcpy(fname,
 lex->current.str_start,
 (size_t)flen);
 fname[flen] = '\0';
 lexer_next(lex);

 // Auto-append .BAS if no extension
 pi_ensure_bas_ext(fname, flen, 259);

 if (remove(fname) != 0) {
 printf("File not found: %s\n",
 fname);
 }
 }
 return;
}

 // pi_parse_scratch - Handle SCRATCH command.
void pi_parse_scratch(Lexer *lex, RuntimeState *rt, int line_num)
{
 // SCRATCH filename$ -- SUPER BASIC alias for KILL.
 // Deletes a file from disk.
 {
 char fname[260];
 int flen;

 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 flen = lex->current.str_length;
 if (flen > 259) flen = 259;
 memcpy(fname,
 lex->current.str_start,
 (size_t)flen);
 fname[flen] = '\0';
 lexer_next(lex);

 if (remove(fname) != 0) {
 printf("File not found: %s\n",
 fname);
 }
 }
 return;
}

 // pi_parse_unsave - Handle UNSAVE command.
void pi_parse_unsave(Lexer *lex, RuntimeState *rt, int line_num)
{
 // UNSAVE - Delete the last-saved file.
 // If no file has been saved this session,
 // prints an error message.
 (void)lex; (void)line_num;
 if (rt->last_save_file[0] == '\0') {
 printf("UNSAVE: No saved file to delete.\n");
 return;
 }
 if (remove(rt->last_save_file) != 0) {
 printf("File not found: %s\n",
 rt->last_save_file);
 } else {
 printf("Deleted: %s\n",
 rt->last_save_file);
 }
 rt->last_save_file[0] = '\0';
 return;
}

 // pi_parse_copy - Handle COPY command.
void pi_parse_copy(Lexer *lex, RuntimeState *rt, int line_num)
{
 // COPY "source" TO "dest"
 // Copy a file using binary fopen/fread/fwrite.
 // Pure C89 -- no SHELL needed.
 {
 char src[260], dst[260];
 int slen, dlen;
 FILE *fin, *fout;
 char buf[4096];
 size_t n;

 // Source filename
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 slen = lex->current.str_length;
 if (slen > 259) slen = 259;
 memcpy(src,
 lex->current.str_start,
 (size_t)slen);
 src[slen] = '\0';
 lexer_next(lex);

 // Expect TO keyword
 if (!lexer_match_keyword(lex,
 KW_TO)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 // Destination filename
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 dlen = lex->current.str_length;
 if (dlen > 259) dlen = 259;
 memcpy(dst,
 lex->current.str_start,
 (size_t)dlen);
 dst[dlen] = '\0';
 lexer_next(lex);

 // Open source for binary reading
 fin = fopen(src, "rb");
 if (fin == NULL) {
 printf("File not found: %s\n",
 src);
 return;
 }
 // Open dest for binary writing
 fout = fopen(dst, "wb");
 if (fout == NULL) {
 fclose(fin);
 printf("Cannot create: %s\n",
 dst);
 return;
 }
 // Copy in 4KB chunks
 while ((n = fread(buf, 1,
 sizeof(buf), fin)) > 0) {
 if (fwrite(buf, 1, n, fout) != n) {
 printf("Write error: %s\n",
 dst);
 fclose(fin);
 fclose(fout);
 return;
 }
 }
 fclose(fin);
 fclose(fout);
 }
 return;
}

 // pi_parse_move - Handle MOVE command.
void pi_parse_move(Lexer *lex, RuntimeState *rt, int line_num)
{
 // MOVE "source" TO "dest"
 // Move a file. Tries rename() first;
 // if that fails (cross-device), falls
 // back to copy + delete.
 {
 char src[260], dst[260];
 int slen, dlen;

 // Source filename
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 slen = lex->current.str_length;
 if (slen > 259) slen = 259;
 memcpy(src,
 lex->current.str_start,
 (size_t)slen);
 src[slen] = '\0';
 lexer_next(lex);

 // Expect TO keyword
 if (!lexer_match_keyword(lex,
 KW_TO)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 // Destination filename
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 dlen = lex->current.str_length;
 if (dlen > 259) dlen = 259;
 memcpy(dst,
 lex->current.str_start,
 (size_t)dlen);
 dst[dlen] = '\0';
 lexer_next(lex);

 // Try rename first (fast, same device)
 if (rename(src, dst) != 0) {
 // Fallback: copy + delete
 FILE *fin, *fout;
 char buf[4096];
 size_t n;
 int ok = 1;
 fin = fopen(src, "rb");
 if (fin == NULL) {
 printf("File not found: %s\n",
 src);
 return;
 }
 fout = fopen(dst, "wb");
 if (fout == NULL) {
 fclose(fin);
 printf("Cannot create: %s\n",
 dst);
 return;
 }
 while ((n = fread(buf, 1,
 sizeof(buf), fin)) > 0) {
 if (fwrite(buf, 1, n,
 fout) != n) {
 ok = 0;
 break;
 }
 }
 fclose(fin);
 fclose(fout);
 if (ok) {
 remove(src);
 } else {
 printf("Move failed: %s\n",
 src);
 }
 }
 }
 return;
}

 // pi_parse_pwd - Handle PWD command.
void pi_parse_pwd(Lexer *lex, RuntimeState *rt, int line_num)
{
 // PWD - Print working directory.
 // No arguments. Prints the current
 // working directory path to stdout.
 {
 char cwd[512];
#ifdef _WIN32
 if (_getcwd(cwd, sizeof(cwd)) != NULL)
#else
 if (getcwd(cwd, sizeof(cwd)) != NULL)
#endif
 {
 printf("%s\n", cwd);
 } else {
 printf("?\n");
 }
 }
 return;
}

 // pi_parse_chdir - Handle CHDIR command.
void pi_parse_chdir(Lexer *lex, RuntimeState *rt, int line_num)
{
 // CHDIR path$
 // Change the current working directory.
 {
 BValue pv;
 pv = parse_expression_bval(
 lex, rt, line_num);
 if (error_occurred()) return;
 if (bval_is_string(&pv) &&
 pv.v.sval.data != NULL) {
 char path[512];
 int pl = pv.v.sval.length;
 if (pl > 510) pl = 510;
 memcpy(path, pv.v.sval.data,
 (size_t)pl);
 path[pl] = '\0';
#ifdef _WIN32
 _chdir(path);
#else
 chdir(path);
#endif
 } else {
 error_raise(ERR_WHAT, line_num);
 }
 }
 return;
}

 // pi_parse_mkdir - Handle MKDIR command.
void pi_parse_mkdir(Lexer *lex, RuntimeState *rt, int line_num)
{
 // MKDIR path$
 // Create a directory.
 {
 char dname[260];
 int dlen;

 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 dlen = lex->current.str_length;
 if (dlen > 259) dlen = 259;
 memcpy(dname,
 lex->current.str_start,
 (size_t)dlen);
 dname[dlen] = '\0';
 lexer_next(lex);
#if defined(_WIN32) || defined(__WATCOMC__)
 _mkdir(dname);
#else
 mkdir(dname, 0755);
#endif
 }
 return;
}

 // pi_parse_rmdir - Handle RMDIR command.
void pi_parse_rmdir(Lexer *lex, RuntimeState *rt, int line_num)
{
 // RMDIR path$
 // Remove a directory.
 {
 char dname[260];
 int dlen;

 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 dlen = lex->current.str_length;
 if (dlen > 259) dlen = 259;
 memcpy(dname,
 lex->current.str_start,
 (size_t)dlen);
 dname[dlen] = '\0';
 lexer_next(lex);
#ifdef _WIN32
 _rmdir(dname);
#else
 rmdir(dname);
#endif
 }
 return;
}

 // pi_parse_name - Handle NAME command.
void pi_parse_name(Lexer *lex, RuntimeState *rt, int line_num)
{
 // NAME "oldname" AS "newname"
 // Rename a file (GW-BASIC compatible).
 {
 char old_name[260], new_name[260];
 int olen, nlen;

 // Old filename
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 olen = lex->current.str_length;
 if (olen > 259) olen = 259;
 memcpy(old_name,
 lex->current.str_start,
 (size_t)olen);
 old_name[olen] = '\0';
 lexer_next(lex);

 // Expect AS keyword
 if (!lexer_match_keyword(lex,
 KW_AS)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 // New filename
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 nlen = lex->current.str_length;
 if (nlen > 259) nlen = 259;
 memcpy(new_name,
 lex->current.str_start,
 (size_t)nlen);
 new_name[nlen] = '\0';
 lexer_next(lex);

 if (rename(old_name, new_name) != 0) {
 printf("File not found: %s\n",
 old_name);
 }
 }
 return;
}

 // pi_parse_rename - Handle RENAME command.
void pi_parse_rename(Lexer *lex, RuntimeState *rt, int line_num)
{
 // RENAME "oldname" AS "newname"
 // Like NAME but auto-appends .bas/.bpp
 // extension if not present.
 {
 char old_name[260], new_name[260];
 char old_try[264], new_try[264];
 int olen, nlen;
 int has_ext_o, has_ext_n;
 int ok = 0;

 // Old filename
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 olen = lex->current.str_length;
 if (olen > 255) olen = 255;
 memcpy(old_name,
 lex->current.str_start,
 (size_t)olen);
 old_name[olen] = '\0';
 lexer_next(lex);

 // Expect AS keyword
 if (!lexer_match_keyword(lex,
 KW_AS)) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 lexer_next(lex);

 // New filename
 if (lex->current.type != TOK_STRING
 || lex->current.str_start == NULL
 || lex->current.str_length < 1) {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 nlen = lex->current.str_length;
 if (nlen > 255) nlen = 255;
 memcpy(new_name,
 lex->current.str_start,
 (size_t)nlen);
 new_name[nlen] = '\0';
 lexer_next(lex);

 // Check if names have extensions
 {
 int di;
 has_ext_o = 0;
 has_ext_n = 0;
 for (di = olen - 1; di >= 0;
 di--) {
 if (old_name[di] == '.') {
 has_ext_o = 1;
 break;
 }
 if (old_name[di] == '/' ||
 old_name[di] == '\\')
 break;
 }
 for (di = nlen - 1; di >= 0;
 di--) {
 if (new_name[di] == '.') {
 has_ext_n = 1;
 break;
 }
 if (new_name[di] == '/' ||
 new_name[di] == '\\')
 break;
 }
 }

 // Try with extensions if none given
 if (!has_ext_o && !has_ext_n) {
 // Try .bas first
 sprintf(old_try, "%s.bas",
 old_name);
 sprintf(new_try, "%s.bas",
 new_name);
 if (rename(old_try,
 new_try) == 0) {
 ok = 1;
 } else {
 // Try .bpp
 sprintf(old_try, "%s.bpp",
 old_name);
 sprintf(new_try, "%s.bpp",
 new_name);
 if (rename(old_try,
 new_try) == 0) {
 ok = 1;
 }
 }
 }

 // Fall back to exact names
 if (!ok) {
 if (rename(old_name,
 new_name) != 0) {
 printf("File not found: "
 "%s\n", old_name);
 }
 }
 }
 return;
}

 // pi_parse_erase - Handle ERASE command.
void pi_parse_erase(Lexer *lex, RuntimeState *rt, int line_num)
{
 // ERASE # n - truncate file (ECMA-116)
 // ERASE arrayname [, ...] - clear DIM arrays
 if (lex->current.type == TOK_HASH) {
 pi_parse_erase_file(lex, rt, line_num);
 return;
 }
 {
 for (;;) {
 const char *nm;
 int nlen, di;
 char namebuf[MAX_VAR_NAME_LEN + 1];

 if (lex->current.type ==
 TOK_NAMED_VAR) {
 nm = lex->current.str_start;
 nlen = lex->current.str_length;
 } else if (lex->current.type ==
 TOK_VARIABLE) {
 namebuf[0] = lex->current.value
 .var_name;
 namebuf[1] = '\0';
 nm = namebuf;
 nlen = 1;
 } else {
 error_raise(ERR_WHAT, line_num);
 return;
 }
 // Save name before advancing
 if (nlen > MAX_VAR_NAME_LEN)
 nlen = MAX_VAR_NAME_LEN;
 memcpy(namebuf, nm, (size_t)nlen);
 namebuf[nlen] = '\0';
 nm = namebuf;
 lexer_next(lex);

 // Find and clear the DIM array
 for (di = 0; di < rt->dim_count;
 di++) {
 DimArray *da =
 &rt->dim_arrays[di];
 if ((int)strlen(da->name) ==
 nlen &&
 memcmp(da->name, nm,
 (size_t)nlen) == 0) {
 // Zero out elements
 int j;
 for (j = 0; j < da->total;
 j++) {
 da->elements[j] =
 bval_int(0);
 }
 break;
 }
 }

 if (lex->current.type != TOK_COMMA)
 break;
 lexer_next(lex);
 }
 }
 return;
}

