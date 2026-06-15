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
#include "platform.h"

/* --- State ---
 */
static SecLevel current_level = SEC_OPEN;

/* --- Permission Matrix ---
 * allowed[level][operation] - 1 = permitted, 0 = denied
 *
 * Index mapping:
 *  0=FILE_READ  1=FILE_WRITE 2=FILE_MGMT  3=FILE_BLOCK
 *  4=FILE_STRM  5=COMPILE    6=CHAIN      7=SYSTEM
 *  8=MODULE     9=USB       10=VDEV      11=VTERM
 * 12=VCON      13=EVAL      14=NETWORK   15=MEM_READ
 * 16=MEM_WRITE
 */
static const int allowed[SEC_COUNT][SECOP_COUNT] = {
    /* SEC_OPEN: all operations permitted */
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    /* SEC_STANDARD: file I/O + modules + eval + vdev + network yes;
     * compile/chain/system/usb no.
     * Network is port-gated via security_check_port(). */
    { 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
    /* SEC_RESTRICTED: nothing sensitive permitted */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

/* --- Operation names (for error messages) ---
 */
static const char *op_names[SECOP_COUNT] = {
    "file read",
    "file write",
    "file management",
    "block I/O",
    "stream I/O",
    "compile",
    "chain",
    "system",
    "module activation",
    "usb access",
    "virtual device access",
    "terminal intercept",
    "console hijack",
    "dynamic string evaluation",
    "network sockets",
    "memory read (PEEK)",
    "memory write (POKE)"
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
                            CAP_NETWORK | CAP_USB)) {
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

/* --- security_check_mem ---
 * Validates hardware memory bounds.
 */
int security_check_mem(unsigned long address, int size)
{
    const PlatformInfo *plat;
    
    if (current_level == SEC_OPEN) return 0;

    plat = platform_get_info();
    if (plat->id == PLAT_DOS) {
        /* FreeDOS: Allow hardware memory access under STANDARD */
        if (current_level == SEC_STANDARD) return 0;
    } else {
        /* Windows / Linux: Native pointer access is extremely dangerous.
         * Unless running in SEC_OPEN, we restrict memory mapping tightly.
         * For now, we will deny all non-sandboxed memory requests unless SEC_OPEN.
         * (If BASIC++ has a simulated 64K memory array mapped at 0x0000, 
         * we would check bounds against that block here.)
         */
        
        /* If it's a simulated pointer range (e.g., 0x0000 to 0xFFFF)
           we can allow it. But real pointer numbers are blocked. */
        if (address < 0x10000) {
            return 0; /* Simulated 64K RAM block */
        }
    }

    printf("SORRY? Security: memory access at 0x%lX not permitted at level %s\n",
           address, level_names[current_level]);
    return -1;
}

/* --- security_check_port ---
 * Port-based network access control.
 *
 * Well-known service ports allowed at STANDARD:
 *   FTP(21), SSH(22), Telnet(23), SMTP(25), DNS(53),
 *   HTTP(80), POP3(110), NNTP(119), IMAP(143), SNMP(161/162),
 *   HTTPS(443), SMTPS(465), SMTP-SUB(587), IMAPS(993),
 *   POP3S(995), IRC(6667/6697), HTTP-ALT(8080/8443),
 *   TNFS(16384).
 *   Ephemeral ports 1024-49151 also allowed (client-side).
 */
int security_check_port(int port, int line_num)
{
    /* SEC_OPEN: all ports */
    if (current_level == SEC_OPEN) return 0;

    /* SEC_RESTRICTED: no network at all */
    if (current_level == SEC_RESTRICTED) {
        printf("SORRY? Security: network access "
               "not permitted at level %s",
               level_names[current_level]);
        if (line_num > 0)
            printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

    /* SEC_STANDARD: well-known + ephemeral ports */
    if (port >= 1024 && port <= 49151) return 0;

    switch (port) {
    case 21:    /* FTP */
    case 22:    /* SSH */
    case 23:    /* Telnet */
    case 25:    /* SMTP */
    case 53:    /* DNS */
    case 80:    /* HTTP */
    case 110:   /* POP3 */
    case 119:   /* NNTP */
    case 143:   /* IMAP */
    case 161:   /* SNMP */
    case 162:   /* SNMP Trap */
    case 443:   /* HTTPS */
    case 465:   /* SMTPS */
    case 587:   /* SMTP Submission */
    case 993:   /* IMAPS */
    case 995:   /* POP3S */
    case 6667:  /* IRC */
    case 6697:  /* IRC over TLS */
    case 8080:  /* HTTP alternate */
    case 8443:  /* HTTPS alternate */
    case 16384: /* TNFS */
        return 0;
    default:
        break;
    }

    printf("SORRY? Security: port %d "
           "not permitted at level %s",
           port, level_names[current_level]);
    if (line_num > 0)
        printf(" in line %d", line_num);
    printf("\n");
    return -1;
}
