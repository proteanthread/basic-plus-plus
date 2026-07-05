/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: main.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BValue tagged union structure fields, core memory allocator logic, security capability ratings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - main.c
 // ---
 //
 // Entry point, REPL (Read-Eval-Print Loop), and bootstrap.
 //
 // PURPOSE:
 // This module provides:
 // 1. The main() entry point.
 // 2. Memory subsystem initialization and shutdown.
 // 3. The interactive REPL loop (classic BASIC prompt style).
 // 4. Input classification (line storage vs. immediate execution).
 //
 // REPL BEHAVIOR (Palo Alto Tiny BASIC compatible):
 // - On startup, prints a banner with version and dialect info.
 // - Prints "READY" and a prompt ("> ").
 // - Reads a line of input.
 // - If the line starts with a digit -> it's a program line:
 // - If only a line number -> delete that line.
 // - Otherwise -> store the line (insert or replace).
 // - If the line starts with a letter or keyword -> immediate mode:
 // - Tokenize and execute immediately.
 // - Results are printed; errors return to prompt.
 // - After any error, prints "READY" and returns to prompt.
 // - On EOF (Ctrl-Z on Windows, Ctrl-D on Unix) -> exit.
 //
 // DESIGN RATIONALE:
 // The REPL is kept minimal. It classifies input and delegates
 // to the appropriate subsystem. No parsing logic lives here
 // (that's in parser.c). No execution logic lives here (that's
 // in exec.c). Main.c is glue code.
 //
 // ---

#ifndef _WIN32
  #if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 200112L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200112L
  #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "memory.h"
#include "lexer.h"
#include "parser.h"
#include "exec.h"
#include "runtime.h"
#include "dialect.h"
#include "errors.h"
#include "vdev.h"
#include "funcreg.h"
#include "builtins.h"
#include "fileio.h"
#include "io/vdev_net.h"
#include "vm.h"
#include "module.h"
#include "error_registry.h"
#include "security.h"
#include "platform.h"
#include "gfxbuf.h"
#ifndef BPP_LITE_BUILD
#include "pcode.h"
#endif
#include "scope.h"
#include "segmented_mem.h"
#include "keyword_props.h"
#include "override.h"
#include "config_file.h"
#include "boot.h"
#include "mod_legacy_compat.h"
#include "console.h"
#include "memmap.h"
#ifndef NO_SDL2
#include "sdl2_emu.h"
#endif
#include "memmap.h"

FILE *g_out_fp = NULL;
int cli_trace = 0;
int g_cli_lite = 0;

// Globals used by archive.c and parser_cmds.c
char g_argv_0[512] = "";
char g_runner_path[512] = "";
long g_embedded_offset = 0;
#if 0
static struct RuntimeState *g_main_runtime_ptr = NULL;
#endif
// --- Input Classification ---

 // parse_line_number - Extract a line number from the start of input.
 //
 // If the input starts with digits, parses the line number and
 // returns it. Sets *end_pos to the position after the line number
 // and any trailing whitespace.
 //
 // Returns the line number (>= 1), or 0 if the input does not
 // start with a digit.
static int parse_line_number(const char *input, int *end_pos)
{
 int pos = 0;
 int num = 0;

 // Skip leading whitespace
 while (input[pos] == ' ' || input[pos] == '\t') {
 pos++;
 }

 // Check for digit
 if (!isdigit((unsigned char)input[pos])) {
 *end_pos = pos;
 return 0;
 }

 // Parse the number
 while (isdigit((unsigned char)input[pos])) {
 num = num * 10 + (input[pos] - '0');
 pos++;
 }

 *end_pos = pos;
 return num;
}

#ifdef __GNUC__
__attribute__((unused))
#endif
static int is_blank_after(const char *input, int pos)
{
 while (input[pos] != '\0') {
 if (input[pos] != ' ' && input[pos] != '\t') {
 return 0;
 }
 pos++;
 }
 return 1;
}

#ifdef __GNUC__
__attribute__((unused))
#endif
static void strip_newline(char *str)
{
 int len = (int)strlen(str);
 while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
 str[--len] = '\0';
 }
}

