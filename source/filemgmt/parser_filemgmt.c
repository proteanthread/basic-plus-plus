#ifdef _WIN32
#define PLAT_STRNICMP _strnicmp
#else
#include <strings.h>
#define PLAT_STRNICMP strncasecmp
#endif
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
#include "console.h"

 // pi_parse_files - Handle FILES command.

static int is_basic_ext(const char *name) {
    const char *ext = strrchr(name, '.');
    if (!ext) return 0;
    char e[5];
    int i;
    for (i = 0; i < 4 && ext[i]; i++) {
        e[i] = (char)tolower((unsigned char)ext[i]);
    }
    e[i] = '\0';
    if (strcmp(e, ".bas") == 0 || strcmp(e, ".bpl") == 0 ||
        strcmp(e, ".bpp") == 0 || strcmp(e, ".bpe") == 0) return 1;
    return 0;
}

void pi_parse_files(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)rt;
    (void)line_num;
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
 gw_printf("\n");
#ifdef _WIN32
 {
 WIN32_FIND_DATAA fd;
 HANDLE hFind;
 int col = 0;
 if (!have_pat) strcpy(pattern, "*");
 hFind = FindFirstFileA(pattern, &fd);
 if (hFind == INVALID_HANDLE_VALUE) {
 gw_printf("No files found.\n");
 } else {
 do {
 if (fd.dwFileAttributes &
 FILE_ATTRIBUTE_DIRECTORY) {
 gw_printf("%-12s <DIR> ", fd.cFileName);
 } else {
 gw_printf("%-18s ", fd.cFileName);
 }
 col++;
 if (col >= 4) {
 gw_printf("\n");
 col = 0;
 }
 } while (FindNextFileA(hFind, &fd));
 FindClose(hFind);
 if (col > 0) gw_printf("\n");
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
 gw_printf("No files found.\n");
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
 gw_printf("%-12s <DIR> ", ep->d_name);
 } else {
 gw_printf("%-18s ", ep->d_name);
 }
 } else {
 gw_printf("%-18s ", ep->d_name);
 }
 col++;
 if (col >= 4) {
 gw_printf("\n");
 col = 0;
 }
 }
 closedir(dp);
 if (col > 0) gw_printf("\n");
 }
 }
#endif
 gw_printf("\n");
 }
 return;
}

 // pi_parse_dir - Handle DIR command.
void pi_parse_dir(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)rt;
    (void)line_num;
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
 gw_printf("\n");
#ifdef _WIN32
 {
 WIN32_FIND_DATAA fd;
 HANDLE hFind;
 int col = 0;
 if (!have_pat) strcpy(pattern, "*");
 hFind = FindFirstFileA(pattern, &fd);
 if (hFind == INVALID_HANDLE_VALUE) {
 gw_printf("No files found.\n");
 } else {
 do {
 if (have_pat || !is_basic_ext(fd.cFileName)) {
 if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
 gw_printf("%-12s <DIR> ", fd.cFileName);
 } else {
 gw_printf("%-18s ", fd.cFileName);
 }
 col++;
 if (col >= 4) {
 gw_printf("\n");
 col = 0;
 }
 }
 } while (FindNextFileA(hFind, &fd));
 FindClose(hFind);
 if (col > 0) gw_printf("\n");
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
 gw_printf("No files found.\n");
 } else {
 while ((ep = readdir(dp)) != NULL) {
 char fullpath[560];
 if (ep->d_name[0] == '.' &&
 (ep->d_name[1] == '\0' ||
 (ep->d_name[1] == '.' &&
 ep->d_name[2] == '\0')))
 continue;
 if (have_pat || !is_basic_ext(ep->d_name)) {
 sprintf(fullpath, "%s/%s", dirpath, ep->d_name);
 if (stat(fullpath, &st) == 0) {
 if (S_ISDIR(st.st_mode)) {
 gw_printf("%-12s <DIR> ", ep->d_name);
 } else {
 gw_printf("%-18s ", ep->d_name);
 }
 } else {
 gw_printf("%-18s ", ep->d_name);
 }
 col++;
 if (col >= 4) {
 gw_printf("\n");
 col = 0;
 }
 }
 }
 closedir(dp);
 if (col > 0) gw_printf("\n");
 }
 }
