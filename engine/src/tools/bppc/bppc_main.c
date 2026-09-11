// FILENAME: bppc_main.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (bppc_internal.h)
// Implements toolchain and compiler subsystem components for bppc_main.
//
// ---- Includes ----

#include "tools/bppc_internal.h"
#include "platform/platform.h"
#include "hal/hal.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"

#ifdef _MSC_VER
#define runtime_strcasecmp runtime_strcasecmp
#endif

static void show_help(void) {
    platform_console_printf("BASIC++ Compiler (bppc) - v%s\n\n", BASIC_VERSION_STRING);
    platform_console_printf("Usage:\n");
    platform_console_printf("  bppc <input.bas> <output.bpp>                         (Compile to Bytecode)\n");
    platform_console_printf("  bppc --bytecode <input.bas> -o <output.bpp>           (Compile to Bytecode)\n");
    platform_console_printf("  bppc --c17 <input.bas> -o <output.c>                  (Transpile to C17 Source)\n");
    platform_console_printf("  bppc --native <input.bas> -o <output.exe>             (Compile to Native Executable via C17)\n");
    platform_console_printf("  bppc --standalone <input.bas> <output.exe>            (Compile Standalone Binary for Host OS)\n");
    platform_console_printf("  bppc --standalone --windows <input.bas> <output.exe>  (Compile Standalone Binary for Windows)\n");
    platform_console_printf("  bppc --standalone --linux <input.bas> <output>        (Compile Standalone Binary for Linux)\n");
    platform_console_printf("\n");
    platform_console_printf("Switches:\n");
    platform_console_printf("  --help, -h, -?   Display this command usage information.\n");
    platform_console_printf("  --about          Display information about the bppc compiler.\n");
    platform_console_printf("  --license        Display the software license details.\n");
    platform_console_printf("  --version        Display compiler version details.\n");
    platform_console_printf("  --debug, -g      Output verbose diagnostics during the compilation process.\n");
    platform_console_printf("  --optimize, -O   Enable optimization passes.\n");
    platform_console_printf("  --freedos, --dos Target FreeDOS / MS-DOS C code (with conio.h/dos.h).\n");
    platform_console_printf("  --c89, --c90     Target strict ANSI C89/C90 standard source code.\n");
    platform_console_printf("  --knr, --kr      Target classic K&R 1st-edition C syntax.\n");
    platform_console_printf("  --freestanding   Target freestanding C17 (zero libc / OS dependencies).\n");
    platform_console_printf("  --uefi           Target 64-bit UEFI firmware application (BOOTX64.EFI).\n");
    platform_console_printf("  --pascal         Transpile to Object Pascal for Free Pascal Compiler (FPC).\n");
    platform_console_printf("  --fast           Disable runtime array bounds checking.\n");
    platform_console_printf("  -o <file>        Specify target output path.\n");
}