#ifdef __GNUC__
__attribute__((unused))
#endif
static void print_usage(const char *prog)
{
    printf("Usage: %s [options] [program.bas]\n\n", prog);
    printf("Execution Modes:\n");
    printf("  <filename>       Load, run, and remain in basic++\n");
    printf("  --run <file>     Run specified program then exit\n");
    printf("  --load <file>    Load program and remain in basic++\n");
    printf("  --list <file>    Load, list program, then exit\n");
    printf("  --dry-run <file> Load in debug/step-by-step mode (Disables File I/O)\n");
    printf("  --edit           Start in screen-editor mode\n");
    printf("  -c \"CMD\"         Execute a command and exit\n");
    printf("\nInfo:\n");
    printf("  -h, --help       Print this help and exit\n");
    printf("  -v, --version    Print version and exit\n");
    printf("  --about          Describe version and what's new\n");
    printf("  --license        Display full MIT license text\n");
    printf("\nEnvironment & Memory:\n");
    printf("  -d DIALECT       Set default dialect (GWBS, QBAS, PATB, BPP, etc.)\n");
    printf("  -s LEVEL         Set security level (OPEN, STANDARD, RESTRICTED)\n");
    printf("  -S               Enable strict dialect mode\n");
    printf("  -f FILE          Use a specific configuration file\n");
    printf("  -q               Quiet mode (suppress startup banner)\n");
    printf("  --debug          Enable verbose error mode and diagnostics\n");
    printf("  --log [file]     Redirect output log to file\n");
    printf("  --out [file]     Redirect stdout to file\n");
    printf("  --com <n>        RS-232 receive buffer size (bytes)\n");
    printf("  --files <n>      Max open files (Default 3)\n");
    printf("  --mem <m,n>      Set memory limits (high mem, workspace)\n");
    printf("  --records <n>    Max record length for RANDOM files (Default 128)\n");
    printf("  --block          Preallocate blocks/buffers\n");
    printf("  --dbl            Enable double-precision math\n");
    printf("  --dynamic        Allow dynamic arrays and >64KB structures\n");
    printf("\nVideo & Display:\n");
    printf("  --no-color       Monochrome display mode\n");
    printf("  --cga            Enable faster CGA screen updates\n");
    printf("  --hi-res         Use maximum display lines\n");
    printf("  --no-high        Disable high-intensity colors\n");
    printf("\nCompatibility & Interop:\n");
    printf("  --mbf            Use Microsoft Binary Format (MBF)\n");
    printf("  --kbd            Full keyboard re-mapping\n");
    printf("  --break <on|off> Enable/disable Ctrl+Break handling\n");
    printf("  --cmd <string>   Pass string to COMMAND$ (must be last)\n");
    printf("  --lib <file>     Load external library (.dll / .so)\n");
    printf("  --func <name>    Load external function\n");
    printf("  --mod <name>     Load external module\n");
    printf("  --ext            Enable extended features\n");
    printf("  <file, >file     Manual file redirection\n");
    printf("\nConfig file: %s (searched in current dir, then home)\n",
           config_file_get_name(prog));
    printf("Priority: config file < CLI switches < runtime commands\n");
}


#ifdef _WIN32
#define TokenType WinTokenType
#include <windows.h>
#undef TokenType
#if 0
static LONG WINAPI crash_handler(EXCEPTION_POINTERS *ExceptionInfo) {
    if (g_main_runtime_ptr && g_main_runtime_ptr->log_fp) {
        fprintf((FILE*)g_main_runtime_ptr->log_fp, "[FATAL] Process crashed! Exception code: 0x%08X at address %p\n", 
                ExceptionInfo->ExceptionRecord->ExceptionCode,
                ExceptionInfo->ExceptionRecord->ExceptionAddress);
        fflush((FILE*)g_main_runtime_ptr->log_fp);
    }
    fprintf(stderr, "\n[FATAL] Process crashed! Exception code: 0x%08X at address %p\n", 
            ExceptionInfo->ExceptionRecord->ExceptionCode,
            ExceptionInfo->ExceptionRecord->ExceptionAddress);
    fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif
#else
#include <signal.h>
#if 0
static void crash_handler(int sig) {
    if (g_main_runtime_ptr && g_main_runtime_ptr->log_fp) {
        fprintf((FILE*)g_main_runtime_ptr->log_fp, "[FATAL] Process crashed! Received signal: %d\n", sig);
        fflush((FILE*)g_main_runtime_ptr->log_fp);
    }
    fprintf(stderr, "\n[FATAL] Process crashed! Received signal: %d\n", sig);
    fflush(stderr);
    exit(1);
}
#endif
#endif

#include <time.h>

#ifdef BPP_LITE_BUILD
#include "legacy_compat.h"
int g_gw_machine_type = 0;
int g_cga_snow = 0;
char g_mda_color[16] = "";
GW_State *g_state = NULL;
#endif

static int str_eq_nocase_ext(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        char c1 = *s1;
        char c2 = *s2;
        if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
        if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
        if (c1 != c2) return 0;
        s1++;
        s2++;
    }
    return *s1 == *s2;
}

static void get_default_log_filename(const char *prog_path, const char *script_path, const char *ext, time_t t, char *out_buf, int buf_size)
{
    char base_name[128] = "basicpp";
    const char *source = prog_path;
    
    if (script_path != NULL) {
        source = script_path;
    }
    
    const char *last_sep = strrchr(source, '/');
    const char *last_sep2 = strrchr(source, '\\');
    if (last_sep2 > last_sep) last_sep = last_sep2;
    
    const char *name_start = (last_sep != NULL) ? (last_sep + 1) : source;
    
    int len = (int)strlen(name_start);
    if (len > 120) len = 120;
    memcpy(base_name, name_start, (size_t)len);
    base_name[len] = '\0';
    
    char *dot = strrchr(base_name, '.');
    if (dot != NULL) {
        if (str_eq_nocase_ext(dot, ".exe") || str_eq_nocase_ext(dot, ".bas") || 
            str_eq_nocase_ext(dot, ".bpp") || str_eq_nocase_ext(dot, ".bpl")) {
            *dot = '\0';
        }
    }
    
    struct tm *tm = localtime(&t);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d-%H-%M-%S", tm);
    
    snprintf(out_buf, (size_t)buf_size, "%s-%s%s", base_name, time_str, ext);
}

// Platform clean-up implementation moved to platform.c

// --- Main Entry Point ---

