/*
 * ---
 * BASIC++ Interpreter - security.c
 * ---
 *
 * Security system implementation.
 *
 * IMPLEMENTATION:
 * Static permission matrix indexed by [level][operation].
 * security_check() does a single array lookup and raises
 * ERR_SORRY on denial. Zero overhead when SEC_OPEN.
 *
 * PERMISSION MATRIX:
 * Operation OPEN STANDARD RESTRICTED
 * --------------- ---- -------- ----------
 * FILE_READ 1 1 0
 * FILE_WRITE 1 1 0
 * COMPILE 1 0 0
 * CHAIN 1 0 0
 * SYSTEM 1 0 0
 * MODULE 1 1 0
 *
 * ---
 */

#include <stdio.h>
#include "security.h"
#include "module.h"

/* --- State ---
 */
static SecLevel current_level = SEC_OPEN;

/* --- Permission Matrix ---
 * allowed[level][operation] - 1 = permitted, 0 = denied
 */
static const int allowed[SEC_COUNT][SECOP_COUNT] = {
 /* SEC_OPEN: all operations permitted */
 { 1, 1, 1, 1, 1, 1 },
 /* SEC_STANDARD: file I/O + modules yes, compile/chain/system no */
 { 1, 1, 0, 0, 0, 1 },
 /* SEC_RESTRICTED: nothing sensitive permitted */
 { 0, 0, 0, 0, 0, 0 }
};

/* --- Operation names (for error messages) ---
 */
static const char *op_names[SECOP_COUNT] = {
 "file read",
 "file write",
 "compile",
 "chain",
 "system",
 "module activation"
};

/* --- Level names ---
 */
static const char *level_names[SEC_COUNT] = {
 "OPEN",
 "STANDARD",
 "RESTRICTED"
};

/* --- security_init ---
 */
void security_init(SecLevel level)
{
 if (level >= 0 && level < SEC_COUNT) {
 current_level = level;
 } else {
 current_level = SEC_OPEN;
 }
}

/* --- security_get_level ---
 */
SecLevel security_get_level(void)
{
 return current_level;
}

/* --- security_set_level ---
 */
void security_set_level(SecLevel level)
{
 if (level >= 0 && level < SEC_COUNT) {
 current_level = level;
 }
}

/* --- security_level_name ---
 */
const char *security_level_name(SecLevel level)
{
 if (level >= 0 && level < SEC_COUNT) {
 return level_names[level];
 }
 return "UNKNOWN";
}

/* --- security_check ---
 */
int security_check(SecOperation op, int line_num)
{
 /* Fast path: SEC_OPEN permits everything */
 if (current_level == SEC_OPEN) return 0;

 if (op < 0 || op >= SECOP_COUNT) return -1;

 if (allowed[current_level][op]) {
 return 0;
 }

 /* Denied - report error */
 printf("SORRY? Security: %s not permitted "
 "at level %s",
 op_names[op], level_names[current_level]);
 if (line_num > 0) {
 printf(" in line %d", line_num);
 }
 printf("\n");
 return -1;
}

/* --- security_module_allowed ---
 * Check if a module's capability set is compatible with the
 * current security level.
 *
 * Under RESTRICTED: no modules with IO/FILE/SYSTEM caps.
 * Under STANDARD: no modules with SYSTEM caps.
 * Under OPEN: everything allowed.
 */
int security_module_allowed(unsigned int capabilities)
{
 if (current_level == SEC_OPEN) return 1;

 if (current_level == SEC_RESTRICTED) {
 /* Only pure math/string modules allowed */
 if (capabilities & (CAP_IO | CAP_FILE | CAP_SYSTEM |
 CAP_GRAPHICS | CAP_SOUND |
 CAP_NETWORK)) {
 return 0;
 }
 return 1;
 }

 if (current_level == SEC_STANDARD) {
 /* No system-level modules */
 if (capabilities & CAP_SYSTEM) {
 return 0;
 }
 return 1;
 }

 return 1;
}