#endif
 gw_printf("\n");
 }
 return;
}

 // pi_parse_kill - Handle KILL command.
void pi_parse_kill(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)rt;
 // KILL "filename" - Delete a file (GW-BASIC).
 // Auto-appends .BAS if no extension given.
 {
 char fname[260];
 char resolved[260];
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

 if (vfs_resolve(fname, resolved, sizeof(resolved), 1) == 0) {
  if (remove(resolved) != 0) {
   gw_printf("File not found: %s\n", fname);
  }
 } else {
  gw_printf("File not found: %s\n", fname);
 }
 }
 return;
}

 // pi_parse_scratch - Handle SCRATCH command.
void pi_parse_scratch(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)rt;
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
 gw_printf("File not found: %s\n",
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
 gw_printf("UNSAVE: No saved file to delete.\n");
 return;
 }
 if (remove(rt->last_save_file) != 0) {
 gw_printf("File not found: %s\n",
 rt->last_save_file);
 } else {
 gw_printf("Deleted: %s\n",
 rt->last_save_file);
 }
 rt->last_save_file[0] = '\0';
 return;
}

 // pi_parse_copy - Handle COPY command.
void pi_parse_copy(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)rt;
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
 gw_printf("File not found: %s\n",
 src);
 return;
 }
 // Open dest for binary writing
 fout = fopen(dst, "wb");
 if (fout == NULL) {
 fclose(fin);
 gw_printf("Cannot create: %s\n",
 dst);
 return;
 }
 // Copy in 4KB chunks
 while ((n = fread(buf, 1,
 sizeof(buf), fin)) > 0) {
 if (fwrite(buf, 1, n, fout) != n) {
 gw_printf("Write error: %s\n",
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
    (void)rt;
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
 gw_printf("File not found: %s\n",
 src);
 return;
 }
 fout = fopen(dst, "wb");
 if (fout == NULL) {
 fclose(fin);
 gw_printf("Cannot create: %s\n",
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
 gw_printf("Move failed: %s\n",
 src);
 }
 }
 }
 return;
}

 // pi_parse_pwd - Handle PWD command.
void pi_parse_pwd(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex;
    (void)rt;
    (void)line_num;
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
 gw_printf("%s\n", cwd);
 } else {
 gw_printf("?\n");
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
    (void)rt;
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
    (void)rt;
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
    (void)rt;
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
 gw_printf("File not found: %s\n",
 old_name);
 }
 }
 return;
}

 // pi_parse_rename - Handle RENAME command.
