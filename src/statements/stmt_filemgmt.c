/**
 * @file stmt_filemgmt.c
 * @brief Statement handlers for directory and file system management.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements FILES, KILL, CHDIR, MKDIR, RMDIR, and NAME AS statements.
 * - Why it exists: Provides users with shell-like utilities to list, create, delete,
 *   rename, and navigate directories on the host filesystem.
 * - Why it works this way: It evaluates expression paths and delegates execution to
 *   cross-platform abstractions declared in bpp_platform.h.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Default listing formats or output alignments.
 * - What cannot be changed: Expected return values indicating OS error status.
 * - What to expect: Invalid paths trigger runtime error codes (e.g. Permission Denied).
 * - What to do if something breaks: If a directory operation fails, verify read/write
 *   permissions for the current working directory.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: OS filesystem has a hierarchical directory tree structure.
 * - Portability concerns: None. C17 standard compliant.
 */

#include "bpp_stmt.h"
#include "bpp_platform.h"
#include "bpp_eval.h"
#include "bpp_vdev.h"
#include "bpp_security.h"
#include "bpp_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief FILES [pattern$]
 */
BppError stmt_files_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (security_check(SECOP_FILE_MGMT, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    const char *pattern = "*";
    BValue pat_val = {0};
    pat_val.type = VAL_NONE;

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        pat_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (pat_val.type != VAL_STRING) {
            err.code = 13; err.message = "FILES expects a string pattern";
            return err;
        }
        pattern = str_data(pat_val.as.string);
    }

    platform_list_files(vm_get_vdev(vm), pattern);

    if (pat_val.type == VAL_STRING) {
        str_release(vm_get_str(vm), pat_val.as.string);
    }
    return err;
}

/**
 * @brief KILL "filename"
 */