int bppc_main_entry(int argc, char **argv) {
    if (argc < 2) {
        show_help();
        return 0;
    }

    bool is_standalone = false;
    bool is_c_transpile = false;
    bool is_native_compile = false;
    bool is_bytecode = false;
    bool target_windows = false;
    bool target_linux = false;
    bool opt_optimize = false;
    const char *infile = NULL;
    const char *outfile = NULL;

    int arg_idx = 1;
    while (arg_idx < argc) {
        if (runtime_strcmp(argv[arg_idx], "--help") == 0 || runtime_strcmp(argv[arg_idx], "-h") == 0 || runtime_strcmp(argv[arg_idx], "-?") == 0) {
            show_help();
            return 0;
        } else if (runtime_strcmp(argv[arg_idx], "--about") == 0) {
            platform_console_printf("BASIC++ Compiler (bppc)\n");
            platform_console_printf("Compiles BASIC++ source scripts (.bas) into portable bytecode (.bpp) payloads\n");
            platform_console_printf("and builds standalone cross-platform executable binaries for Windows and Linux.\n");
            return 0;
        } else if (runtime_strcmp(argv[arg_idx], "--license") == 0) {
            platform_console_printf("Modified MIT License\n\nCopyright (c) 2025-2026 BASIC++ Community\n\nTHIS SOFTWARE IS NOT TO BE SOLD.\n");
            return 0;
        } else if (runtime_strcmp(argv[arg_idx], "--version") == 0) {
            platform_console_printf("bppc version %s (synchronized with BASIC++ %s)\n", BASIC_VERSION_STRING, BASIC_VERSION_STRING);
            return 0;
        } else if (runtime_strcmp(argv[arg_idx], "--debug") == 0 || runtime_strcmp(argv[arg_idx], "-g") == 0) {
            g_bppc_debug_mode = true;
        } else if (runtime_strcmp(argv[arg_idx], "--optimize") == 0 || runtime_strcmp(argv[arg_idx], "-O") == 0) {
            opt_optimize = true;
        } else if (runtime_strcmp(argv[arg_idx], "--freedos") == 0 || runtime_strcmp(argv[arg_idx], "--dos") == 0) {
            is_c_transpile = true;
            g_target_dialect = TARGET_FREEDOS;
        } else if (runtime_strcmp(argv[arg_idx], "--c89") == 0 || runtime_strcmp(argv[arg_idx], "--c90") == 0 || runtime_strcmp(argv[arg_idx], "--ansi") == 0) {
            is_c_transpile = true;
            g_target_dialect = TARGET_C89;
        } else if (runtime_strcmp(argv[arg_idx], "--knr") == 0 || runtime_strcmp(argv[arg_idx], "--kr") == 0) {
            is_c_transpile = true;
            g_target_dialect = TARGET_KNR;
        } else if (runtime_strcmp(argv[arg_idx], "--freestanding") == 0) {
            is_c_transpile = true;
            g_target_dialect = TARGET_FREESTANDING;
        } else if (runtime_strcmp(argv[arg_idx], "--uefi") == 0) {
            is_c_transpile = true;
            g_target_dialect = TARGET_UEFI;
        } else if (runtime_strcmp(argv[arg_idx], "--pascal") == 0) {
            is_c_transpile = true;
            g_target_dialect = TARGET_PASCAL;
        } else if (runtime_strcmp(argv[arg_idx], "--fast") == 0 || runtime_strcmp(argv[arg_idx], "-fno-bounds-check") == 0) {
            g_no_bounds_check = true;
        } else if (runtime_strcmp(argv[arg_idx], "--c") == 0 || runtime_strcmp(argv[arg_idx], "--c17") == 0 || runtime_strcmp(argv[arg_idx], "--transpile") == 0) {
            is_c_transpile = true;
        } else if (runtime_strcmp(argv[arg_idx], "--bytecode") == 0 || runtime_strcmp(argv[arg_idx], "-b") == 0) {
            is_bytecode = true;
        } else if (runtime_strcmp(argv[arg_idx], "--native") == 0 || runtime_strcmp(argv[arg_idx], "--compile") == 0) {
            is_native_compile = true;
        } else if (runtime_strcmp(argv[arg_idx], "--standalone") == 0) {
            is_standalone = true;
        } else if (runtime_strcmp(argv[arg_idx], "--windows") == 0) {
            target_windows = true;
            is_standalone = true;
        } else if (runtime_strcmp(argv[arg_idx], "--linux") == 0) {
            target_linux = true;
            is_standalone = true;
        } else if (runtime_strcmp(argv[arg_idx], "-o") == 0) {
            if (arg_idx + 1 < argc) {
                outfile = argv[++arg_idx];
            }
        } else {
            if (!infile) {
                infile = argv[arg_idx];
            } else if (!outfile) {
                outfile = argv[arg_idx];
            }
        }
        arg_idx++;
    }

    if (!infile) {
        platform_console_eputs("Error: Input file parameter is required.\n");
        return 1;
    }

    char default_out[1024];
    if (!outfile) {
        if (is_c_transpile) {
            if (g_target_dialect == TARGET_PASCAL) {
                runtime_snprintf(default_out, sizeof(default_out), "%s.pas", infile);
            } else {
                runtime_snprintf(default_out, sizeof(default_out), "%s.c", infile);
            }
        } else if (is_native_compile || is_standalone) {
#ifdef _WIN32
            runtime_snprintf(default_out, sizeof(default_out), "%s.exe", infile);
#else
            runtime_snprintf(default_out, sizeof(default_out), "%s.out", infile);
#endif
        } else {
            runtime_snprintf(default_out, sizeof(default_out), "%s.bpp", infile);
        }
        outfile = default_out;
    }

    (void)is_bytecode;
    (void)opt_optimize;

#ifdef _WIN32
    if (target_linux) target_windows = true;
    else if (is_standalone && !target_windows) target_windows = true;
#else
    if (target_windows) target_linux = true;
    else if (is_standalone && !target_linux) target_linux = true;
#endif

    if (g_bppc_debug_mode) {
        platform_console_printf("[DEBUG] Target Windows Standalone: %s\n", target_windows ? "YES" : "NO");
        platform_console_printf("[DEBUG] Target Linux Standalone: %s\n", target_linux ? "YES" : "NO");
        platform_console_printf("[DEBUG] Opening input source file: '%s'\n", infile);
    }

    void *in_f = platform_file_open(infile, "rb");
    if (!in_f) {
        platform_console_eprintf("Error: Could not open input file '%s'\n", infile);
        return 1;
    }
    platform_file_seek(in_f, 0, IO_SEEK_END);
    long in_size = platform_file_tell(in_f);
    platform_file_seek(in_f, 0, IO_SEEK_SET);

    unsigned char *in_data = (unsigned char *)runtime_calloc(1, in_size >= 0 ? in_size + 1 : 1);
    if (!in_data) {
        platform_file_close(in_f);
        platform_console_eputs("Error: Out of memory loading input file\n");
        return 1;
    }
    if (in_size > 0) {
        if (platform_file_read(in_f, in_data, in_size) != (size_t)in_size) {
            platform_file_close(in_f);
            runtime_free(in_data);
            platform_console_eputs("Error: Failed to read input file\n");
            return 1;
        }
    }
    platform_file_close(in_f);

    char *source_text = NULL;
    bool is_detokenized = false;
    if (in_size > 0 && in_data[0] == 0xFF) {
        source_text = detokenize_gw_basic(in_data, in_size);
        runtime_free(in_data);
        in_data = NULL;
        if (!source_text) {
            platform_console_eputs("Error: Failed to detokenize GW-BASIC program\n");
            return 1;
        }
        is_detokenized = true;
    } else {
        in_data[in_size] = '\0';
        source_text = (char *)in_data;
    }

    if (is_c_transpile) {
        if (!transpile_basic_to_c_opts(source_text, outfile, true, opt_optimize, g_bppc_debug_mode)) {
            if (is_detokenized) runtime_free(source_text);
            return 1;
        }
        const char *tname = (g_target_dialect == TARGET_PASCAL) ? "Object Pascal" :
                            (g_target_dialect == TARGET_UEFI) ? "UEFI C" :
                            (g_target_dialect == TARGET_FREESTANDING) ? "Freestanding C" : "C17";
        platform_console_printf("Successfully transpiled '%s' to %s source '%s'\n", infile, tname, outfile);
    } else if (is_native_compile) {
        char temp_c[1024];
        runtime_snprintf(temp_c, sizeof(temp_c), "%s.tmp.c", outfile);
        if (!transpile_basic_to_c_opts(source_text, temp_c, true, opt_optimize, g_bppc_debug_mode)) {
            if (is_detokenized) runtime_free(source_text);
            platform_console_eputs("Error: Failed to transpile source code to intermediate C\n");
            return 1;
        }
        char cmd[2048];
#ifdef _WIN32
        runtime_snprintf(cmd, sizeof(cmd), "cl /O2 /nologo /Fe:\"%s\" \"%s\"", outfile, temp_c);
#else
        runtime_snprintf(cmd, sizeof(cmd), "gcc -O3 -std=c17 -o \"%s\" \"%s\" -lm", outfile, temp_c);
#endif
        platform_execute_command(cmd);
        platform_remove(temp_c);
        platform_console_printf("Successfully compiled native executable '%s'\n", outfile);
    } else if (is_standalone) {
        char temp_bpp[1024];
        runtime_snprintf(temp_bpp, sizeof(temp_bpp), "%s.tmp.bpp", outfile);
        
        if (!compile_to_bpp(source_text, temp_bpp)) {
            if (is_detokenized) runtime_free(source_text);
            platform_console_eputs("Error: Failed to compile source code to intermediate bytecode\n");
            return 1;
        }
        
        char win_path[1024];
        size_t out_len = runtime_strlen(outfile);
        if (out_len > 4 && runtime_strcasecmp(outfile + out_len - 4, ".exe") == 0) {
            runtime_strncpy(win_path, outfile, sizeof(win_path) - 1);
            win_path[sizeof(win_path) - 1] = '\0';
        } else {
            runtime_snprintf(win_path, sizeof(win_path), "%s.exe", outfile);
        }
        
        if (!package_standalone(temp_bpp, win_path, argv[0], "basstub.exe")) {
            platform_remove(temp_bpp);
            if (is_detokenized) runtime_free(source_text);
            return 1;
        }
        platform_console_printf("Successfully compiled standalone executable '%s'\n", win_path);
        platform_remove(temp_bpp);
    } else {
        if (!compile_to_bpp(source_text, outfile)) {
            if (is_detokenized) runtime_free(source_text);
            platform_console_eprintf("Error: Failed to compile source code to BPP bytecode '%s'\n", outfile);
            return 1;
        }
        platform_console_printf("Successfully compiled bytecode '%s'\n", outfile);
    }

    if (is_detokenized) runtime_free(source_text);
    return 0;
}