void pi_parse_rename(Lexer *lex, RuntimeState *rt, int line_num)
{
    if (rt->running) {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    // Syntax: RENAME [SUB|FUNCTION|LABEL|VAR|ARRAY|MATRIX] <old_name> [AS] <new_name>
    int obj_type = 0; // 1=SUB, 2=FUNCTION, 3=LABEL, 4=VAR/ARRAY/MATRIX, 0=Any

    // Optional Type specifier
    if (lex->current.type == TOK_KEYWORD) {
        if (lex->current.value.keyword == KW_SUB) { obj_type = 1; lexer_next(lex); }
        else if (lex->current.value.keyword == KW_FUNCTION) { obj_type = 2; lexer_next(lex); }
    } else if (lex->current.type == TOK_NAMED_VAR) {
        if (PLAT_STRNICMP(lex->current.str_start, "LABEL", lex->current.str_length) == 0 && lex->current.str_length == 5) { obj_type = 3; lexer_next(lex); }
        else if (PLAT_STRNICMP(lex->current.str_start, "VAR", lex->current.str_length) == 0 && lex->current.str_length == 3) { obj_type = 4; lexer_next(lex); }
        else if (PLAT_STRNICMP(lex->current.str_start, "ARRAY", lex->current.str_length) == 0 && lex->current.str_length == 5) { obj_type = 4; lexer_next(lex); }
        else if (PLAT_STRNICMP(lex->current.str_start, "MATRIX", lex->current.str_length) == 0 && lex->current.str_length == 6) { obj_type = 4; lexer_next(lex); }
    }

    // Get old_name
    char old_name[256];
    if (lex->current.type == TOK_NAMED_VAR || lex->current.type == TOK_STRING_VAR) {
        int olen = lex->current.str_length;
        if (olen > 255) olen = 255;
        memcpy(old_name, lex->current.str_start, olen);
        old_name[olen] = '\0';
    } else if (lex->current.type == TOK_VARIABLE) {
        old_name[0] = lex->current.value.var_name;
        old_name[1] = '\0';
    } else {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    lexer_next(lex);

    // Optional AS
    if (lex->current.type == TOK_KEYWORD && lex->current.value.keyword == KW_AS) {
        lexer_next(lex);
    }

    // Get new_name
    char new_name[256];
    if (lex->current.type == TOK_NAMED_VAR || lex->current.type == TOK_STRING_VAR) {
        int nlen = lex->current.str_length;
        if (nlen > 255) nlen = 255;
        memcpy(new_name, lex->current.str_start, nlen);
        new_name[nlen] = '\0';
    } else if (lex->current.type == TOK_VARIABLE) {
        new_name[0] = lex->current.value.var_name;
        new_name[1] = '\0';
    } else {
        error_raise(ERR_WHAT, line_num);
        return;
    }
    lexer_next(lex);

    // Now loop over the program and replace occurrences of old_name with new_name
    for (int i = 0; i < rt->program->count; i++) {
        char *src = rt->program->lines[i].text;
        char new_line_buf[4096];
        new_line_buf[0] = '\0';
        
        Lexer lx;
        lexer_init(&lx, src);
        lexer_next(&lx);
        
        int last_pos = 0;
        int buf_pos = 0;
        
        while (lx.current.type != TOK_EOF) {
            int is_match = 0;
            if (lx.current.type == TOK_NAMED_VAR || lx.current.type == TOK_STRING_VAR || lx.current.type == TOK_VARIABLE || (lx.current.type == TOK_KEYWORD && obj_type == 1 && lx.current.value.keyword == KW_SUB) || (lx.current.type == TOK_KEYWORD && obj_type == 2 && lx.current.value.keyword == KW_FUNCTION)) {
                int match_len = 0;
                const char* match_str = NULL;
                char vname[2];
                if (lx.current.type == TOK_VARIABLE) {
                    vname[0] = lx.current.value.var_name;
                    vname[1] = '\0';
                    match_str = vname;
                    match_len = 1;
                } else if (lx.current.type == TOK_KEYWORD) {
                    match_str = (lx.current.value.keyword == KW_SUB) ? "SUB" : "FUNCTION";
                    match_len = (int)strlen(match_str);
                } else {
                    match_str = lx.current.str_start;
                    match_len = lx.current.str_length;
                }

                if ((int)strlen(old_name) == match_len && PLAT_STRNICMP(match_str, old_name, match_len) == 0) {
                    is_match = 1;
                }
            }
            if (is_match) {
                const char* tk_start = (lx.current.type == TOK_VARIABLE || lx.current.type == TOK_KEYWORD) ? (src + last_pos) : lx.current.str_start; // Approximate start for simple tokens
                if (lx.current.type == TOK_VARIABLE) {
                    // find variable in source since str_start is not set for TOK_VARIABLE
                    const char *p = src + last_pos;
                    while (*p && toupper(*p) != lx.current.value.var_name) p++;
                    tk_start = p;
                }
                
                int prefix_len = (int)(tk_start - (src + last_pos));
                if (prefix_len > 0) {
                    memcpy(new_line_buf + buf_pos, src + last_pos, prefix_len);
                    buf_pos += prefix_len;
                }
                int n_len = (int)strlen(new_name);
                memcpy(new_line_buf + buf_pos, new_name, n_len);
                buf_pos += n_len;
                
                int tk_len = (lx.current.type == TOK_VARIABLE) ? 1 : ((lx.current.type == TOK_KEYWORD) ? (lx.current.value.keyword == KW_SUB ? 3 : 8) : lx.current.str_length);
                if (lx.current.type == TOK_STRING_VAR && tk_len < (int)strlen(old_name)) tk_len++; // Adjust for $
                
                last_pos = (int)((tk_start - src) + tk_len);
            }
            lexer_next(&lx);
        }
        strcpy(new_line_buf + buf_pos, src + last_pos);
        
        if (strcmp(src, new_line_buf) != 0) {
            free(rt->program->lines[i].text);
            rt->program->lines[i].text = plat_strdup(new_line_buf);
        }
    }
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

