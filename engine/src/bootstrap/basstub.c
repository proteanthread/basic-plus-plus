/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file basstub.c
 * @brief Headless standalone runner stub for bundled BASIC++ programs.
 *
 * What it does: Opens its own executable file, reads the appended .BPP bytecode
 * payload at the end of the executable, loads it into the VM, and runs it.
 * Why it exists: Enables zero-toolchain direct compilation of BASIC++ programs
 * into native, standalone binary executables.
 * Why it works this way: Appends the compiled BPP bytecode followed by an 8-byte
 * footer (4-byte payload size + 4-byte 'BPPE' magic) to the end of the runner executable.
 * What can be changed: VM runtime configuration options.
 * What cannot be changed: Appended payload offset calculation and binary layout.
 * Assumptions: argv[0] contains a resolvable path to the executing binary.
 * Portability concerns: ANSI C17 compatible. Works on Windows and POSIX.
 */

#include "core/boot.h"
#include "device/vdev.h"
#include "vm/vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform/platform.h"

/* Forward declaration of stream loader */
BppError vm_bload_program_from_stream(VMContext *vm, FILE *fp);

int main(int argc, char **argv) {
    char exe_path[1024] = "";

    if (!platform_get_executable_path(exe_path, sizeof(exe_path))) {
        strncpy(exe_path, argv[0], sizeof(exe_path) - 1);
        exe_path[sizeof(exe_path) - 1] = '\0';
    }

    FILE *fp = fopen(exe_path, "rb");
    if (!fp) {
        fprintf(stderr, "FATAL: Standalone stub could not open own executable '%s'\n", exe_path);
        return 1;
    }

    /* Seek to footer (last 8 bytes) */
    if (fseek(fp, -8, SEEK_END) != 0) {
        fprintf(stderr, "FATAL: Executable size too small for standalone stub\n");
        fclose(fp);
        return 1;
    }

    long file_size = ftell(fp) + 8;

    unsigned char footer[8];
    if (fread(footer, 1, 8, fp) != 8) {
        fprintf(stderr, "FATAL: Failed to read standalone stub footer\n");
        fclose(fp);
        return 1;
    }

    /* Check magic signature 'BPPE' (0x45505042) */
    uint32_t magic = (uint32_t)(footer[4] | (footer[5] << 8) | (footer[6] << 16) | (footer[7] << 24));
    if (magic != 0x45505042) {
        fprintf(stderr, "FATAL: Standalone runner stub contains no compiled payload.\n");
        fclose(fp);
        return 1;
    }

    uint32_t payload_size = (uint32_t)(footer[0] | (footer[1] << 8) | (footer[2] << 16) | (footer[3] << 24));

    /* Seek to payload start */
    long payload_offset = file_size - 8 - (long)payload_size;
    if (fseek(fp, payload_offset, SEEK_SET) != 0) {
        fprintf(stderr, "FATAL: Failed to seek to standalone payload offset\n");
        fclose(fp);
        return 1;
    }

    /* Verify BPP magic signature */
    unsigned char bpp_magic[4];
    if (fread(bpp_magic, 1, 4, fp) != 4 || bpp_magic[0] != 'B' || bpp_magic[1] != 'P' || bpp_magic[2] != 'P' || bpp_magic[3] != 0x1A) {
        fprintf(stderr, "FATAL: Appended standalone payload contains invalid BPP magic\n");
        fclose(fp);
        return 1;
    }

    /* Position stream to load BPP body (magic already validated) */
    fseek(fp, payload_offset + 4, SEEK_SET);

    BootConfig config;
    memset(&config, 0, sizeof(config));
    config.is_repl = false;
    config.prog_mem = 67108864L;
    config.var_mem  = 67108864L;
    config.str_mem  = 67108864L;
    config.scratch_mem = 67108864L;

    BootContext *boot = boot_execute(&config);
    if (!boot) {
        fprintf(stderr, "FATAL: BASIC++ VM failed to boot.\n");
        fclose(fp);
        return 1;
    }

    BppError err = vm_bload_program_from_stream(boot->vm, fp);
    fclose(fp);

    if (err.code != 0) {
        fprintf(stderr, "FATAL: Failed to load appended standalone payload (Error %d: %s)\n", err.code, err.message);
        boot_shutdown(boot);
        return 1;
    }

    /* Run the bytecode program */
    vm_run_program(boot->vm);

    if (vm_has_error(boot->vm)) {
        BppError run_err = vm_get_error(boot->vm);
        VDevContext *vdev = vm_get_vdev(boot->vm);
        vdev_printf(vdev, "?Error %d: %s in line %g\n", run_err.code, run_err.message, run_err.line);
        boot_shutdown(boot);
        return 1;
    }

    boot_shutdown(boot);
    return 0;
}