#ifndef BPP_TRANS_BUILD
int main(int argc, char *argv[])
{
 MemorySystem memory;
 static RuntimeState runtime;
 char input_buf[INPUT_BUFFER_SIZE];

    // --- CLI argument storage ---
    const char *cli_dialect = NULL;
    const char *cli_bios = NULL;
    const char *cli_security = NULL;
    int cli_strict = -1; // -1 = unset
    int cli_quiet = 0;
    const char *cli_run_file = NULL;
    const char *cli_load_file = NULL;
    const char *cli_list_file = NULL;
    const char *cli_dry_run_file = NULL;
    int cli_edit = 0;
    int cli_debug = 0;
    int cli_boot_log = 0;
    int cli_verbose = 0;
    int cli_log = 0;
    const char *cli_log_file = NULL;
    int cli_out = 0;
    const char *cli_out_file = NULL;
    const char *cli_command = NULL;
    const char *cli_config_file = NULL;
    const char *cli_program = NULL;
    const char *cli_cmd_str = NULL;
    const char *cli_lib = NULL;
    const char *cli_mod = NULL;
    const char *cli_func = NULL;
    int cli_direct = 1;
    
    // Redirection storage
    const char *cli_redirect_in = NULL;
    const char *cli_redirect_out = NULL;
    const char *cli_redirect_append = NULL;
    (void)cli_config_file;
    (void)cli_cmd_str;

    // --- Config file ---
    ConfigFile cfg = {0};

    // --- Effective settings ---
    DialectId eff_dialect = BASICPP_DEFAULT_DIALECT;
    SecLevel eff_security = SEC_OPEN;
    int eff_strict = 0;
    int eff_quiet = 0;

    int i;

    // Save executable name for embedded binaries
    if (argc > 0 && argv[0]) {
        strncpy(g_argv_0, argv[0], sizeof(g_argv_0) - 1);
        g_argv_0[sizeof(g_argv_0) - 1] = '\0';
    }

    // ----- Parse command-line arguments -----
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
#ifdef BPP_LITE_BUILD
            if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "-s") == 0 ||
                strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "-f") == 0 ||
                strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--command") == 0 ||
                strcmp(argv[i], "--edit") == 0 || strcmp(argv[i], "--lib") == 0 ||
                strcmp(argv[i], "--bios") == 0 ||
                strcmp(argv[i], "--mod") == 0 || strcmp(argv[i], "--func") == 0 ||
                strcmp(argv[i], "--com") == 0 || strcmp(argv[i], "--files") == 0 ||
                strcmp(argv[i], "--mem") == 0 || strcmp(argv[i], "--records") == 0 ||
                strcmp(argv[i], "--block") == 0 || strcmp(argv[i], "--dbl") == 0 ||
                strcmp(argv[i], "--dynamic") == 0 || strcmp(argv[i], "--no-color") == 0 ||
                strcmp(argv[i], "--cga") == 0 || strcmp(argv[i], "--hi-res") == 0 ||
                strcmp(argv[i], "--no-high") == 0 || strcmp(argv[i], "--mbf") == 0 ||
                strcmp(argv[i], "--kbd") == 0 || strcmp(argv[i], "--break") == 0 ||
                strcmp(argv[i], "--cmd") == 0 || strcmp(argv[i], "--ext") == 0 ||
                strcmp(argv[i], "--herc") == 0 || strcmp(argv[i], "--hercules") == 0 ||
                strcmp(argv[i], "--tandy") == 0 || strcmp(argv[i], "--pcjr") == 0 ||
                strcmp(argv[i], "--plantronics") == 0 || strcmp(argv[i], "--att") == 0 ||
                strcmp(argv[i], "--amstrad") == 0 || strcmp(argv[i], "--pc98") == 0 ||
                strcmp(argv[i], "--video") == 0 || strcmp(argv[i], "--mda") == 0 ||
                strcmp(argv[i], "--cleanup") == 0 || strcmp(argv[i], "--clean-up") == 0 ||
                strcmp(argv[i], "--full-cleanup") == 0 || strcmp(argv[i], "--full-clean-up") == 0) {
                printf("Error: Option '%s' is not supported in BASIC++ Lite\n", argv[i]);
                return 1;
            }
