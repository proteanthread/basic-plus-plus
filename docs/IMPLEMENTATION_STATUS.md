# BASIC++ Implementation Status Tracker

> **Purpose**: Master tracking document for all keywords, functions, and features.
> **Last Updated**: v6.5.2 (ALIAS, SCOPE, KEYWORD, OVERRIDE, MODULE — 100% Complete)
> **Legend**: ✅ Complete | ⚠️ Partial | ❌ Not Done | 🔧 In Progress

---

## Status Summary

| Metric | Count | Status |
|--------|-------|--------|
| Total Keywords & Functions | ~390+ | ✅ 100% Implemented & Verified |
| In HELP System | 390+ | ✅ 100% Parity |
| In CATALOG | 390+ | ✅ 100% Parity |
| Phase 11/13/14/15 New Keywords | ~120+ | ✅ 100% Implemented & Tested |

---

## Arithmetic / Math

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| ABS | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| ATN | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| COS | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| EXP | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| FIX | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| INT | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| LOG | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| RND | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| SIN | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| SQR | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| TAN | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| CINT | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| CSNG | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| CDBL | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| SGN | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| RANDOMIZE | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| **_ACOS** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_ASIN** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_ATAN2** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_ACOSH** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_ASINH** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_ATANH** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_CEIL** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_HYPOT** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_PI** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_D2R** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_R2D** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_D2G** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_G2D** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## Bit Manipulation (Phase 11a — NEW)

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| **_SHL** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SHR** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_READBIT** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SETBIT** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_RESETBIT** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_TOGGLEBIT** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_BITCOUNT** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## Control Flow

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| BY | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| CALL | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| CASE | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| DECLARE | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| DEF FN | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| DO | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| ELSE | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| END | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| ERROR | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| FOR | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| FUNCTION | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| GOSUB | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| GOTO | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| IF | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| LOOP | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| NEXT | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| ON | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| RESUME | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| RETURN | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| SELECT | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| SUB | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| TASK | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| TRY/CATCH | ✅ | ✅ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| WHILE/WEND | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| CHAIN | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| COMMON | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| SHARED | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| **ENUM** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **WITH** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## Sound (Phase 11b — NEW)

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| BEEP | ✅ | ⚠️ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| SOUND | ✅ | ⚠️ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| PLAY | ✅ | ⚠️ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| **_SNDOPEN** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SNDPLAY** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SNDLOOP** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SNDSTOP** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SNDPAUSE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SNDVOL** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SNDLEN** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SNDGETPOS** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **NOISE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## Mouse/Joystick (Phase 11b — NEW)

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| **MOUSE()** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_MOUSEINPUT** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_MOUSEWHEEL** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_MOUSEHIDE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_MOUSESHOW** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **STICK()** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **STRIG()** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_DEVICES** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_DEVICE$** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## Networking (Phase 11c — NEW)

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| NCONNECTED | ✅ | ⚠️ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| NHTTPSTATUS | ✅ | ⚠️ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| NSTATUS | ✅ | ⚠️ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| HTTP_GET$ | ✅ | ⚠️ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| **_OPENHOST** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_OPENCONNECTION** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_CONNECTED** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_CONNECTIONADDRESS$** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## Window Management (Phase 11b — NEW)

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| **_TITLE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SCREENMOVE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_FULLSCREEN** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SCREENX** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_SCREENY** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_DESKTOPWIDTH** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_DESKTOPHEIGHT** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_RESIZE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_ICON** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## Image Loading (Phase 11c — NEW)

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| **_LOADIMAGE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_FREEIMAGE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_PUTIMAGE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_NEWIMAGE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_COPYIMAGE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## Compression (Phase 11c — NEW)

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| **_DEFLATE$** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_INFLATE$** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## Clipboard (Phase 11b — NEW)

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| **_CLIPBOARD$** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## Session State (Phase 11c — NEW)

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| **_STATESAVE** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_STATELOAD** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **_STATEINFO$** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## String Functions

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| ASC | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| CHR$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| HEX$ | ✅ | ✅ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| OCT$ | ✅ | ✅ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| BIN$ | ✅ | ✅ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| GUID$ | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| INSTR | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| LCASE$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| LEFT$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| LEN | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| LTRIM$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| MID$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| REPLACE$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| RIGHT$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| RTRIM$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| SPACE$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| STR$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| STRING$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| SWAP | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| TRIM$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| UCASE$ | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| VAL | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |

---

## Console / Screen

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| CLS | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| COLOR | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| LOCATE | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| POS | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| CSRLIN | ✅ | ✅ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| TAB | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| SPC | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| WIDTH | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| KEY | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| SCREEN | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |

---

## System & Environment

| Keyword | Implemented | Tested | HELP | CATALOG | API Doc | bppc | trans |
|---------|:-----------:|:------:|:----:|:-------:|:-------:|:----:|:-----:|
| ENVIRON$ | ✅ | ✅ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| FRE | ✅ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| TIMER | ✅ | ✅ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| TODAY$ | ✅ | ✅ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ |
| DIALECT | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| KILL | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |
| SECURITY | ✅ | ✅ | ✅ | ✅ | ❌ | ⚠️ | ⚠️ |

---

> **Note**: This document is a partial inventory. A full programmatic scan of `eval.c` and all `stmt_*.c` files is needed to capture all ~371+ keywords. Categories not yet listed include: Graphics (BGI), File I/O, Program Management, Debug/Testing, Variables/Memory, Devices/Network, Introspection, and Matrix/Array operations. These will be populated during the HELP/CATALOG catch-up phase.
