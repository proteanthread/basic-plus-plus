# BASIC++ v6.5.2 MBASIC / CP/M BASIC Dialect

## 1. HISTORY

MBASIC (Microsoft BASIC for CP/M) was the dominant BASIC implementation on CP/M systems in the late 1970s and early 1980s. Written by Microsoft, MBASIC was the direct ancestor of BASICA and GW-BASIC for the IBM PC. Programs written for MBASIC are largely compatible with GW-BASIC, but there are differences in file I/O, memory management, and available functions.

## 2. MBASIC COMPATIBILITY IN BASIC++

BASIC++ runs MBASIC programs through its GW-BASIC compatible dialect (GWBS, the default). Since GW-BASIC is a direct descendant of MBASIC, most MBASIC programs run without modification. The following sections document differences and adaptations.

## 3. FILE I/O DIFFERENCES

MBASIC file I/O differs from GW-BASIC in syntax:

MBASIC: `OPEN "I", 1, "DATA.TXT"` (mode letter, no # sign, no AS keyword)
GW-BASIC: `OPEN "DATA.TXT" FOR INPUT AS #1`

BASIC++ accepts both forms. The MBASIC short form maps mode letters: "I" = INPUT, "O" = OUTPUT, "R" = RANDOM, "A" = APPEND.

MBASIC: `PRINT #1, Data$` (no AS keyword needed)
GW-BASIC: `PRINT #1, Data$` (same syntax)

Both styles work identically in BASIC++.

## 4. MEMORY FUNCTIONS

MBASIC used FRE("") to force garbage collection and FRE(0) to report free memory. BASIC++ supports both. USR(n) for machine-language calls works the same way through the host callback interface.

PEEK and POKE in MBASIC accessed the CP/M address space (Z80 memory map at 0x0000-0xFFFF). In BASIC++, PEEK and POKE access the BIOS emulation space. CP/M-specific addresses (BDOS entry point at 0x0005, TPA start at 0x0100) are not meaningful in the BASIC++ context.

## 5. LINE LENGTH

MBASIC supported line lengths up to 255 characters. BASIC++ has no practical line length limit on modern builds.

## 6. VARIABLE NAME LENGTH

MBASIC recognized only the first two characters of variable names: COUNTER and CO were the same variable. BASIC++ uses the full variable name. Programs that relied on two-character truncation may need review.

## 7. PRINT ZONES

MBASIC used 14-column print zones (same as GW-BASIC). BASIC++ maintains the same 14-column zone width for compatibility.

## 8. STRING FUNCTIONS

MBASIC supported the same core string functions as GW-BASIC: LEFT$, RIGHT$, MID$, LEN, STR$, VAL, CHR$, ASC, INSTR, STRING$, SPACE$. All are available in BASIC++.

MBASIC did not have UCASE$, LCASE$, TRIM$, LTRIM$, or RTRIM$ — these are GW-BASIC and BASIC++ additions.

## 9. ERROR HANDLING

MBASIC supported ON ERROR GOTO and RESUME. ERR returned the error code and ERL returned the error line number. These work identically in BASIC++. The error code values are compatible.

## 10. GRAPHICS AND SOUND

MBASIC on CP/M had no built-in graphics or sound statements (CP/M did not define a standard graphics interface). Programs that used custom POKE sequences for terminal-specific graphics will need adaptation to BASIC++ SCREEN modes.

## 11. MIGRATION TIPS

1. Replace MBASIC short-form OPEN with the GW-BASIC long form for clarity.
2. Check variable names — if two variables differ only after the second character, rename them.
3. Replace any CP/M-specific PEEK/POKE addresses with BASIC++ VDev calls.
4. Add SCREEN mode selection for any graphics output.
5. CP/M file handling: MBASIC filenames were 8.3 format (e.g., "MYFILE.BAS"). BASIC++ supports long filenames on modern systems.
