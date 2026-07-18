/**
 * @file bpp_version.h
 * @brief Version information for the BASIC++ interpreter.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Defines the version number (Major, Minor, Patch), codename, release date, and build info.
 * - Why it exists: To act as the single source of truth for version numbers across the CMake build system,
 *   REPL banners, documentation, and error reports.
 * - Why it works this way: By centralizing these definition macros in a single header, we avoid version
 *   inconsistencies where config.h and script build files mismatch.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: The major, minor, patch numbers, codename, and release date when releasing new features.
 * - What cannot be changed: The macro names themselves (BPP_VERSION_MAJOR, etc.) as the interpreter REPL and build files query them.
 * - What to expect: Changes here will propagate to all binary editions and self-test reports during compilation.
 * - What to do if something breaks: Ensure the version definitions compile without syntax errors and that the date format remains a string.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Assumes standard C macro substitution is supported by the compiler (universal in C17).
 * - Portability concerns: None. Fully C17 and older standard compatible.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Additional macros (e.g. build number, git commit hash) can be added cleanly.
 * - How to write external extensions: External plugins can query the version from the VM context to check version compatibility.
 */

#ifndef BPP_VERSION_H
#define BPP_VERSION_H

#define BPP_VERSION_MAJOR    6
#define BPP_VERSION_MINOR    4
#define BPP_VERSION_PATCH    0
#define BPP_VERSION_STRING   "6.4.0"
#define BPP_VERSION_DATE     "2026-07-13"
#define BPP_VERSION_CODENAME "Phoenix"

#endif /* BPP_VERSION_H */
