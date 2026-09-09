// FILENAME: system_var_descriptors.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libflex
// NEEDS: libcore (language_descriptor.h, system_var_descriptors.h)
// Implements authoritative descriptor definitions for built-in system variables.

#include "runtime/descriptors/system_var_descriptors.h"
#include "runtime/descriptors/builtin_descriptors.h"

static const LangDesc g_system_vars[] = {
    // 1. Math & Constants
    {
        .name = "PI", .category = "Math & Constants", .syntax = "PI or PI()",
        .description = "Exact mathematical constant pi (3.141592653589793).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TRUE", .category = "Math & Constants", .syntax = "TRUE or TRUE()",
        .description = "Boolean truth constant (-1 in classic BASIC, 1 in modern logic).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
    },
    {
        .name = "FALSE", .category = "Math & Constants", .syntax = "FALSE or FALSE()",
        .description = "Boolean false constant (0).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
    },
    {
        .name = "INF", .category = "Math & Constants", .syntax = "INF or INF()",
        .description = "Positive IEEE 754 floating-point infinity constant.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
    },
    {
        .name = "MAXNUM", .category = "Math & Constants", .syntax = "MAXNUM or MAXNUM()",
        .description = "Maximum representable double-precision floating-point number (approx 1.79769e+308).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
    },
    {
        .name = "EPS", .category = "Math & Constants", .syntax = "EPS or EPS()",
        .description = "Machine epsilon precision constant (approx 2.22044e-16).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_PURE, .type = FEATURE_VARIABLE
    },

    // 2. System Identification
    {
        .name = "OS$", .category = "System Identification", .syntax = "OS$ or SYS.OS$",
        .description = "Host operating system name (Windows, Linux, macOS, FreeBSD, ESP32, POSIX).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "SYS.OS$", .category = "System Identification", .syntax = "SYS.OS$",
        .description = "Namespace query for host operating system name.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "ARCH$", .category = "System Identification", .syntax = "ARCH$ or SYS.ARCH$",
        .description = "Host CPU architecture family (x86_64, ARM64, x86, ARM, RISC-V).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "SYS.ARCH$", .category = "System Identification", .syntax = "SYS.ARCH$",
        .description = "Namespace query for host CPU architecture family.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "PLATFORM$", .category = "System Identification", .syntax = "PLATFORM$ or SYS.PLATFORM$",
        .description = "Platform execution environment (Win32, POSIX, Embedded, FreeDOS).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "MACHINE$", .category = "System Identification", .syntax = "MACHINE$ or SYS.MACHINE$",
        .description = "Machine profile identifier.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "EDITION$", .category = "System Identification", .syntax = "EDITION$ or SYS.EDITION$",
        .description = "BASIC++ edition name (Standard Edition, Lite Edition, Batch Runner, IoT).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "PROFILE$", .category = "System Identification", .syntax = "PROFILE$ or SYS.PROFILE$",
        .description = "Active runtime profile (Desktop, Headless, IoT).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "COMPILER$", .category = "System Identification", .syntax = "COMPILER$ or SYS.COMPILER$",
        .description = "Host C compiler used to build BASIC++ (MSVC, GCC, Clang).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "WORDSIZE", .category = "System Identification", .syntax = "WORDSIZE or SYS.WORDSIZE",
        .description = "Target machine word size in bits (32 or 64).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "PTRSIZE", .category = "System Identification", .syntax = "PTRSIZE or SYS.PTRSIZE",
        .description = "Pointer size in bytes (4 or 8).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "CORES", .category = "System Identification", .syntax = "CORES or SYS.CORES",
        .description = "Number of logical CPU processing cores available.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "PID", .category = "System Identification", .syntax = "PID or SYS.PID",
        .description = "Active BASIC++ operating system process identifier.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "VERSION$", .category = "System Identification", .syntax = "VERSION$ or SYS.VERSION$",
        .description = "Authoritative BASIC++ semantic release version string (e.g. '6.5.2').",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "HOSTNAME$", .category = "System Identification", .syntax = "HOSTNAME$ or SYS.HOSTNAME$",
        .description = "Host computer network hostname.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "USERNAME$", .category = "System Identification", .syntax = "USERNAME$ or SYS.USERNAME$",
        .description = "Active login user account name.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },

    // 3. Clocks & Timers
    {
        .name = "UPTIME", .category = "Clocks & Timers", .syntax = "UPTIME or UPTIME()",
        .description = "Returns total seconds elapsed since host operating system booted.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "UPTIME$", .category = "Clocks & Timers", .syntax = "UPTIME$ or UPTIME$()",
        .description = "Formatted duration string (e.g. '1d 14h 41m 47s') since host OS booted.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TI", .category = "Clocks & Timers", .syntax = "TI or TI()",
        .description = "Elapsed duration in seconds (subsecond float) since BASIC++ interpreter startup.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TI$", .category = "Clocks & Timers", .syntax = "TI$ or TI$()",
        .description = "Elapsed session duration string ('HHMMSS' or 'D days, HH:MM:SS') since interpreter startup.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TIMER", .category = "Clocks & Timers", .syntax = "TIMER or TIMER()",
        .description = "Seconds elapsed since midnight (0..86399.9999).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TIMER$", .category = "Clocks & Timers", .syntax = "TIMER$ or TIMER$()",
        .description = "Formatted session duration string ('HH:MM:SS') since interpreter startup.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "JIFFIES", .category = "Clocks & Timers", .syntax = "JIFFIES or JIFFIES()",
        .description = "50 Hz PAL/SECAM timer ticks elapsed since system boot.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TICKS", .category = "Clocks & Timers", .syntax = "TICKS or TICKS()",
        .description = "60 Hz NTSC timer ticks elapsed since system boot.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TICKS_MS", .category = "Clocks & Timers", .syntax = "TICKS_MS or TICKS_MS()",
        .description = "Elapsed milliseconds since system boot.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TICKS_US", .category = "Clocks & Timers", .syntax = "TICKS_US or TICKS_US()",
        .description = "Elapsed microseconds since system boot.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "CLK", .category = "Clocks & Timers", .syntax = "CLK or CLK()",
        .description = "24-hour UTC numeric time (HHMMSS format).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "CLK$", .category = "Clocks & Timers", .syntax = "CLK$ or CLK$()",
        .description = "12-hour UTC formatted time string with AM/PM indicator ('HH:MM:SS AM/PM').",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TIME", .category = "Clocks & Timers", .syntax = "TIME or TIME()",
        .description = "24-hour Local numeric time (HHMMSS format).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TIME$", .category = "Clocks & Timers", .syntax = "TIME$ or TIME$()",
        .description = "12-hour Local formatted time string with AM/PM indicator ('HH:MM:SS AM/PM').",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "CLOCK", .category = "Clocks & Timers", .syntax = "CLOCK or CLOCK()",
        .description = "Local unpunctuated numeric timestamp (YYYYMMDDHHMMSS format).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "CLOCK$", .category = "Clocks & Timers", .syntax = "CLOCK$ or CLOCK$()",
        .description = "Local date and time formatted in ISO 8601 string ('YYYY-MM-DDTHH:MM:SS').",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "UTC", .category = "Clocks & Timers", .syntax = "UTC or UTC()",
        .description = "UTC unpunctuated numeric timestamp (YYYYMMDDHHMMSS format).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "UTC$", .category = "Clocks & Timers", .syntax = "UTC$ or UTC$()",
        .description = "UTC date and time formatted in ISO 8601 string ('YYYY-MM-DDTHH:MM:SSZ').",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "EPOCH", .category = "Clocks & Timers", .syntax = "EPOCH or EPOCH()",
        .description = "Seconds elapsed since 01/01/1980 00:00:00 UTC (MS-DOS epoch).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "EPOCH.MS", .category = "Clocks & Timers", .syntax = "EPOCH.MS",
        .description = "Milliseconds elapsed since 01/01/1980 00:00:00 UTC (MS-DOS epoch).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "UNIXTIME", .category = "Clocks & Timers", .syntax = "UNIXTIME or UNIXTIME()",
        .description = "Seconds elapsed since 01/01/1970 00:00:00 UTC (Standard Unix epoch).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "STARDATE", .category = "Clocks & Timers", .syntax = "STARDATE or STARDATE()",
        .description = "Days elapsed since 01/01/1978 (CP/M epoch) with tenths position as 1/10th of a 24-hr day.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "STARDATE$", .category = "Clocks & Timers", .syntax = "STARDATE$ or STARDATE$()",
        .description = "Formatted days elapsed since 01/01/1974 (CP/M adjusted epoch) with 1/10th day decimal.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "JULIAN", .category = "Clocks & Timers", .syntax = "JULIAN or JULIAN()",
        .description = "Astronomical Julian Day Number.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "JULIAN$", .category = "Clocks & Timers", .syntax = "JULIAN$ or JULIAN$()",
        .description = "Astronomical Julian Day Number formatted string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "JD", .category = "Clocks & Timers", .syntax = "JD or JD()",
        .description = "Synonym for Julian Day Number.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "MJD", .category = "Clocks & Timers", .syntax = "MJD or MJD()",
        .description = "Modified Julian Day (JD - 2400000.5).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "GMST", .category = "Clocks & Timers", .syntax = "GMST or GMST()",
        .description = "Greenwich Mean Sidereal Time in decimal hours.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },

    // 4. Environment
    {
        .name = "COMSPEC", .category = "Environment", .syntax = "COMSPEC or COMSPEC$",
        .description = "Returns the absolute path to the active system command shell interpreter.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "COMSPEC$", .category = "Environment", .syntax = "COMSPEC$ or COMSPEC$()",
        .description = "Returns the absolute path string to the active system command shell interpreter.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "PATH", .category = "Environment", .syntax = "PATH or PATH$",
        .description = "Returns the system executable search PATH environment variable.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "PATH$", .category = "Environment", .syntax = "PATH$ or PATH$()",
        .description = "Returns the system executable search PATH environment string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "LOGNAME", .category = "Environment", .syntax = "LOGNAME or LOGNAME$",
        .description = "Returns the active login user account name.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "LOGNAME$", .category = "Environment", .syntax = "LOGNAME$ or LOGNAME$()",
        .description = "Returns the active login user account name string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "HOMEPATH", .category = "Environment", .syntax = "HOMEPATH or HOMEPATH$",
        .description = "Returns the user home directory path.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "HOMEPATH$", .category = "Environment", .syntax = "HOMEPATH$ or HOMEPATH$()",
        .description = "Returns the user home directory path string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "HOMEDRIVE", .category = "Environment", .syntax = "HOMEDRIVE or HOMEDRIVE$",
        .description = "Returns the host drive letter for the user home directory.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "HOMEDRIVE$", .category = "Environment", .syntax = "HOMEDRIVE$ or HOMEDRIVE$()",
        .description = "Returns the host drive letter string for the user home directory.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "USERPATH", .category = "Environment", .syntax = "USERPATH or USERPATH$",
        .description = "Returns the full absolute user profile directory path.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "USERPATH$", .category = "Environment", .syntax = "USERPATH$ or USERPATH$()",
        .description = "Returns the full absolute user profile directory path string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "COMPUTERNAME", .category = "Environment", .syntax = "COMPUTERNAME or COMPUTERNAME$",
        .description = "Returns the host machine / network computer name.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "COMPUTERNAME$", .category = "Environment", .syntax = "COMPUTERNAME$ or COMPUTERNAME$()",
        .description = "Returns the host machine / network computer name string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "WORKDIR$", .category = "Environment", .syntax = "WORKDIR$ or CURDIR$",
        .description = "Current working directory path.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "BASEDIR$", .category = "Environment", .syntax = "BASEDIR$",
        .description = "Base repository / application installation directory path.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "HOME$", .category = "Environment", .syntax = "HOME$",
        .description = "User home directory path string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TEMP$", .category = "Environment", .syntax = "TEMP$",
        .description = "System temporary storage folder path.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "LANG$", .category = "Environment", .syntax = "LANG$",
        .description = "Host environment locale and language encoding string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "SHELL$", .category = "Environment", .syntax = "SHELL$",
        .description = "Active interactive shell executable name (cmd.exe or /bin/sh).",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TERM$", .category = "Environment", .syntax = "TERM$",
        .description = "Terminal emulator type string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "PROMPT$", .category = "Environment", .syntax = "PROMPT$",
        .description = "Active interactive REPL prompt string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },

    // 5. Diagnostics
    {
        .name = "TOTALMEM", .category = "Diagnostics", .syntax = "TOTALMEM or SYS.MEMORY.TOTAL",
        .description = "Returns total physical RAM installed in the host operating system in bytes.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "AVAILMEM", .category = "Diagnostics", .syntax = "AVAILMEM or SYS.MEMORY.AVAIL",
        .description = "Returns available physical RAM currently free across the host system in bytes.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "SYS.MEMORY.TOTAL", .category = "Diagnostics", .syntax = "SYS.MEMORY.TOTAL",
        .description = "Namespace query for total physical RAM installed in the host system in bytes.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "SYS.MEMORY.AVAIL", .category = "Diagnostics", .syntax = "SYS.MEMORY.AVAIL",
        .description = "Namespace query for available physical RAM currently free across the host system in bytes.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "MEM", .category = "Diagnostics", .syntax = "MEM or FRE(0)",
        .description = "BASIC++ managed memory pool allocation limit in bytes (640 MB on Desktop).",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "KERNEL$", .category = "Diagnostics", .syntax = "KERNEL$ or SYS.KERNEL$",
        .description = "Returns host operating system kernel and build version string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "DOSVER$", .category = "Diagnostics", .syntax = "DOSVER$ or DOSVER",
        .description = "Returns DOS subsystem compatibility version string or number.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "WINVER$", .category = "Diagnostics", .syntax = "WINVER$ or WINVER",
        .description = "Returns Windows version string or number.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "DRIVE$", .category = "Diagnostics", .syntax = "DRIVE$ or SYS.DRIVE$",
        .description = "Returns current working drive letter or root path.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "TEMPDIR$", .category = "Diagnostics", .syntax = "TEMPDIR$ or SYS.TEMPDIR$",
        .description = "Returns host temporary files directory path.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "ISADMIN", .category = "Diagnostics", .syntax = "ISADMIN or SYS.ISADMIN or ISROOT",
        .description = "Returns -1 if running with administrator / root privileges, 0 otherwise.",
        .error_summary = "None", .subsystem = SUBSYSTEM_PLATFORM, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "ERR", .category = "Diagnostics", .syntax = "ERR or ERR()",
        .description = "Returns the numeric error code of the most recent runtime error.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "ERL", .category = "Diagnostics", .syntax = "ERL or ERL()",
        .description = "Returns the program line number where the most recent runtime error occurred.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "ERDEV$", .category = "Diagnostics", .syntax = "ERDEV$ or ERDEV$()",
        .description = "Returns the device name or message associated with the most recent I/O error.",
        .error_summary = "None", .subsystem = SUBSYSTEM_CORE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "FREEFILE", .category = "Diagnostics", .syntax = "FREEFILE or FREEFILE()",
        .description = "Returns the next available unused file channel number (#1..#255).",
        .error_summary = "None", .subsystem = SUBSYSTEM_SCRIPT, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "MEMMAP$", .category = "Diagnostics", .syntax = "MEMMAP$ or MEMMAP$()",
        .description = "Returns formatted memory allocation map across heaps and virtual segments.",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "COMMAND$", .category = "Diagnostics", .syntax = "COMMAND$ or COMMAND$(n)",
        .description = "Returns command-line arguments passed to the BASIC++ program.",
        .error_summary = "None", .subsystem = SUBSYSTEM_SCRIPT, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },

    // 6. Hardware & Peripherals
    {
        .name = "STICK", .category = "Hardware & Peripherals", .syntax = "STICK or STICK(n)",
        .description = "Reads joystick axis position (0..3).",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "STRIG", .category = "Hardware & Peripherals", .syntax = "STRIG or STRIG(n)",
        .description = "Reads joystick fire button trigger status (0..3).",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "PADDLE", .category = "Hardware & Peripherals", .syntax = "PADDLE or PADDLE(n)",
        .description = "Reads rotary paddle controller position (0..3).",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "PTRIG", .category = "Hardware & Peripherals", .syntax = "PTRIG or PTRIG(n)",
        .description = "Reads paddle controller button trigger status (0..3).",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "BAUD", .category = "Hardware & Peripherals", .syntax = "BAUD or BAUD(channel)",
        .description = "Queries serial communication channel baud rate.",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "SPEED&", .category = "Hardware & Peripherals", .syntax = "SPEED&",
        .description = "Console transmission speed in bits per second.",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "SPEED%", .category = "Hardware & Peripherals", .syntax = "SPEED%",
        .description = "Apple II console output delay scaling rate (0..255).",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "ATTR", .category = "Hardware & Peripherals", .syntax = "ATTR or ATTR(row, col)",
        .description = "Character cell color attribute byte at screen position.",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "STATUS", .category = "Hardware & Peripherals", .syntax = "STATUS or ST",
        .description = "Commodore 64 I/O status byte.",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "DS", .category = "Hardware & Peripherals", .syntax = "DS",
        .description = "Commodore DOS disk drive error code.",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "DS$", .category = "Hardware & Peripherals", .syntax = "DS$",
        .description = "Commodore DOS disk drive status string ('00, OK, 00, 00').",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "PAGE", .category = "Hardware & Peripherals", .syntax = "PAGE",
        .description = "Apple II active video display page (1 or 2).",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "VBL", .category = "Hardware & Peripherals", .syntax = "VBL",
        .description = "Vertical blanking interval flag (0.0).",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "FRAMES", .category = "Hardware & Peripherals", .syntax = "FRAMES",
        .description = "Sinclair ZX Spectrum 50 Hz frame counter.",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "VCOUNT", .category = "Hardware & Peripherals", .syntax = "VCOUNT",
        .description = "Atari vertical scanline raster counter.",
        .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },

    // 7. Console & Screen
    {
        .name = "CSRLIN", .category = "Console & Screen", .syntax = "CSRLIN or CSRLIN()",
        .description = "Current text cursor screen row (1-indexed).",
        .error_summary = "None", .subsystem = SUBSYSTEM_KERNEL, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "POS", .category = "Console & Screen", .syntax = "POS or POS(0)",
        .description = "Current text cursor screen column (1-indexed).",
        .error_summary = "None", .subsystem = SUBSYSTEM_KERNEL, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "LPOS", .category = "Console & Screen", .syntax = "LPOS or LPOS(0)",
        .description = "Current line printer column position.",
        .error_summary = "None", .subsystem = SUBSYSTEM_KERNEL, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "INKEY$", .category = "Console & Screen", .syntax = "INKEY$ or INKEY$()",
        .description = "Non-blocking keyboard character reader.",
        .error_summary = "None", .subsystem = SUBSYSTEM_KERNEL, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },

    // 8. IoT & Networking
    {
        .name = "IP$", .category = "IoT & Networking", .syntax = "IP$ or IP$()",
        .description = "Local network IPv4 address string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "WIFI.SSID$", .category = "IoT & Networking", .syntax = "WIFI.SSID$",
        .description = "Connected Wi-Fi access point SSID string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "WIFI.RSSI", .category = "IoT & Networking", .syntax = "WIFI.RSSI",
        .description = "Wi-Fi received signal strength indicator in dBm.",
        .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "WIFI.STATUS$", .category = "IoT & Networking", .syntax = "WIFI.STATUS$",
        .description = "Wi-Fi connection status string ('CONNECTED', 'DISCONNECTED').",
        .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "NET.GATEWAY$", .category = "IoT & Networking", .syntax = "NET.GATEWAY$",
        .description = "Local default network gateway IP address string.",
        .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "_BATTERY%", .category = "IoT & Networking", .syntax = "_BATTERY%",
        .description = "Device battery level percentage (0..100%).",
        .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "_TEMPERATURE%", .category = "IoT & Networking", .syntax = "_TEMPERATURE%",
        .description = "Microcontroller core temperature in Celsius.",
        .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "_CPU_LOAD%", .category = "IoT & Networking", .syntax = "_CPU_LOAD%",
        .description = "Microcontroller CPU load percentage (0..100%).",
        .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    },
    {
        .name = "_FREE_STACK", .category = "IoT & Networking", .syntax = "_FREE_STACK",
        .description = "Free stack space remaining in bytes.",
        .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_VARIABLE
    }
};

void system_var_descriptors_register(void) {
    size_t count = sizeof(g_system_vars) / sizeof(g_system_vars[0]);
    for (size_t i = 0; i < count; i++) {
        lang_desc_register(&g_system_vars[i]);
    }
    builtin_descriptors_register();
}
