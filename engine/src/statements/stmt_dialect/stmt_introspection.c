/**
 * @file stmt_introspection.c
 * @brief Handles shell-like introspection statement commands for network/user identity.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements HOSTNAME and USERNAME statements that output environment information directly.
 * - Why it exists: Provides direct MS-DOS or Unix shell integration diagnostics inside immediate mode.
 * - Why it works this way: It queries local OS configuration using standard platform wrapper fallback logic and flushes results directly to the VDev console stream.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Environment fallbacks or output format adjustments.
 * - What cannot be changed: BValue stack neutrality; these are console-printing statements with no expression return value.
 * - What to expect: Standard OS-level identity buffers, potentially fallback strings under sandboxed runs.
 * - What to do if something breaks: Verify platform-specific header definitions (GetComputerName/gethostname/getuid).
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: OS provides standard user and host identification APIs.
 * - Portability concerns: Fully compliant with strict C17 standards, utilizing clean compile-time platforms.
 */

#include "stmt/stmt.h"
#include "platform/platform.h"
#include "security/security.h"
#include "device/vdev.h"
#include <string.h>

BppError stmt_hostname_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    char name[256] = "";
    platform_get_hostname(name, sizeof(name));

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "%s\n", name);
    return err;
}

BppError stmt_username_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;

    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    char name[256] = "";
    platform_get_username(name, sizeof(name));

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "%s\n", name);
    return err;
}
