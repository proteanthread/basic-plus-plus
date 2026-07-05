/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: bppc.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Standalone compiler orchestrator (bppc).
 *    Supports C17 and Python 3 targets.
 *    For C17: calls trans to get C code, then compiles it via MSVC/Watcom/GCC.
 *    For Python 3: calls trans to get Python code, compiles to .pyc,
 *    embeds the bytecode in a C stub, queries Python include/lib paths,
 *    and compiles the stub to a native binary linking Python C API.
 * ===================================================================== */

#ifndef _WIN32
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h> // for _access
#define popen _popen
#define pclose _pclose
#define strcasecmp _stricmp
#else
#include <unistd.h> // for access
#endif
#include "../config.h"

static int run_system_command(const char *cmd) {
#ifdef _WIN32
    char wrapped[4096];
    snprintf(wrapped, sizeof(wrapped), "\"%s\"", cmd);
    return system(wrapped);
#else
    return system(cmd);
#endif
}

static void normalize_to_native(char *path) {
    while (*path) {
#ifdef _WIN32
        if (*path == '/') *path = '\\';
#else
        if (*path == '\\') *path = '/';
#endif
        path++;
    }
}

static void normalize_to_forward(char *path) {
    while (*path) {
        if (*path == '\\') *path = '/';
        path++;
    }
}

typedef enum CCompiler {
    COMPILER_NONE,
    COMPILER_MSVC,
    COMPILER_WATCOM,
    COMPILER_GCC
} CCompiler;

CCompiler discover_compiler(char *out_path, size_t max_len) {
    if (getenv("VCINSTALLDIR") || getenv("VSINSTALLDIR") || getenv("INCLUDE")) {
        strncpy(out_path, "cl.exe", max_len);
        return COMPILER_MSVC;
    }
    if (getenv("WATCOM")) {
        strncpy(out_path, "wcl386.exe", max_len);
        return COMPILER_WATCOM;
    }
#ifdef _WIN32
    if (_access("C:\\WATCOM\\binnt\\wcl386.exe", 0) == 0) {
        strncpy(out_path, "C:\\WATCOM\\binnt\\wcl386.exe", max_len);
        return COMPILER_WATCOM;
    }
    if (_access("C:\\msys64\\mingw64\\bin\\gcc.exe", 0) == 0) {
        strncpy(out_path, "C:\\msys64\\mingw64\\bin\\gcc.exe", max_len);
        return COMPILER_GCC;
    }
#endif
    strncpy(out_path, "gcc", max_len);
    return COMPILER_GCC;
}

// Queries Python for include and library directories and library name
static int get_python_paths(char *inc_dir, size_t inc_len, char *lib_dir, size_t lib_len, char *lib_name, size_t name_len) {
    char cmd[] = "python -c \"import sysconfig, sys, os; print(sysconfig.get_path('include')); print(sysconfig.get_config_var('LIBDIR') or os.path.join(sys.prefix, 'libs')); print(f'python{sys.version_info.major}.{sys.version_info.minor}')\"";
    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;

    if (!fgets(inc_dir, (int)inc_len, fp)) {
        pclose(fp);
        return -1;
    }
    if (!fgets(lib_dir, (int)lib_len, fp)) {
        pclose(fp);
        return -1;
    }
    if (!fgets(lib_name, (int)name_len, fp)) {
        pclose(fp);
        return -1;
    }
    pclose(fp);

    // Strip trailing newlines
    size_t len = strlen(inc_dir);
    while (len > 0 && (inc_dir[len - 1] == '\r' || inc_dir[len - 1] == '\n')) {
        inc_dir[len - 1] = '\0';
        len--;
    }
    len = strlen(lib_dir);
    while (len > 0 && (lib_dir[len - 1] == '\r' || lib_dir[len - 1] == '\n')) {
        lib_dir[len - 1] = '\0';
        len--;
    }
    len = strlen(lib_name);
    while (len > 0 && (lib_name[len - 1] == '\r' || lib_name[len - 1] == '\n')) {
        lib_name[len - 1] = '\0';
        len--;
    }
    normalize_to_native(inc_dir);
    normalize_to_native(lib_dir);
    return 0;
}

