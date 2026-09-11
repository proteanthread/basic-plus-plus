// FILENAME: trans.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (bppc_internal.h, trans_internal.h)
// Implements toolchain and compiler subsystem components for the BASIC++ Transpiler (trans).
//
// ---- Includes ----

#include "tools/trans_internal.h"
#include "tools/bppc_internal.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static void show_trans_help(void) {
    platform_console_printf("BASIC++ Transpiler (trans) - v%s\n\n", BASIC_VERSION_STRING);
    platform_console_printf("Usage:\n");
    platform_console_printf("  trans [options] <input1.bas> [input2.bas...] -o <output.c>\n\n");
    platform_console_printf("Options:\n");
    platform_console_printf("  --c17             Emit ISO C17 standard source code (default).\n");
    platform_console_printf("  --freedos, --dos  Emit FreeDOS / MS-DOS compatible C code (conio.h/dos.h).\n");
    platform_console_printf("  --c89, --c90      Emit strict ANSI C89/C90 standard source code.\n");
    platform_console_printf("  --knr, --kr       Emit classic K&R 1st-edition C syntax.\n");
    platform_console_printf("  --freestanding    Emit freestanding C17 source (zero libc / OS dependencies).\n");
    platform_console_printf("  --uefi            Emit 64-bit UEFI firmware application source (BOOTX64.EFI).\n");
    platform_console_printf("  --pascal          Transpile to Object Pascal for Free Pascal Compiler (FPC).\n");
    platform_console_printf("  --fast            Disable array bounds checks in generated code.\n");
    platform_console_printf("  --inline-runtime  Inline freestanding runtime helper routines into output file.\n");
    platform_console_printf("  --optimize, -O    Enable static constant folding and loop optimizations.\n");
    platform_console_printf("  --debug, -g       Emit original BASIC line numbers and source comments.\n");
    platform_console_printf("  -o <outfile>      Specify target output C/Pascal file.\n");
    platform_console_printf("  --help, -h        Display this help message.\n");
    platform_console_printf("  --version         Display transpiler version.\n");
}

int trans_main_entry(int argc, char **argv) {
    if (argc < 2) {
        show_trans_help();
        return 0;
    }

    bool opt_c17 = true;
    bool opt_inline_runtime = true;
    bool opt_optimize = false;
    bool opt_debug = false;
    const char *outfile = NULL;

    const char *infiles[128];
    int num_infiles = 0;

    for (int i = 1; i < argc; i++) {
        if (runtime_strcmp(argv[i], "--help") == 0 || runtime_strcmp(argv[i], "-h") == 0 || runtime_strcmp(argv[i], "-?") == 0) {
            show_trans_help();
            return 0;
        } else if (runtime_strcmp(argv[i], "--version") == 0) {
            platform_console_printf("trans version %s (BASIC++ %s)\n", BASIC_VERSION_STRING, BASIC_VERSION_STRING);
            return 0;
        } else if (runtime_strcmp(argv[i], "--c17") == 0) {
            opt_c17 = true;
            g_target_dialect = TARGET_C17;
        } else if (runtime_strcmp(argv[i], "--freedos") == 0 || runtime_strcmp(argv[i], "--dos") == 0) {
            g_target_dialect = TARGET_FREEDOS;
        } else if (runtime_strcmp(argv[i], "--c89") == 0 || runtime_strcmp(argv[i], "--c90") == 0 || runtime_strcmp(argv[i], "--ansi") == 0) {
            g_target_dialect = TARGET_C89;
        } else if (runtime_strcmp(argv[i], "--knr") == 0 || runtime_strcmp(argv[i], "--kr") == 0) {
            g_target_dialect = TARGET_KNR;
        } else if (runtime_strcmp(argv[i], "--freestanding") == 0) {
            g_target_dialect = TARGET_FREESTANDING;
        } else if (runtime_strcmp(argv[i], "--uefi") == 0) {
            g_target_dialect = TARGET_UEFI;
        } else if (runtime_strcmp(argv[i], "--pascal") == 0) {
            g_target_dialect = TARGET_PASCAL;
        } else if (runtime_strcmp(argv[i], "--fast") == 0 || runtime_strcmp(argv[i], "-fno-bounds-check") == 0) {
            g_no_bounds_check = true;
        } else if (runtime_strcmp(argv[i], "--inline-runtime") == 0) {
            opt_inline_runtime = true;
        } else if (runtime_strcmp(argv[i], "--optimize") == 0 || runtime_strcmp(argv[i], "-O") == 0) {
            opt_optimize = true;
        } else if (runtime_strcmp(argv[i], "--debug") == 0 || runtime_strcmp(argv[i], "-g") == 0) {
            opt_debug = true;
        } else if (runtime_strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                outfile = argv[++i];
            } else {
                platform_console_eputs("Error: -o option requires an output filename\n");
                return 1;
            }
        } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
            platform_console_eprintf("Warning: Unrecognized option '%s'\n", argv[i]);
        } else {
            if (num_infiles < 128) {
                infiles[num_infiles++] = argv[i];
            }
        }
    }

    if (num_infiles == 0) {
        platform_console_eputs("Error: No input files specified.\n");
        return 1;
    }

    char default_out[512];
    if (!outfile) {
        if (g_target_dialect == TARGET_PASCAL) {
            runtime_snprintf(default_out, sizeof(default_out), "%s.pas", infiles[0]);
        } else {
            runtime_snprintf(default_out, sizeof(default_out), "%s.c", infiles[0]);
        }
        outfile = default_out;
    }

    (void)opt_c17;
    if (!transpile_files_to_c(infiles, num_infiles, outfile, opt_inline_runtime, opt_optimize, opt_debug)) {
        platform_console_eprintf("Error: Failed to transpile source files to '%s'\n", outfile);
        return 1;
    }

    const char *tname = (g_target_dialect == TARGET_PASCAL) ? "Object Pascal" :
                        (g_target_dialect == TARGET_UEFI) ? "UEFI C" :
                        (g_target_dialect == TARGET_FREESTANDING) ? "Freestanding C" : "C17";
    platform_console_printf("Successfully transpiled %d file(s) to %s source '%s'\n", num_infiles, tname, outfile);
    return 0;
}

int main(int argc, char **argv) {
    return trans_main_entry(argc, argv);
}
