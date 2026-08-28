// FILENAME: version.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, libcore, libengine, libkernel, libstandard
// NEEDS: platform, memory
// Provides core logic and interface definitions for version within BASIC++.
//
// ---- Includes ----

#ifndef TYPES_VERSION_H
#define TYPES_VERSION_H

#define BASIC_VERSION_MAJOR    6
#define BASIC_VERSION_MINOR    5
#define BASIC_VERSION_PATCH    2
#define BASIC_VERSION_STRING   "6.5.2"
#define BASIC_VERSION_DATE     "2026-08-06"
#define BASIC_VERSION_CODENAME "Phoenix"

// Clean non-prefixed aliases per project rules
#define VERSION_MAJOR    BASIC_VERSION_MAJOR
#define VERSION_MINOR    BASIC_VERSION_MINOR
#define VERSION_PATCH    BASIC_VERSION_PATCH
#define VERSION_STRING   BASIC_VERSION_STRING
#define VERSION_DATE     BASIC_VERSION_DATE
#define VERSION_CODENAME BASIC_VERSION_CODENAME

#endif // TYPES_VERSION_H