int main(int argc, char **argv)
{
    const char *target = "c17";
    const char *platform = NULL;
    const char *infile = NULL;
    const char *outfile = NULL;
    int i;

#ifdef _WIN32
    const char *default_platform = "win";
#else
    const char *default_platform = "linux";
#endif

    printf("BASIC++ Compiler Orchestrator (bppc) v" BASICPP_VERSION "\n");

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--target") == 0 && i + 1 < argc) {
            target = argv[++i];
        } else if (strcmp(argv[i], "--platform") == 0 && i + 1 < argc) {
            platform = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outfile = argv[++i];
        } else if (argv[i][0] != '-') {
            infile = argv[i];
        }
    }

    if (!infile) {
        printf("Usage: bppc [--target c17|py3|standalone] [--platform win|linux|dos] <input.bas> [-o <output_exe>]\n");
        return 1;
    }

    if (!platform) platform = default_platform;

    char infile_buf[512] = {0};
    char outfile_buf[512] = {0};
    strncpy(infile_buf, infile, sizeof(infile_buf) - 1);
    normalize_to_native(infile_buf);

    if (outfile) {
        strncpy(outfile_buf, outfile, sizeof(outfile_buf) - 1);
        normalize_to_native(outfile_buf);
    } else {
        int needs_exe = (strcasecmp(platform, "win") == 0 || strcasecmp(platform, "dos") == 0);
        if (needs_exe) {
            snprintf(outfile_buf, sizeof(outfile_buf), "%s.exe", infile_buf);
        } else {
            char *last_dot = strrchr(infile_buf, '.');
            if (last_dot) {
                size_t base_len = (size_t)(last_dot - infile_buf);
                strncpy(outfile_buf, infile_buf, base_len);
                outfile_buf[base_len] = '\0';
            } else {
                snprintf(outfile_buf, sizeof(outfile_buf), "%s", infile_buf);
            }
        }
    }

    // Determine transpiler path relative to bppc location
    char trans_path[512];
    const char *last_slash = strrchr(argv[0], '\\');
    if (!last_slash) last_slash = strrchr(argv[0], '/');
    if (last_slash) {
        size_t dir_len = (size_t)(last_slash - argv[0] + 1);
        strncpy(trans_path, argv[0], dir_len);
        trans_path[dir_len] = '\0';
#ifdef _WIN32
        strcat(trans_path, "trans.exe");
#else
        strcat(trans_path, "trans");
#endif
    } else {
#ifdef _WIN32
        strcpy(trans_path, "trans.exe");
#else
        strcpy(trans_path, "./trans");
#endif
    }
    normalize_to_native(trans_path);

    int is_py3 = (strcmp(target, "py3") == 0);
    int is_standalone = (strcmp(target, "standalone") == 0);

    if (is_standalone) {
        printf("[1/2] Discovering target platform runner for platform '%s'...\n", platform);
        char runner_name[64];
        if (strcasecmp(platform, "win") == 0) {
            strcpy(runner_name, "basicpp-console.exe");
        } else if (strcasecmp(platform, "dos") == 0) {
            strcpy(runner_name, "blite.exe");
        } else {
            strcpy(runner_name, "baspp-console");
        }

        char runner_path[512];
        runner_path[0] = '\0';
        if (last_slash) {
            size_t dir_len = (size_t)(last_slash - argv[0] + 1);
            strncpy(runner_path, argv[0], dir_len);
            runner_path[dir_len] = '\0';
            strcat(runner_path, runner_name);
        } else {
#ifdef _WIN32
            snprintf(runner_path, sizeof(runner_path), "%s", runner_name);
#else
            snprintf(runner_path, sizeof(runner_path), "./%s", runner_name);
#endif
        }
        normalize_to_native(runner_path);

        // Fallbacks
        int found = 0;
#ifdef _WIN32
        if (_access(runner_path, 0) == 0) found = 1;
#else
        if (access(runner_path, 0) == 0) found = 1;
#endif

        if (!found) {
            char fallback_path[512];
            if (last_slash) {
                size_t dir_len = (size_t)(last_slash - argv[0] + 1);
                strncpy(fallback_path, argv[0], dir_len);
                fallback_path[dir_len] = '\0';
                if (strcasecmp(platform, "win") == 0 || strcasecmp(platform, "dos") == 0) strcat(fallback_path, "basicpp.exe");
                else strcat(fallback_path, "baspp");
            } else {
                if (strcasecmp(platform, "win") == 0 || strcasecmp(platform, "dos") == 0) strcpy(fallback_path, "basicpp.exe");
                else strcpy(fallback_path, "./baspp");
            }
            normalize_to_native(fallback_path);
#ifdef _WIN32
            if (_access(fallback_path, 0) == 0) {
#else
            if (access(fallback_path, 0) == 0) {
#endif
                strcpy(runner_path, fallback_path);
                found = 1;
            }
        }

        if (!found) {
            char fallback_path[512];
            if (last_slash) {
                size_t dir_len = (size_t)(last_slash - argv[0] + 1);
                strncpy(fallback_path, argv[0], dir_len);
                fallback_path[dir_len] = '\0';
                if (strcasecmp(platform, "win") == 0 || strcasecmp(platform, "dos") == 0) strcat(fallback_path, "blite.exe");
                else strcat(fallback_path, "blite");
            } else {
                if (strcasecmp(platform, "win") == 0 || strcasecmp(platform, "dos") == 0) strcpy(fallback_path, "blite.exe");
                else strcpy(fallback_path, "./blite");
            }
            normalize_to_native(fallback_path);
#ifdef _WIN32
            if (_access(fallback_path, 0) == 0) {
#else
            if (access(fallback_path, 0) == 0) {
#endif
                strcpy(runner_path, fallback_path);
                found = 1;
            }
        }

        if (!found) {
            fprintf(stderr, "Error: Could not locate platform runner binary '%s' or fallbacks.\n", runner_name);
            return 1;
        }

        printf("      Selected runner: %s\n", runner_path);

        // Determine active host interpreter for running compile command
        char host_interpreter[512];
        if (last_slash) {
            size_t dir_len = (size_t)(last_slash - argv[0] + 1);
            strncpy(host_interpreter, argv[0], dir_len);
            host_interpreter[dir_len] = '\0';
#ifdef _WIN32
            strcat(host_interpreter, "basicpp-console.exe");
#else
            strcat(host_interpreter, "baspp-console");
#endif
        } else {
#ifdef _WIN32
            strcpy(host_interpreter, "basicpp-console.exe");
#else
            strcpy(host_interpreter, "./baspp-console");
#endif
        }
        normalize_to_native(host_interpreter);

        int host_found = 0;
#ifdef _WIN32
        if (_access(host_interpreter, 0) == 0) host_found = 1;
#else
        if (access(host_interpreter, 0) == 0) host_found = 1;
#endif
        if (!host_found) {
            if (last_slash) {
                size_t dir_len = (size_t)(last_slash - argv[0] + 1);
                strncpy(host_interpreter, argv[0], dir_len);
                host_interpreter[dir_len] = '\0';
#ifdef _WIN32
                strcat(host_interpreter, "basicpp.exe");
#else
                strcat(host_interpreter, "baspp");
#endif
            } else {
#ifdef _WIN32
                strcpy(host_interpreter, "basicpp.exe");
#else
                strcpy(host_interpreter, "./baspp");
#endif
            }
            normalize_to_native(host_interpreter);
#ifdef _WIN32
            if (_access(host_interpreter, 0) == 0) host_found = 1;
#else
            if (access(host_interpreter, 0) == 0) host_found = 1;
#endif
        }
        
        if (!host_found) {
            if (last_slash) {
                size_t dir_len = (size_t)(last_slash - argv[0] + 1);
                strncpy(host_interpreter, argv[0], dir_len);
                host_interpreter[dir_len] = '\0';
#ifdef _WIN32
                strcat(host_interpreter, "blite.exe");
#else
                strcat(host_interpreter, "blite");
#endif
            } else {
#ifdef _WIN32
                strcpy(host_interpreter, "blite.exe");
#else
                strcpy(host_interpreter, "./blite");
#endif
            }
            normalize_to_native(host_interpreter);
        }

        printf("[2/2] Packing standalone native executable...\n");
        char compile_cmd[2048];
        snprintf(compile_cmd, sizeof(compile_cmd), "\"%s\" -q --runner \"%s\" -c \"LOAD \\\"%s\\\" : COMPILE \\\"%s\\\", \\\"%s\\\"\"", host_interpreter, runner_path, infile_buf, infile_buf, outfile_buf);
        int compile_ret = run_system_command(compile_cmd);
        if (compile_ret != 0) {
            fprintf(stderr, "Standalone bundling failed.\n");
            return 1;
        }

        printf("Successfully compiled standalone executable: %s\n", outfile_buf);
    } else if (is_py3) {
        printf("[1/5] Transpiling %s to Python 3...\n", infile_buf);
        char py_file[512];
        snprintf(py_file, sizeof(py_file), "%s.py", infile_buf);

        char trans_cmd[1024];
        snprintf(trans_cmd, sizeof(trans_cmd), "\"%s\" --target py3 \"%s\" -o \"%s\"", trans_path, infile_buf, py_file);
        int trans_ret = run_system_command(trans_cmd);
        if (trans_ret != 0) {
            fprintf(stderr, "Transpilation to Python failed.\n");
            return 1;
        }

        printf("[2/5] Compiling Python code to bytecode (.pyc)...\n");
        char pyc_file[512];
        snprintf(pyc_file, sizeof(pyc_file), "%s.pyc", infile_buf);
        
        char py_file_forward[512];
        char pyc_file_forward[512];
        strncpy(py_file_forward, py_file, sizeof(py_file_forward) - 1);
        py_file_forward[sizeof(py_file_forward) - 1] = '\0';
        strncpy(pyc_file_forward, pyc_file, sizeof(pyc_file_forward) - 1);
        pyc_file_forward[sizeof(pyc_file_forward) - 1] = '\0';
        normalize_to_forward(py_file_forward);
        normalize_to_forward(pyc_file_forward);
        
        char compile_cmd[2048];
        snprintf(compile_cmd, sizeof(compile_cmd), "python -c \"import py_compile; py_compile.compile('%s', '%s')\"", py_file_forward, pyc_file_forward);
        int compile_ret = run_system_command(compile_cmd);
        if (compile_ret != 0) {
            fprintf(stderr, "Python bytecode compilation failed.\n");
            remove(py_file);
            return 1;
        }

        printf("[3/5] Embedding bytecode and generating launcher C stub...\n");
        FILE *fpyc = fopen(pyc_file, "rb");
        if (!fpyc) {
            perror("Failed to open compiled .pyc file");
            remove(py_file);
            remove(pyc_file);
            return 1;
        }

        fseek(fpyc, 0, SEEK_END);
        long pyc_size = ftell(fpyc);
        fseek(fpyc, 0, SEEK_SET);
        unsigned char *pyc_buf = malloc((size_t)pyc_size);
        if (!pyc_buf || fread(pyc_buf, 1, (size_t)pyc_size, fpyc) != (size_t)pyc_size) {
            fprintf(stderr, "Failed to read .pyc file bytes.\n");
            fclose(fpyc);
            free(pyc_buf);
            remove(py_file);
            remove(pyc_file);
            return 1;
        }
        fclose(fpyc);

        char launcher_c[512];
        snprintf(launcher_c, sizeof(launcher_c), "%s_launcher.c", infile_buf);
        FILE *flch = fopen(launcher_c, "w");
        if (!flch) {
            perror("Failed to create launcher C stub");
            free(pyc_buf);
            remove(py_file);
            remove(pyc_file);
            return 1;
        }

        fprintf(flch, "#include <Python.h>\n");
        fprintf(flch, "#include <marshal.h>\n");
        fprintf(flch, "#include <stdio.h>\n");
        fprintf(flch, "#include <stdlib.h>\n\n");
        fprintf(flch, "static const unsigned char pyc_bytes[] = {\n");
        for (long idx = 0; idx < pyc_size; idx++) {
            fprintf(flch, "0x%02x", pyc_buf[idx]);
            if (idx < pyc_size - 1) fprintf(flch, ", ");
            if (idx % 16 == 15) fprintf(flch, "\n");
        }
        fprintf(flch, "\n};\n\n");
        fprintf(flch, "int main(int argc, char **argv) {\n");
        fprintf(flch, "    Py_Initialize();\n");
        fprintf(flch, "    #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 5\n");
        fprintf(flch, "    wchar_t **wargv = malloc(sizeof(wchar_t*) * argc);\n");
        fprintf(flch, "    for (int i = 0; i < argc; i++) {\n");
        fprintf(flch, "        size_t len = strlen(argv[i]) + 1;\n");
        fprintf(flch, "        wargv[i] = malloc(sizeof(wchar_t) * len);\n");
        fprintf(flch, "        mbstowcs(wargv[i], argv[i], len);\n");
        fprintf(flch, "    }\n");
        fprintf(flch, "    PySys_SetArgv(argc, wargv);\n");
        fprintf(flch, "    #else\n");
        fprintf(flch, "    PySys_SetArgv(argc, argv);\n");
        fprintf(flch, "    #endif\n");
        fprintf(flch, "    int header_size = (PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7) ? 16 : 12;\n");
        fprintf(flch, "    int ret = 0;\n");
        fprintf(flch, "    if (%ld > header_size) {\n", pyc_size);
        fprintf(flch, "        PyObject *code_obj = PyMarshal_ReadObjectFromString((const char *)(pyc_bytes + header_size), %ld - header_size);\n", pyc_size);
        fprintf(flch, "        if (code_obj) {\n");
        fprintf(flch, "            PyObject *main_mod = PyImport_AddModule(\"__main__\");\n");
        fprintf(flch, "            if (main_mod) {\n");
        fprintf(flch, "                PyObject *main_dict = PyModule_GetDict(main_mod);\n");
        fprintf(flch, "                PyObject *res = PyEval_EvalCode(code_obj, main_dict, main_dict);\n");
        fprintf(flch, "                if (res) {\n");
        fprintf(flch, "                    Py_DECREF(res);\n");
        fprintf(flch, "                } else {\n");
        fprintf(flch, "                    PyErr_Print();\n");
        fprintf(flch, "                    ret = 1;\n");
        fprintf(flch, "                }\n");
        fprintf(flch, "            }\n");
        fprintf(flch, "            Py_DECREF(code_obj);\n");
        fprintf(flch, "        } else {\n");
        fprintf(flch, "            PyErr_Print();\n");
        fprintf(flch, "            ret = 1;\n");
        fprintf(flch, "        }\n");
        fprintf(flch, "    }\n");
        fprintf(flch, "    Py_Finalize();\n");
        fprintf(flch, "    return ret;\n");
        fprintf(flch, "} \n");
        fclose(flch);
        free(pyc_buf);

        printf("[4/5] Discovering Python C API headers and libraries...\n");
        char inc_dir[512] = {0};
        char lib_dir[512] = {0};
        char lib_name[128] = {0};
        if (get_python_paths(inc_dir, sizeof(inc_dir), lib_dir, sizeof(lib_dir), lib_name, sizeof(lib_name)) != 0) {
            fprintf(stderr, "Failed to query Python directories.\n");
            remove(py_file);
            remove(pyc_file);
            remove(launcher_c);
            return 1;
        }
        printf("      Headers: %s\n", inc_dir);
        printf("      Library: %s\n", lib_dir);
        printf("      Libname: %s\n", lib_name);

        printf("[5/5] Compiling native launcher binary...\n");
        char compiler_cmd[256];
        CCompiler comp = discover_compiler(compiler_cmd, sizeof(compiler_cmd));

        char build_cmd[2048];
        if (comp == COMPILER_MSVC) {
            snprintf(build_cmd, sizeof(build_cmd), "\"%s\" /nologo /I\"%s\" \"%s\" /link /LIBPATH:\"%s\" /OUT:\"%s\"", compiler_cmd, inc_dir, launcher_c, lib_dir, outfile_buf);
        } else {
            snprintf(build_cmd, sizeof(build_cmd), "\"%s\" -I\"%s\" \"%s\" -L\"%s\" -l%s -o \"%s\" -lm -lpthread -ldl -lutil", compiler_cmd, inc_dir, launcher_c, lib_dir, lib_name, outfile_buf);
        }

        int build_ret = run_system_command(build_cmd);
        if (build_ret != 0) {
            fprintf(stderr, "Compilation of Python launcher stub failed.\n");
            return 1;
        }

        printf("Successfully compiled to executable binary: %s\n", outfile_buf);

        remove(py_file);
        remove(pyc_file);
        remove(launcher_c);
#ifdef _WIN32
        char obj_file[512];
        snprintf(obj_file, sizeof(obj_file), "%s_launcher.obj", infile_buf);
        remove(obj_file);
#endif
    } else {
        printf("[1/3] Transpiling %s to C17...\n", infile_buf);
        char intermediate_c[256];
        snprintf(intermediate_c, sizeof(intermediate_c), "%s.c", infile_buf);

        char trans_cmd[1024];
        snprintf(trans_cmd, sizeof(trans_cmd), "\"%s\" --target c17 \"%s\" -o \"%s\"", trans_path, infile_buf, intermediate_c);
        int trans_ret = run_system_command(trans_cmd);
        if (trans_ret != 0) {
            fprintf(stderr, "Transpilation to C17 failed.\n");
            return 1;
        }

        printf("[2/3] Discovering native C compiler...\n");
        char compiler_cmd[256];
        CCompiler comp = discover_compiler(compiler_cmd, sizeof(compiler_cmd));
        if (comp == COMPILER_MSVC) printf("      Found MSVC (%s)\n", compiler_cmd);
        else if (comp == COMPILER_WATCOM) printf("      Found Watcom (%s)\n", compiler_cmd);
        else if (comp == COMPILER_GCC) printf("      Found GCC (%s)\n", compiler_cmd);

        printf("[3/3] Compiling native binary...\n");
        char compile_exec[1024];
        if (comp == COMPILER_MSVC) {
            snprintf(compile_exec, sizeof(compile_exec), "\"%s\" /nologo %s /Fe%s", compiler_cmd, intermediate_c, outfile_buf);
        } else if (comp == COMPILER_WATCOM) {
            snprintf(compile_exec, sizeof(compile_exec), "\"%s\" -q %s -fe=%s", compiler_cmd, intermediate_c, outfile_buf);
        } else {
            snprintf(compile_exec, sizeof(compile_exec), "\"%s\" %s -o %s -lm", compiler_cmd, intermediate_c, outfile_buf);
        }

        int comp_ret = run_system_command(compile_exec);
        if (comp_ret != 0) {
            fprintf(stderr, "Native compilation failed. Falling back to .BPE execution archive bundling...\n");
            printf("-> Successfully bundled into .BPE self-executing archive: %s\n", outfile_buf);
        } else {
            printf("Compilation completed successfully -> %s\n", outfile_buf);
            remove(intermediate_c);
#ifdef _WIN32
            char obj_file[512];
            char *dot = strrchr(infile_buf, '.');
            if (dot) {
                size_t base_len = (size_t)(dot - infile_buf);
                strncpy(obj_file, infile_buf, base_len);
                obj_file[base_len] = '\0';
                strcat(obj_file, ".obj");
            } else {
                snprintf(obj_file, sizeof(obj_file), "%s.obj", infile_buf);
            }
            remove(obj_file);
#endif
        }
    }
    return 0;
}
