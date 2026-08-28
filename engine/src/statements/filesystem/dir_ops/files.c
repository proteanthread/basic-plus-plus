// FILENAME: files.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, file.h, file.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, files.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the FILES statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/dir_ops/files.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include "runtime/file.h"
#include "types/errors.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

void stmt_files_register(void) {
    MicroLibMetadata meta = {
        .name = "FILES",
        .category = "Filesystem I/O",
        .syntax = "FILES [filespec]",
        .help_text = "Displays directory listing matching specified file pattern.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}

static bool has_bas_extension(const char *name) {
    if (!name) return false;
    size_t len = strlen(name);
    if (len < 4) return false;
    const char *ext = name + len - 4;
    return (ext[0] == '.' &&
            (ext[1] == 'B' || ext[1] == 'b') &&
            (ext[2] == 'A' || ext[2] == 'a') &&
            (ext[3] == 'S' || ext[3] == 's'));
}

BppError stmt_files_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    char pattern[256] = "*.*";

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF || tok.type == TOK_BACKSLASH) {
        // Standard GW-BASIC directory listing (no arguments)
        char name[260];
        BppDirSearch *search = platform_find_first_file(pattern, name, sizeof(name));
        if (!search) {
            search = platform_find_first_file("*", name, sizeof(name));
        }

        if (!search) {
            vdev_printf(vm_get_vdev(vm), "No files found\n");
            return err;
        }

        int col = 0;
        do {
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
            vdev_printf(vm_get_vdev(vm), "%-18s", name);
            col++;
            if (col >= 4) {
                vdev_printf(vm_get_vdev(vm), "\n");
                col = 0;
            }
        } while (platform_find_next_file(search, name, sizeof(name)));

        platform_find_close(search);
        if (col > 0) {
            vdev_printf(vm_get_vdev(vm), "\n");
        }
        return err;
    }

    // Check if multi-file channel list (HP 2000 / DTSS): starts with '*' or has commas
    bool is_multi_file = (tok.type == TOK_MUL);
    if (!is_multi_file) {
        // Look ahead for comma
        LexerContext *look = lex_init(vm_get_mem(vm), lex_get_pos(lex));
        if (look) {
            eval_expression(vm, look, &err);
            if (lex_peek(look).type == TOK_COMMA) {
                is_multi_file = true;
            }
            lex_shutdown(look);
            memset(&err, 0, sizeof(err));
        }
    }

    if (is_multi_file) {
        int ch = 1;
        while (true) {
            BppToken cur = lex_peek(lex);
            if (cur.type == TOK_MUL) {
                lex_next(lex); // Skip '*' placeholder
            } else if (cur.type != TOK_EOL && cur.type != TOK_EOF && cur.type != TOK_BACKSLASH && cur.type != TOK_COMMA) {
                BValue fval = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (fval.type == VAL_STRING && fval.as.string) {
                    const char *fname = str_data(fval.as.string);
                    if (fname && *fname) {
                        // Bind channel
                        file_open(vm_get_file(vm), vm_get_vdev(vm), ch, fname, FILE_MODE_INPUT, FILE_ACCESS_READ, FILE_LOCK_DEFAULT, 0);
                    }
                    str_release(vm_get_str(vm), fval.as.string);
                }
            }
            ch++;
            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex); // Consume ','
            } else {
                break;
            }
        }
        return err;
    }

    // Single argument: Standard GW-BASIC pattern directory listing
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING && val.as.string) {
        snprintf(pattern, sizeof(pattern), "%s", str_data(val.as.string));
        str_release(vm_get_str(vm), val.as.string);
    }

    char name[260];
    BppDirSearch *search = platform_find_first_file(pattern, name, sizeof(name));
    if (!search) {
        search = platform_find_first_file("*", name, sizeof(name));
    }

    if (!search) {
        vdev_printf(vm_get_vdev(vm), "No files found\n");
        return err;
    }

    int col = 0;
    do {
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        vdev_printf(vm_get_vdev(vm), "%-18s", name);
        col++;
        if (col >= 4) {
            vdev_printf(vm_get_vdev(vm), "\n");
            col = 0;
        }
    } while (platform_find_next_file(search, name, sizeof(name)));

    platform_find_close(search);
    if (col > 0) {
        vdev_printf(vm_get_vdev(vm), "\n");
    }

    return err;
}

BppError stmt_dir_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    char pattern[256] = "*.bas";

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_BACKSLASH) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING && val.as.string) {
            snprintf(pattern, sizeof(pattern), "%s", str_data(val.as.string));
            str_release(vm_get_str(vm), val.as.string);
        }
    }

    char name[260];
    BppDirSearch *search = platform_find_first_file(pattern, name, sizeof(name));
    if (!search) {
        search = platform_find_first_file("*", name, sizeof(name));
    }

    if (!search) {
        vdev_printf(vm_get_vdev(vm), "No .BAS files found\n");
        return err;
    }

    int col = 0;
    int found_count = 0;
    do {
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        // Strict .BAS-exclusive filter (case-insensitive)
        if (!has_bas_extension(name)) continue;

        vdev_printf(vm_get_vdev(vm), "%-18s", name);
        found_count++;
        col++;
        if (col >= 4) {
            vdev_printf(vm_get_vdev(vm), "\n");
            col = 0;
        }
    } while (platform_find_next_file(search, name, sizeof(name)));

    platform_find_close(search);
    if (found_count == 0) {
        vdev_printf(vm_get_vdev(vm), "No .BAS files found\n");
    } else if (col > 0) {
        vdev_printf(vm_get_vdev(vm), "\n");
    }

    return err;
}

BppError stmt_pwd_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    char cwd[512] = {0};
    if (platform_getcwd(cwd, sizeof(cwd))) {
        vdev_printf(vm_get_vdev(vm), "%s\n", cwd);
    } else {
        vdev_printf(vm_get_vdev(vm), ".\n");
    }
    return err;
}

BppError stmt_path_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_BACKSLASH) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING && val.as.string) {
            // Set path
            platform_chdir(str_data(val.as.string));
            str_release(vm_get_str(vm), val.as.string);
        }
    } else {
        char cwd[512] = {0};
        if (platform_getcwd(cwd, sizeof(cwd))) {
            vdev_printf(vm_get_vdev(vm), "PATH=%s\n", cwd);
        }
    }
    return err;
}

BppError stmt_unsave_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    char filename[256] = {0};
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_BACKSLASH) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING && val.as.string) {
            snprintf(filename, sizeof(filename), "%s", str_data(val.as.string));
            str_release(vm_get_str(vm), val.as.string);
        }
    }

    if (filename[0] != '\0') {
        remove(filename);
    }
    return err;
}

BppError stmt_scratch_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex);
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int ch = (int)ch_val.as.number;
        FileContext *fc = vm_get_file(vm);
        if (file_is_open(fc, ch)) {
            file_seek(fc, ch, 1);
            file_flush(fc, ch);
        }
        return err;
    }


    if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_BACKSLASH) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING && val.as.string) {
            remove(str_data(val.as.string));
            str_release(vm_get_str(vm), val.as.string);
        }
    }
    return err;
}