#endif
            if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
                printf("%s %s\n", BASICPP_NAME, BASICPP_VERSION);
                return 0;
            }
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                print_usage(argv[0]);
                return 0;
            }
            if (strcmp(argv[i], "--about") == 0) {
                printf("%s %s\n", BASICPP_NAME, BASICPP_VERSION);
                printf("A hybrid AST/P-Code interpreter for classic BASIC dialects.\n");
                printf("Milestone 5 includes Config and CLI Supercharging.\n");
                return 0;
            }
            if (strcmp(argv[i], "--license") == 0) {
                printf("MIT License\n(See LICENSE file for full text)\n");
                return 0;
            }
            if (strcmp(argv[i], "-q") == 0) {
                cli_quiet = 1;
                continue;
            }
            if (strcmp(argv[i], "--debug") == 0) {
                cli_debug = 1;
                continue;
            }
            if (strcmp(argv[i], "--boot-log") == 0) {
                cli_boot_log = 1;
                continue;
            }
            if (strcmp(argv[i], "--verbose") == 0) {
                cli_verbose = 1;
                continue;
            }
            if (strcmp(argv[i], "--edit") == 0) {
                cli_edit = 1;
                continue;
            }
            if (strcmp(argv[i], "-S") == 0) {
                cli_strict = 1;
                continue;
            }
            if (strcmp(argv[i], "--run") == 0 || strcmp(argv[i], "-r") == 0) {
                if (i + 1 < argc) cli_run_file = argv[++i];
                else { printf("Error: %s requires a filename\n", argv[i-1]); return 1; }
                continue;
            }
            if (strcmp(argv[i], "--load") == 0) {
                if (i + 1 < argc) cli_load_file = argv[++i];
                else { printf("Error: --load requires a filename\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "--list") == 0) {
                if (i + 1 < argc) cli_list_file = argv[++i];
                else { printf("Error: --list requires a filename\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "--dry-run") == 0) {
                if (i + 1 < argc) cli_dry_run_file = argv[++i];
                else { printf("Error: --dry-run requires a filename\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "-d") == 0) {
                if (i + 1 < argc) cli_dialect = argv[++i];
                else { printf("Error: -d requires a dialect name\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "--bios") == 0) {
                if (i + 1 < argc) cli_bios = argv[++i];
                else { printf("Error: --bios requires a bios model name\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "--herc") == 0 || strcmp(argv[i], "--hercules") == 0) {
                g_gw_machine_type = 1;
                continue;
            }
            if (strcmp(argv[i], "--tandy") == 0) {
                g_gw_machine_type = 2;
                continue;
            }
            if (strcmp(argv[i], "--pcjr") == 0) {
                g_gw_machine_type = 3;
                continue;
            }
            if (strcmp(argv[i], "--plantronics") == 0) {
                g_gw_machine_type = 4;
                continue;
            }
            if (strcmp(argv[i], "--att") == 0) {
                g_gw_machine_type = 5;
                continue;
            }
            if (strcmp(argv[i], "--amstrad") == 0) {
                g_gw_machine_type = 6;
                continue;
            }
            if (strcmp(argv[i], "--pc98") == 0) {
                g_gw_machine_type = 7;
                continue;
            }
            if (strcmp(argv[i], "--video") == 0) {
                if (i + 1 < argc) {
                    const char *video_type = argv[++i];
                    if (strcmp(video_type, "MDA") == 0 || strcmp(video_type, "mda") == 0) {
                        g_gw_machine_type = 8; // MACHINE_MDA
                    } else if (strcmp(video_type, "CGA") == 0 || strcmp(video_type, "cga") == 0) {
                        g_gw_machine_type = 9; // MACHINE_CGA
                    } else if (strcmp(video_type, "EGA") == 0 || strcmp(video_type, "ega") == 0) {
                        g_gw_machine_type = 10; // MACHINE_EGA
                    } else if (strcmp(video_type, "VGA") == 0 || strcmp(video_type, "vga") == 0) {
                        g_gw_machine_type = 0; // MACHINE_VGA
                    } else {
                        printf("Error: unknown video card '%s'\n", video_type);
                        return 1;
                    }
                } else {
                    printf("Error: --video requires a card type (MDA, CGA, EGA, VGA)\n");
                    return 1;
                }
                continue;
            }
            if (strcmp(argv[i], "--mda") == 0) {
                g_gw_machine_type = 8; // Switch to MDA mode
                if (i + 1 < argc && argv[i+1][0] != '-') {
                    const char *color_arg = argv[i+1];
                    if (strcmp(color_arg, "grey") == 0 || strcmp(color_arg, "gray") == 0 ||
                        strcmp(color_arg, "white") == 0 || strcmp(color_arg, "green") == 0 ||
                        strcmp(color_arg, "amber") == 0 ||
                        strcmp(color_arg, "GREY") == 0 || strcmp(color_arg, "GRAY") == 0 ||
                        strcmp(color_arg, "WHITE") == 0 || strcmp(color_arg, "GREEN") == 0 ||
                        strcmp(color_arg, "AMBER") == 0) {
                        strncpy(g_mda_color, color_arg, sizeof(g_mda_color) - 1);
                        g_mda_color[sizeof(g_mda_color) - 1] = '\0';
                        i++; // consume color argument
                    }
                }
                continue;
            }
            if (strcmp(argv[i], "--cga") == 0) {
                g_cga_snow = 1; // Enable CGA updates / snow simulation
                continue;
            }
            if (strcmp(argv[i], "-s") == 0) {
                if (i + 1 < argc) cli_security = argv[++i];
                else { printf("Error: -s requires a security level\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "-c") == 0) {
                if (i + 1 < argc) cli_command = argv[++i];
                else { printf("Error: -c requires a command string\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "-f") == 0) {
                if (i + 1 < argc) cli_config_file = argv[++i];
                else { printf("Error: -f requires a filename\n"); return 1; }
                continue;
            }
            if (strcmp(argv[i], "--cmd") == 0) {
                if (i + 1 < argc) {
                    // Join remaining arguments for COMMAND$
                    cli_cmd_str = argv[++i];
                    break;
                }
            }
            if (strcmp(argv[i], "--log") == 0) {
                cli_log = 1;
                if (i + 1 < argc && argv[i+1][0] != '-') {
                    cli_log_file = argv[++i];
                }
                continue;
            }
            if (strcmp(argv[i], "--out") == 0) {
                cli_out = 1;
                if (i + 1 < argc && argv[i+1][0] != '-') {
                    cli_out_file = argv[++i];
                }
                continue;
            }
            if (strcmp(argv[i], "--trace") == 0) {
                cli_trace = 1;
                continue;
            }
            if (strcmp(argv[i], "--direct") == 0) {
                cli_direct = 1;
                continue;
            }
            if (strcmp(argv[i], "--vm") == 0) {
                cli_direct = 0;
                continue;
            }
            if (strcmp(argv[i], "--lite") == 0) {
                g_cli_lite = 1;
                continue;
            }
            if (strcmp(argv[i], "--clean-up") == 0 || strcmp(argv[i], "--cleanup") == 0) {
                platform_cleanup_logs(0);
                return 0;
            }
            if (strcmp(argv[i], "--full-clean-up") == 0 || strcmp(argv[i], "--full-cleanup") == 0) {
                platform_cleanup_logs(1);
                return 0;
            }
            // Extract --lib, --mod, --func for module loading
            if (strcmp(argv[i], "--lib") == 0) {
                if (i + 1 < argc) cli_lib = argv[++i];
                continue;
            }
            if (strcmp(argv[i], "--mod") == 0) {
                if (i + 1 < argc) cli_mod = argv[++i];
                continue;
            }
            if (strcmp(argv[i], "--func") == 0) {
                if (i + 1 < argc) cli_func = argv[++i];
                continue;
            }
            // Silently accept/ignore environment & hardware switches for now
            if (strcmp(argv[i], "--com") == 0 || strcmp(argv[i], "--files") == 0 ||
                strcmp(argv[i], "--mem") == 0 || strcmp(argv[i], "--records") == 0 ||
                strcmp(argv[i], "--break") == 0) {
                if (i + 1 < argc) i++;
                continue;
            }
            if (strcmp(argv[i], "--block") == 0 || strcmp(argv[i], "--dbl") == 0 ||
                strcmp(argv[i], "--no-color") == 0 || strcmp(argv[i], "--cga") == 0 ||
                strcmp(argv[i], "--hi-res") == 0 || strcmp(argv[i], "--no-high") == 0 ||
                strcmp(argv[i], "--mbf") == 0 || strcmp(argv[i], "--dynamic") == 0 ||
                strcmp(argv[i], "--ext") == 0 || strcmp(argv[i], "--kbd") == 0) {
                continue;
            }
            
            printf("Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else if (argv[i][0] == '<') {
            cli_redirect_in = argv[i] + 1;
            if (*cli_redirect_in == '\0' && i + 1 < argc) cli_redirect_in = argv[++i];
        } else if (argv[i][0] == '>') {
            if (argv[i][1] == '>') {
                cli_redirect_append = argv[i] + 2;
                if (*cli_redirect_append == '\0' && i + 1 < argc) cli_redirect_append = argv[++i];
            } else {
                cli_redirect_out = argv[i] + 1;
                if (*cli_redirect_out == '\0' && i + 1 < argc) cli_redirect_out = argv[++i];
            }
        } else {
            // Positional argument: treat as program file
            cli_program = argv[i];
        }
    }

    // Process manual file redirection
    if (cli_redirect_in != NULL) freopen(cli_redirect_in, "r", stdin);
    if (cli_redirect_out != NULL) freopen(cli_redirect_out, "w", stdout);
    if (cli_redirect_append != NULL) freopen(cli_redirect_append, "a", stdout);

 // ----- Early init (needed before config lookups) -----
 platform_init();
 dialect_register_all();
 keyword_registry_init();

#ifndef BPP_LITE_BUILD
 // ----- Load config file (lowest priority) -----
 if (cli_config_file != NULL) {
  if (config_file_load_path(&cfg, cli_config_file) != 0) {
   printf("Error: cannot open config file '%s'\n",
    cli_config_file);
   return 1;
  }
 } else {
  config_file_load(&cfg, argv[0]);
 }
#endif

 // ----- Apply settings: config file first, then CLI overrides -----

// Dialect
 if (cfg.found && cfg.dialect[0] != '\0') {
  int did = dialect_find_by_name(cfg.dialect);
  if (did >= 0) eff_dialect = (DialectId)did;
 }
 if (cli_dialect != NULL) {
  int did = dialect_find_by_name(cli_dialect);
  if (did >= 0) {
   eff_dialect = (DialectId)did;
  } else {
   printf("Warning: unknown dialect '%s', using default\n",
    cli_dialect);
  }
 }
 if (g_cli_lite) {
     eff_dialect = DIALECT_GW_BASIC;
 }

  // Default security depends on context
  eff_security = SEC_OPEN;
  if (cli_run_file != NULL || cli_program != NULL) {
   // eff_security = SEC_STANDARD; // REMOVED: default to OPEN per user request
  }
 
  // Piped execution defaults to RESTRICTED
  if (!platform_stdin_is_tty()) {
   // eff_security = SEC_RESTRICTED; // REMOVED: default to OPEN per user request
  }

  // Dialect security recommendation (informational only).
  // Dialects recommend a security level but do NOT override
  // the user's effective setting. SEC_OPEN is the default.
  // Users control security via -s flag, config file, or
  // SECURITY command at runtime.
 
 // Security
 if (cfg.found && cfg.security[0] != '\0') {
  if (strcmp(cfg.security, "OPEN") == 0)
   eff_security = SEC_OPEN;
  else if (strcmp(cfg.security, "STANDARD") == 0)
   eff_security = SEC_STANDARD;
  else if (strcmp(cfg.security, "RESTRICTED") == 0)
   eff_security = SEC_RESTRICTED;
 }
 if (cli_security != NULL) {
  char sec_upper[16];
  int j;
  int len = (int)strlen(cli_security);
  if (len > 15) len = 15;
  for (j = 0; j < len; j++) {
   char c = cli_security[j];
   if (c >= 'a' && c <= 'z') c = (char)(c - 32);
   sec_upper[j] = c;
  }
  sec_upper[len] = '\0';
  if (strcmp(sec_upper, "OPEN") == 0)
   eff_security = SEC_OPEN;
  else if (strcmp(sec_upper, "STANDARD") == 0)
   eff_security = SEC_STANDARD;
  else if (strcmp(sec_upper, "RESTRICTED") == 0)
   eff_security = SEC_RESTRICTED;
  else
   printf("Warning: unknown security level '%s'\n",
    cli_security);
 }

 // Strict
 if (cfg.found && cfg.strict >= 0)
  eff_strict = cfg.strict;
 if (cli_strict >= 0)
  eff_strict = cli_strict;

 // Quiet
 if (cfg.found && cfg.quiet >= 0)
  eff_quiet = cfg.quiet;
 if (cli_quiet)
  eff_quiet = 1;

 // Run file from positional arg
 if (cli_program != NULL && cli_run_file == NULL)
  cli_run_file = cli_program;

 {
      time_t shared_time = time(NULL);
      if (cli_trace) {
          runtime.log_fp = stderr;
          runtime.log_level = BOOT_VERBOSE;
          runtime.log_to_stderr = 1;
          boot_init_runtime(&runtime);
      } else if (cli_log) {
          char final_log_path[260];
          if (cli_log_file != NULL) {
              strncpy(final_log_path, cli_log_file, sizeof(final_log_path) - 1);
              final_log_path[sizeof(final_log_path) - 1] = '\0';
          } else {
              const char *script_file = cli_run_file;
              if (script_file == NULL) script_file = cli_load_file;
              if (script_file == NULL) script_file = cli_list_file;
              if (script_file == NULL) script_file = cli_dry_run_file;
              get_default_log_filename(argv[0], script_file, ".LOG", shared_time, final_log_path, sizeof(final_log_path));
          }
          runtime.log_fp = fopen(final_log_path, "w");
          runtime.log_level = BOOT_VERBOSE;
          boot_init_runtime(&runtime);
          vdev_set_mock_time_active(1);
          if (runtime.log_fp == NULL) {
              printf("Warning: failed to open log file '%s'\n", final_log_path);
          }
      }
      if (cli_out) {
          char final_out_path[260];
          if (cli_out_file != NULL) {
              strncpy(final_out_path, cli_out_file, sizeof(final_out_path) - 1);
              final_out_path[sizeof(final_out_path) - 1] = '\0';
          } else {
              const char *script_file = cli_run_file;
              if (script_file == NULL) script_file = cli_load_file;
              if (script_file == NULL) script_file = cli_list_file;
              if (script_file == NULL) script_file = cli_dry_run_file;
              get_default_log_filename(argv[0], script_file, ".OUT", shared_time, final_out_path, sizeof(final_out_path));
          }
          g_out_fp = fopen(final_out_path, "w");
          if (g_out_fp == NULL) {
              printf("Warning: failed to open out file '%s'\n", final_out_path);
          }
      }
  }

#ifndef INPUT_CONSOLE
#ifndef NO_SDL2
  // Initialize the SDL console window at boot for GUI builds
  gw_sdl2_init(640, 400, "BASIC++ Interpreter", 0);
#endif
#endif

  // ----- Print startup banner (unless quiet) -----
 if (!eff_quiet) {
  printf("\n");
  printf("%s %s\n", BASICPP_NAME, BASICPP_VERSION);
  printf("%s\n", BASICPP_COPYRIGHT);
  printf("%s\n", __DATE__);
  if (cfg.found) {
   printf("Config: %s\n", cfg.filepath);
  }
  printf("\n");
 }

 // ----- Build BootConfig from resolved settings -----
 {
  BootConfig boot_cfg;
  BootStatus boot_result;

  memset(&boot_cfg, 0, sizeof(boot_cfg));
  boot_cfg.dialect   = eff_dialect;
  boot_cfg.security  = eff_security;
  boot_cfg.strict    = eff_strict;
  boot_cfg.quiet     = eff_quiet;
  boot_cfg.run_file  = cli_run_file;
  boot_cfg.command   = cli_command;
  boot_cfg.cli_lib   = cli_lib;
  boot_cfg.cli_mod   = cli_mod;
  boot_cfg.cli_func  = cli_func;

  // Map diagnostic flags to boot verbosity
  boot_cfg.verbosity = BOOT_SILENT;
  if (cli_boot_log) boot_cfg.verbosity = BOOT_LOG;
  if (cli_debug)    boot_cfg.verbosity = BOOT_DEBUG;
  if (cli_verbose)  boot_cfg.verbosity = BOOT_VERBOSE;

  // ----- Execute formal boot sequence -----
  boot_result = boot_execute(&boot_cfg, &memory, &runtime);

  if (boot_result == BOOT_CRITICAL) {
   return 1;
  }

  runtime.direct_mode = cli_direct;
  runtime.lite_mode = g_cli_lite;

  {
      MemMapType mtype = MMAP_MSDOS; // Default to MS-DOS / IBM PC BIOS map if unspecified
      if (cli_bios != NULL) {
          mtype = memmap_from_string(cli_bios, (int)strlen(cli_bios));
          if (mtype == MMAP_COUNT || (mtype != MMAP_NONE && mtype != MMAP_MSDOS && 
              mtype != MMAP_IBM_PC && mtype != MMAP_IBM_PCJR && 
              mtype != MMAP_IBM_XT && mtype != MMAP_IBM_AT)) {
              printf("Error: invalid BIOS map '%s'. Must be NONE, MSDOS, IBMPC, PCJR, PCXT, or PCAT.\n", cli_bios);
              return 1;
          }
      }
      if (mtype != MMAP_NONE) {
          memmap_init(runtime.mem_segment, mtype);
          runtime.memmap_type = (int)mtype;
          runtime.mem_seg_base = 0;
#ifndef BPP_LITE_BUILD
          extern struct GW_Memory *g_gw_mem;
          if (g_gw_mem != NULL) {
              gw_mem_def_seg(g_gw_mem, 0);
          }
#endif
      } else {
          runtime.memmap_type = (int)MMAP_NONE;
      }
  }

  if (!eff_quiet) {
      long free_bytes = memory.variable.size + runtime.strpool.size + (memory.program.capacity * sizeof(ProgramLine));
      if (free_bytes >= 1024L * 1024L * 1024L) {
          printf("%.1f GB FREE\n\n", (double)free_bytes / (1024.0 * 1024.0 * 1024.0));
      } else if (free_bytes >= 1024L * 1024L) {
          printf("%.1f MB FREE\n\n", (double)free_bytes / (1024.0 * 1024.0));
      } else {
          printf("%.1f KB FREE\n\n", (double)free_bytes / 1024.0);
      }
  }
 }

 // ----- Handle -c (batch command) mode -----
 if (cli_command != NULL) {
  Lexer lex;
  error_clear();
  mem_pool_reset(&memory.scratch);
  lexer_init(&lex, cli_command);
  parser_execute_line(&lex, &runtime, 0);
  fflush(stdout);
  runtime_cleanup(&runtime);
  mem_shutdown(&memory);
  return error_occurred() ? 1 : 0;
 }

 // ----- Handle -r (run file) mode -----
 if (cli_run_file != NULL) {
  char load_cmd[INPUT_BUFFER_SIZE];
  Lexer lex;

  // Build LOAD command
  sprintf(load_cmd, "LOAD \"%s\"", cli_run_file);
  error_clear();
  mem_pool_reset(&memory.scratch);
  lexer_init(&lex, load_cmd);
  parser_execute_line(&lex, &runtime, 0);

  if (!error_occurred()) {
   // RUN the loaded program
   error_clear();
   mem_pool_reset(&memory.scratch);
   lexer_init(&lex, "RUN");
   parser_execute_line(&lex, &runtime, 0);
  }

  if (runtime.log_fp && runtime.log_fp != stderr && runtime.log_fp != stdout) {
      fclose((FILE*)runtime.log_fp);
      runtime.log_fp = NULL;
  }
  if (g_out_fp) {
      fclose(g_out_fp);
      g_out_fp = NULL;
  }
  runtime_cleanup(&runtime);
  mem_shutdown(&memory);
  return 0;
 }

 // ----- Handle --edit (screen editor mode) -----
  if (cli_edit) {
   printf("Starting BASIC++ Editor (Memo Pad Mode)\n");
   printf("Press Ctrl+Z (Windows) or Ctrl+D (Unix) to exit.\n");
   for (;;) {
    if (gw_console_read_line(input_buf, INPUT_BUFFER_SIZE) == NULL) {
     break; // EOF
    }
    // In a true full-screen mode, this would hook into vdev_display.
       // For now, it acts as a simple text buffer that doesn't scroll
       //       beyond screen limits (simulated). 
   }
   printf("\nExiting editor.\n");
   if (runtime.log_fp && runtime.log_fp != stderr && runtime.log_fp != stdout) { fclose((FILE*)runtime.log_fp); }
   runtime.log_fp = NULL;
   if (g_out_fp) { fclose(g_out_fp); g_out_fp = NULL; }
   mem_shutdown(&memory);
   return 0;
  }

 // ----- REPL loop -----
 printf("%s\n", BASICPP_READY);

 for (;;) {
 int line_num;
 int end_pos;

 // --- AUTO mode prompt ---
 if (runtime.auto_line > 0) {
 // AUTO mode is active. Print the line number
 // as the prompt. If the user enters text, it
 // becomes the content of that line. If the
 // user enters an empty line or '.', AUTO
 // mode is cancelled.
 printf("%d ", runtime.auto_line);
 fflush(stdout);

  if (gw_console_read_line(input_buf, INPUT_BUFFER_SIZE) == NULL) {
  // EOF - exit AUTO and interpreter
  printf("\n");
  break;
  }
 strip_newline(input_buf);

 // Cancel on empty line or '.'
 if (input_buf[0] == '\0' ||
 (input_buf[0] == '.' &&
 input_buf[1] == '\0')) {
 runtime.auto_line = 0;
 runtime.auto_step = 0;
 printf("%s\n", BASICPP_READY);
 continue;
 }

 // Build full line: "linenum content"
 {
 char full_line[INPUT_BUFFER_SIZE + 16];
 sprintf(full_line, "%d %s",
  (int)runtime.auto_line, input_buf);
 lexer_normalize_line(full_line);
 program_insert(&memory.program,
 runtime.auto_line, full_line);
 }

 // Advance to next line
 runtime.auto_line += runtime.auto_step;
 if (runtime.auto_line > LINE_NUMBER_MAX) {
 runtime.auto_line = 0;
 runtime.auto_step = 0;
 printf("Line number overflow.\n");
 printf("%s\n", BASICPP_READY);
 }
 continue;
 }

 // Print prompt
 if (rpn_is_active(&runtime.rpn)) {
  printf("RPN> ");
 } else {
  printf("%s", BASICPP_PROMPT);
 }
 fflush(stdout);

 // Read input line
  if (gw_console_read_line(input_buf, INPUT_BUFFER_SIZE) == NULL) {
  // EOF - exit gracefully
  printf("\n");
  break;
  }

 // Strip trailing newline
 strip_newline(input_buf);

 // Skip empty lines
 if (input_buf[0] == '\0') {
 continue;
 }

 // --- RPN mode: intercept all input ---
 if (rpn_is_active(&runtime.rpn)) {
  rpn_eval_line(&runtime.rpn, input_buf);
  continue;
 }

 // Clear any previous error state
 error_clear();

 // Reset scratch pool for this input cycle
 mem_pool_reset(&memory.scratch);

 // ----- Classify input -----
 line_num = parse_line_number(input_buf, &end_pos);

 if (line_num > 0) {
  if (runtime.bytecode_only) {
      printf("Line editing is prohibited in obfuscated/bytecode-only mode.\n");
      continue;
  }
 // Input starts with a line number.
 //
 // If the rest of the line is blank, this is a
 // delete command (entering just "10" deletes line 10).
 //
 // Otherwise, store the entire line (including the
 // line number prefix) in the program store.
 if (line_num > LINE_NUMBER_MAX) {
 error_raise(ERR_HOW, 0);
 } else if (is_blank_after(input_buf, end_pos)) {
 // Delete line
 program_delete(&memory.program, line_num);
 } else {
 // Store line
 lexer_normalize_line(input_buf);
 program_insert(&memory.program, line_num, input_buf);
 }
 } else {
   // Immediate mode - tokenize and execute.
   //
   // The input is a command or statement to execute
   // right now (e.g., "PRINT 2+3" or "RUN" or "LIST").
   //
   // NOTE: This is direct user input at the REPL, NOT
   // dynamic string evaluation. SECOP_EVAL is only meant
   // to gate programmatic eval (e.g., EXEC "code" from
   // within a running BASIC program). The interactive
   // REPL must always be available regardless of security
   // level - individual commands enforce their own security
   // checks (e.g., SAVE checks SECOP_FILE_WRITE, COMPILE
   // checks SECOP_COMPILE, etc.).
#ifndef BPP_LITE_BUILD
    int bypass = 0;
    {
        Lexer chk;
        lexer_init(&chk, input_buf);
        if (chk.current.type == TOK_KEYWORD) {
            KeywordId kw = chk.current.value.keyword;
            if (kw == KW_RUN || kw == KW_LOAD || kw == KW_SAVE ||
                kw == KW_LIST || kw == KW_NEW || kw == KW_CONT ||
                kw == KW_SYSTEM || kw == KW_AUTO || kw == KW_RENUM ||
                kw == KW_EDIT || kw == KW_MERGE || kw == KW_CHAIN ||
                kw == KW_DIALECT || kw == KW_BIOS || kw == KW_DELETE ||
                kw == KW_COMPILE || kw == KW_SELFTEST) {
                bypass = 1;
            }
        }
    }
    if (!runtime.direct_mode && !bypass) {
        ProgramStore temp_prog;
       ProgramLine temp_line;
       PCodeProgram pcode;
       int result;

       temp_line.line_number = 0;
       temp_line.text = input_buf;

       temp_prog.lines = &temp_line;
       temp_prog.count = 1;
       temp_prog.capacity = 1;
       temp_prog.bulk_buffer = NULL;
       temp_prog.bulk_size = 0;

       if (pcode_compile(&temp_prog, &pcode) == 0) {
           vm_set_state(&runtime, VM_RUNNING);
           g_arithmetic_decimal = runtime.arithmetic_decimal;
           result = vm_exec_pcode(&runtime, &pcode);
           pcode_free(&pcode);
           if (result != 0 && vm_get_state(&runtime) != VM_PAUSED) {
               vm_set_state(&runtime, VM_ERROR);
           } else if (vm_get_state(&runtime) != VM_PAUSED) {
               vm_set_state(&runtime, VM_STOPPED);
           }
       } else {
           Lexer lex;
           lexer_init(&lex, input_buf);
           parser_execute_line(&lex, &runtime, 0);
       }
   } else
#endif
   {
       Lexer lex;
       lexer_init(&lex, input_buf);
       parser_execute_line(&lex, &runtime, 0);
   }
  }

 // After any error, print READY again
 if (error_occurred()) {
 printf("%s\n", BASICPP_READY);
 }
 }

 // ----- Shutdown (reverse phase order) -----
  runtime_cleanup(&runtime);
  boot_shutdown(&memory);

#ifdef _WIN32
 // Prevent console window from closing instantly when launched from Windows Explorer
 if (cli_run_file == NULL && cli_command == NULL && !cli_edit) {
  printf("Interpreter exited.\nPress Enter to exit...");
  fflush(stdout);
  getchar();
 }
#endif

  if (runtime.log_fp && runtime.log_fp != stderr && runtime.log_fp != stdout) {
      fclose((FILE*)runtime.log_fp);
  }
  runtime.log_fp = NULL;
  if (g_out_fp) {
      fclose(g_out_fp);
      g_out_fp = NULL;
  }
  return 0;
}
#endif