BppError stmt_kill_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (security_check(SECOP_FILE_MGMT, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    BValue file_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (file_val.type != VAL_STRING) {
        err.code = 13; err.message = "KILL expects a string filename";
        return err;
    }

    const char *filename = str_data(file_val.as.string);
    int rc = 0;
    if (bpp_logger_is_dry_run()) {
        bpp_log_warn("[DRY-RUN] Intercepted KILL statement for: %s (no-op)", filename);
    } else {
        rc = platform_remove(filename);
    }
    str_release(vm_get_str(vm), file_val.as.string);

    if (rc != 0) {
        err.code = 53; err.message = "File not found or access denied";
    }
    return err;
}

/**
 * @brief CHDIR "path"
 */
BppError stmt_chdir_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (security_check(SECOP_FILE_MGMT, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    BValue path_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (path_val.type != VAL_STRING) {
        err.code = 13; err.message = "CHDIR expects a string path";
        return err;
    }

    const char *path = str_data(path_val.as.string);
    int rc = platform_chdir(path);
    str_release(vm_get_str(vm), path_val.as.string);

    if (rc != 0) {
        err.code = 76; err.message = "Path not found";
    }
    return err;
}

/**
 * @brief MKDIR "path"
 */
BppError stmt_mkdir_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (security_check(SECOP_FILE_MGMT, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    BValue path_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (path_val.type != VAL_STRING) {
        err.code = 13; err.message = "MKDIR expects a string path";
        return err;
    }

    const char *path = str_data(path_val.as.string);
    int rc = 0;
    if (bpp_logger_is_dry_run()) {
        bpp_log_warn("[DRY-RUN] Intercepted MKDIR statement for: %s (no-op)", path);
    } else {
        rc = platform_mkdir(path);
    }
    str_release(vm_get_str(vm), path_val.as.string);

    if (rc != 0) {
        err.code = 75; err.message = "Path/File access error";
    }
    return err;
}

/**
 * @brief RMDIR "path"
 */
BppError stmt_rmdir_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (security_check(SECOP_FILE_MGMT, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    BValue path_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (path_val.type != VAL_STRING) {
        err.code = 13; err.message = "RMDIR expects a string path";
        return err;
    }

    const char *path = str_data(path_val.as.string);
    int rc = 0;
    if (bpp_logger_is_dry_run()) {
        bpp_log_warn("[DRY-RUN] Intercepted RMDIR statement for: %s (no-op)", path);
    } else {
        rc = platform_rmdir(path);
    }
    str_release(vm_get_str(vm), path_val.as.string);

    if (rc != 0) {
        err.code = 76; err.message = "Path not found or access denied";
    }
    return err;
}

/**
 * @brief NAME "oldname" AS "newname"
 */
BppError stmt_name_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (security_check(SECOP_FILE_MGMT, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    BValue old_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (old_val.type != VAL_STRING) {
        err.code = 13; err.message = "NAME expects string parameters";
        return err;
    }

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD || tok.as.keyword != KW_AS) {
        err.code = 2; err.message = "Expected AS keyword in NAME statement";
        str_release(vm_get_str(vm), old_val.as.string);
        return err;
    }

    BValue new_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        str_release(vm_get_str(vm), old_val.as.string);
        return err;
    }
    if (new_val.type != VAL_STRING) {
        err.code = 13; err.message = "NAME expects string parameters";
        str_release(vm_get_str(vm), old_val.as.string);
        return err;
    }

    const char *oldname = str_data(old_val.as.string);
    const char *newname = str_data(new_val.as.string);
    int rc = 0;
    if (bpp_logger_is_dry_run()) {
        bpp_log_warn("[DRY-RUN] Intercepted NAME rename statement from %s to %s (no-op)", oldname, newname);
    } else {
        rc = platform_rename(oldname, newname);
    }

    str_release(vm_get_str(vm), old_val.as.string);
    str_release(vm_get_str(vm), new_val.as.string);

    if (rc != 0) {
        err.code = 58; err.message = "File rename failed";
    }
    return err;
}

/**
 * @brief DEVICES
 */
BppError stmt_devices_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    vdev_list_all(vm_get_vdev(vm));
    return err;
}

BppError stmt_unsave_handler(VMContext *vm, LexerContext *lex) {
    return stmt_kill_handler(vm, lex);
}

static int is_basic_ext(const char *ext) {
    if (!ext) return 0;
    char uext[8] = {0};
    for(int i = 0; i < 7 && ext[i]; i++) {
        char c = ext[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        uext[i] = c;
    }
    if (strcmp(uext, ".BAS") == 0) return 1;
    if (strcmp(uext, ".BPL") == 0) return 1;
    if (strcmp(uext, ".BPP") == 0) return 1;
    if (strcmp(uext, ".BPE") == 0) return 1;
    return 0;
}

BppError stmt_dir_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (security_check(SECOP_FILE_MGMT, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }
    char name[256];
    BppDirSearch *search = platform_find_first_file("*", name, sizeof(name));
    if (!search) {
        return err;
    }
    do {
        const char *ext = strrchr(name, '.');
        if (ext && is_basic_ext(ext)) {
            vdev_printf(vm_get_vdev(vm), "%s\n", name);
        }
    } while (platform_find_next_file(search, name, sizeof(name)));
    platform_find_close(search);
    return err;
}

BppError stmt_setattr_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (security_check(SECOP_FILE_MGMT, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }
    BValue path_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (path_val.type != VAL_STRING) {
        if (path_val.type == VAL_STRING) str_release(vm_get_str(vm), path_val.as.string);
        err.code = 13; err.message = "Type mismatch (expected path)";
        return err;
    }
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        str_release(vm_get_str(vm), path_val.as.string);
        err.code = 2; err.message = "Expected comma";
        return err;
    }
    BValue attr_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        str_release(vm_get_str(vm), path_val.as.string);
        return err;
    }
    int mask = 0;
    if (attr_val.type == VAL_NUMBER || attr_val.type == VAL_INTEGER) {
        mask = (int)attr_val.as.number;
    } else {
        err.code = 13; err.message = "Type mismatch";
    }
    
    if (err.code == 0) {
        const char *path = str_data(path_val.as.string);
        int rc = platform_set_attributes(path, mask);
        if (!rc) {
            err.code = 53; err.message = "File not found or access denied";
        }
    }
    
    str_release(vm_get_str(vm), path_val.as.string);
    if (attr_val.type == VAL_STRING) str_release(vm_get_str(vm), attr_val.as.string);
    return err;
}
