# BASIC++ Master Planned Subsystems & Dialect Gap Analysis Reference (`engine/PLANNED.md`)

> **Authoritative Planned Subsystems Reference**: An exhaustive catalog of missing keywords, statements, functions, compiler passes, virtual hardware interfaces, and dialect specifications across the 60-year history of BASIC (1964–2026), synthesized from `source/`, `v5/`, `project notes/`, git history, and historical language specifications.

---

## 1. Executive Summary & Core Compiler/VM Roadmap

The BASIC++ v6.5.2 engine provides a unified, non-recursive, C17 runtime executing interpreted AST statements and bytecode. To achieve full universal dialect emulation and native compilation, the following core subsystems are actively planned:

### 1.1 Native Whole-Program AOT Compiler (`bppc.exe` / `bppc`)
- **Whole-Program AST Restructuring**: Analyzes global program structure, infers strict variable types, and eliminates runtime symbol lookup overhead.
- **Optional Line Numbers in Multi-Statement Files**: Supports pure modern structured BASIC syntax without line numbers across multi-module projects.
- **Freestanding C17 Code Generation**: Emits standard ISO C17 source code compiling via MSVC, GCC, Clang, or Zig into zero-dependency standalone binaries.
- **Bytecode VM Stub Injection**: Generates lightweight executable stubs embedding pre-compiled Universal IR bytecode.

### 1.2 Cross-Dialect Source-to-Source Transpiler (`trans.exe` / `trans`)
- **Bidirectional Dialect Translation**: Translates between legacy dialects (GW-BASIC, QBASIC, Super BASIC, ECMA-116, Dartmouth) and modern structured BASIC++.
- **Dialect Metaprogramming & Introspection**: Natively validates, translates, and generates language specifications using BASIC++ macros and metadata.

### 1.3 16-Bit Real Mode Segmented Memory Virtualization (`vmem`)
- **`DEF SEG = segment`**: Full 20-bit real-mode address calculation (`Physical = (SEG * 16) + OFFSET`).
- **Memory Trapping & Bounds Validation**: Safe emulation of IBM PC BIOS Data Area (BDA `0x0040:0x0000`), CGA/MDA/EGA VRAM buffers (`0xB800:0x0000`, `0xA000:0x0000`), and conventional memory (`0x0000:0x0000` to `0x9FFF:0x0000`).
- **`CALL ABSOLUTE(args..., addr)`**: Sandboxed machine-code execution via the micro-8086 interpreter (`libcpu8086`).

### 1.4 Multi-Modal Built-In Editors
- **MS-DOS EDLIN (`--edlin` / `mod_edlin.h`)**: Line editor emulation with classic syntax (`A` Append, `D` Delete, `E` End/Save, `I` Insert, `L` List, `M` Move, `P` Page, `R` Replace, `S` Search, `W` Write, `Q` Quit).
- **Bill Joy VI (`--vi` / `mod_vi.h`)**: Modal text editor supporting Normal, Insert, Visual, and Ex Command modes.
- **WordStar 3.3/4.0 (`--ws` / `mod_ws.h`)**: Classic Control-key diamond cursor control (`^E` up, `^X` down, `^S` left, `^D` right, `^A` word-left, `^F` word-right) and block commands (`^KB`, `^KK`, `^KV`, `^KC`, `^KY`).

### 1.5 FujiNet Hardware Bus & IoT SIO Virtualization
- **FujiNet Network Device (`N:`)**: URL-based stream communication (`OPEN "N:HTTP://...", #1`, `OPEN "N:TCP://...", #1`, `OPEN "N:UDP://...", #1`).
- **TNFS Remote Filesystem Mounting**: Native mounting of remote Trivial Network File System archives.
- **Microcontroller Embedded HAL**: Zero-libc freestanding compilation for ESP32, STM32, Raspberry Pi Pico, and bare-metal ARM.

---

## 2. Mainframe & Timesharing Dialects Gap Analysis

```
+-------------------------------------------------------------------------------+
|                    HISTORICAL MAINFRAME & TIMESHARING LINEAGE                 |
+-------------------------------------------------------------------------------+
| 1964: DTSS Dartmouth BASIC (GE-225/635) -> 1965: GE Mark I/II Time-Sharing   |
| 1968: HP 2000 Time-Shared BASIC --------> 1974: HP 3000 MPE / Business BASIC  |
| 1969: DEC PDP-11 RSTS/E BASIC-PLUS ------> 1975: DEC BASIC-PLUS-2 / VAX BASIC |
| 1969: Tymshare Super BASIC (SDS 940) ----> 1970: IBM CALL/360 & VS-BASIC      |
| 1970s: Univac 1100, Burroughs CANDE, CDC Kronos/NOS, Multics, Prime PRIMOS    |
+-------------------------------------------------------------------------------+
```

### 2.1 Dartmouth DTSS BASIC (1st Edition 1964 to 7th Edition 1979 / SBASIC)
*Hardware: GE-225, GE-235, GE-635 Dartmouth Time-Sharing System*
- **Planned Statements**: `DEF FN` multi-line function blocks (`DEF FN... / FNEND`), `CHANGE array TO string$` / `CHANGE string$ TO array`, `LINPUT` (unformatted line input), `MARGIN` (sets teletype line width), `PAGE` (sets printer page length), `TIM` (returns current CPU execution seconds), `TIME` (returns time of day string).
- **Matrix Operations**: `MAT INPUT`, `MAT PRINT`, `MAT READ`, `MAT A = B + C`, `MAT A = B * C`, `MAT A = INV(B)`, `MAT A = TRN(B)`, `MAT A = ZER`, `MAT A = CON`, `MAT A = IDN`.

### 2.2 GE Mark I & Mark II Time-Sharing BASIC
*Hardware: GE-265, GE-400, GE-635 Information Systems Time-Sharing Network*
- **Planned Statements**: `EDIT` (remote teletype program editing), `BUILD` (automatic prompt-driven line number entry), `EXTRACT` (pulls line range into working buffer), `MERGE` (combines saved files into current program), `ALTER` (character-level string replacement in lines).

### 2.3 HP 2000 & HP 3000 Time-Shared BASIC (MPE 3000, Business BASIC, HP 3000/V)
*Hardware: HP 2000A/B/C/E/F, HP 3000 Series II/III/30/33/44/68/9000, HP 9845*
- **Planned Substring Slicing Syntax**: `A$(start, end)` and `A$(start)` string slicing syntax without `MID$` / `LEFT$` / `RIGHT$`.
- **File System Architecture**: `ASSIGN #channel TO filename$ [, error_return]`, `FILES file1, file2, ...` (multi-file declaration), `PRINT #channel, record; expr_list`, `READ #channel, record; var_list`, `ADVANCE #channel, records`, `RESTORE #channel`.
- **System Commands**: `CATALOG`, `NAME`, `PURGE`, `PROTECT`, `UNPROTECT`, `LOCK`, `UNLOCK`, `STORE`, `SCRATCH`.

### 2.4 DEC Timesharing Lineage (TSS-8, BASIC-10, BASIC-PLUS, BASIC-PLUS-2, VAX BASIC)
*Hardware: PDP-8 (TSS-8), PDP-10 (TOPS-10/20), PDP-11 (RSTS/E, RSX-11M), VAX-11 / Alpha (OpenVMS)*
- **Statement Modifiers (Postfix)**: `statement IF condition`, `statement UNLESS condition`, `statement FOR var = start TO end [STEP step]`, `statement WHILE condition`, `statement UNTIL condition`.
- **Virtual Array Subsystem**: `DIM #channel, array_name(dim1, dim2)` mapping multi-megabyte arrays directly to disk files.
- **Record Mapping & File Locking**: `MAP (map_name) field_list`, `FIELD #channel, width AS var$`, `GET #channel, RECORD rec_num, COUNT bytes`, `PUT #channel, RECORD rec_num, COUNT bytes`, `FIND #channel, KEY rel_op key_val`, `RESTORE #channel, KEY key_val`.
- **String Functions**: `CVT$%(str$)`, `CVT%$ (int)`, `CVT$F(str$)`, `CVTF$(float)`, `XLATE$(str$, table$)`, `COMP% (str1$, str2$)`, `RAD$(int)` (Radix-50 conversion), `NUM$(n)`, `NUM1$(n)`, `VAL%(str$)`.

### 2.5 Tymshare Super BASIC
*Hardware: Scientific Data Systems SDS 940, Xerox Data Systems XDS 940, PDP-10*
- **Planned Capabilities**: Dynamic expression command line execution, `INSPECT variable` (interactive symbol inspection), `MODIFY line` (in-place teletype line editor), `ON expr GOTO line1, line2, ... ELSE line_def`, multiple statements on one line using colon or semicolon delimiters.

### 2.6 IBM Mainframe Lineage (CALL/360, VS-BASIC, ITF, 5100, Series/1, S/34/36/38, AS/400)
*Hardware: IBM System/360, System/370 (OS/MVT, TSS/360, TSO, VM/370 CMS), IBM 5100/5110 Portable, IBM Series/1 (EDX), IBM AS/400 (OS/400)*
- **CALL/360 & VS-BASIC**: `LET` required / optional toggle, `DEF FN` with local formal arguments, formatted `PRINT USING` with EBCDIC support, `RESET` file pointer, `BACKSPACE #channel`, `REWIND #channel`, `PAUSE [prompt$]`.
- **IBM Series/1 EDX & System/34/36**: Multi-tasking task attach (`ATTACH task_name`), event semaphores (`POST event`, `WAIT event`), indexed file access by key length.

### 2.7 Univac 1100 Series CTS / Exec 8 Time-Sharing BASIC
*Hardware: Sperry Univac 1106, 1108, 1110 (Exec 8 / CTS Conversational Time-Sharing)*
- **Planned Features**: 36-bit word length integer arithmetic, `FILE #channel: "NAME"`, formatted output with floating dollar sign and comma spacing, multi-file simultaneous record locking.

### 2.8 Burroughs B5500 / B6700 CANDE Time-Sharing BASIC
*Hardware: Burroughs B5500, B6700, B7700 (MCP / CANDE Command And Edit)*
- **Planned Features**: Tagged word architecture support, dynamic array bounds sizing via `INTEGER ARRAY` / `REAL ARRAY`, stream string searching with `SCAN` / `WHILE`.

### 2.9 CDC 6000 / Cyber Kronos & NOS Time-Sharing BASIC
*Hardware: Control Data Corporation CDC 6400, 6500, 6600, Cyber 70/170 (Kronos, NOS/BE)*
- **Planned Features**: 60-bit floating-point word math (14 decimal digits precision), Display Code 6-bit character translation tables, `CALL "SUBROUTINE" [args]`.

### 2.10 Honeywell Multics BASIC
*Hardware: Honeywell 6000, 6180, DPS-8/M (Multics Operating System)*
- **Planned Features**: Segmented virtual memory addresses, structured block `IF...THEN...ELSE...END IF`, dynamic external procedure binding via Multics search paths.

### 2.11 Data General Extended / TSS BASIC (Nova / Eclipse RDOS, AOS, TSS, Business BASIC)
*Hardware: Data General Nova 800/1200, Eclipse S/130, S/250, MV/8000 (RDOS, AOS/VS)*
- **Planned Features**: `INPUT (timeout, max_chars) var$`, `OPEN (channel, "filename", mode)`, sequential and random indexed record I/O with error branch targets (`READ (channel, record, error_target)`).

### 2.12 Prime Computer Prime BASIC / BASIC/VM (PRIMOS)
*Hardware: Prime 300, 400, 500, 750 (PRIMOS Operating System)*
- **Planned Features**: Virtual Memory pointer binding, segmented string heap allocation up to 32 MB, `SELECT CASE` with string ranges.

### 2.13 ICL 1900 / 2900 Maximop & George 3 BASIC
*Hardware: International Computers Limited ICL 1900, 2900 Series (George 3, VME/B)*
- **Planned Features**: 24-bit/48-bit arithmetic models, `DATA` file stream redirection, interactive line trace (`TRACE ON / OFF`).

### 2.14 Wang 3300 Timesharing BASIC & Harris Vulcan / VOS BASIC
*Hardware: Wang 3300, 2200 Series, Harris Slash 4, Slash 7, Harris 800 (Vulcan, VOS)*
- **Planned Features**: Dedicated CRT character formatting (`HEXOF$`, `VER$`), disk sector `READ #channel, sector, offset; vars...`, `VERIFY`.

### 2.15 PICK System / MultiValue Pick BASIC
*Hardware: Microdata Reality, PICK OS, Prime Information, UniVerse, UniData*
- **Planned MultiValue String Features**: Dynamic arrays with 3-level delimiters: Item Marks (`CHR$(255)`), Field Marks (`CHR$(254)` / `AM`), Value Marks (`CHR$(253)` / `VM`), and Subvalue Marks (`CHR$(252)` / `SVM`).
- **Statements & Functions**: `EXTRACT(str, f, v, sv)`, `INSERT(str, f, v, sv, val)`, `REPLACE(str, f, v, sv, val)`, `DELETE(str, f, v, sv)`, `LOCATE(val, arr, f; pos; order)`.

### 2.16 MAI Basic Four / Thoroughbred / ProvideX Business BASIC
*Hardware: MAI 1200, Basic Four 200/400/600/700/8000, Thoroughbred OS*
- **Planned Features**: Precision rounding mode (`ROUND` statement), variable file record lengths, `KEYED` and `DIRECT` file open modes, `EXTRACT` (record read with auto-lock), `FIND` (record read with lock exception).

---

## 3. International Standards Gap Analysis

### 3.1 ANSI X3.113-1987 & ECMA-116 Full BASIC
*Standard: ISO 8651 / ANSI X3.113-1987 / ECMA-116 Standard for BASIC (1986)*
- **Core Standard Syntax**:
  - `PROGRAM prog_name ... END PROGRAM`
  - Multi-line function blocks: `DEF fn_name(args) ... END DEF`
  - Subprograms: `SUB sub_name(args) ... END SUB` with `CALL sub_name`
  - Structured decision blocks: `SELECT CASE ... CASE ... CASE ELSE ... END SELECT`
  - Exception Handling: `WHEN EXCEPTION USE handler_name ... USE ... END WHEN` with `RETRY` and `CONTINUE`.
- **Module 1: Graphics Extension Module**:
  - World coordinate window transformations: `SET WINDOW xmin, xmax, ymin, ymax`
  - Viewport mapping: `SET VIEWPORT xmin, xmax, ymin, ymax`
  - Primitives: `PLOT POINTS: x, y`, `PLOT LINES: x1, y1; x2, y2`, `PLOT AREA: x1, y1; x2, y2; x3, y3`
  - Drawing Attributes: `SET POINT STYLE n`, `SET LINE STYLE n`, `SET LINE COLOR n`, `SET AREA COLOR n`, `SET AREA STYLE n`.
- **Module 2: Real Time Extension Module**:
  - Event Trapping: `WHEN EVENT event_spec USE handler`
  - Millisecond Timer Waits: `WAIT DURATION seconds`, `WAIT TIME time_val`.
- **Module 3: Fixed Decimal Precision Module**:
  - BCD / Fixed-point decimal arithmetic for financial precision without IEEE float roundoff errors (`OPTION ARITHMETIC FIXED`).
- **Module 4: Editing Extension Module**:
  - Program line buffer manipulation commands.
- **Module 5: Individual Character Input Module**:
  - Single key reading without echo: `GET KEY var$`, `INPUT TIMEOUT(sec) var$`.

### 3.2 ANSI X3.60-1978 & ECMA-55 Minimal BASIC
- Baseline language requirements (single-letter variables, 1-2 character numeric identifiers, basic control flow). Fully conformant in BASIC++ v6.5.2.

---

## 4. Tandy / Radio Shack TRS-80 Family Lineage

```
+-------------------------------------------------------------------------------+
|                       TANDY / TRS-80 FAMILY DIALECT EVOLUTION                 |
+-------------------------------------------------------------------------------+
| 1977: Level I BASIC (4K ROM, Li-Chen Wang) ----------------------------------+
| 1978: Level II BASIC (16K ROM, Microsoft) -> Model III -> Model 4 (TRSDOS 6)  |
| 1980: Color BASIC (CoCo 1/2) -> Extended Color BASIC -> CoCo 3 (SECB / DECB 2)|
| 1983: Model 100 / 102 / 200 Portable CMOS ROM BASIC                           |
+-------------------------------------------------------------------------------+
```

### 4.1 TRS-80 Models I / III / 4 & Business Models (II, 12, 16, 16B, 6000)
- **Level I BASIC (1977)**: `PRINT AT(pos), expr`, `SET(x, y)`, `RESET(x, y)`, `POINT(x, y)`, `MEM`, abbreviated single-letter keywords (`P.` for `PRINT`, `G.` for `GOTO`).
- **Level II / Model III BASIC (1978)**: `CMD "D"` (disk status), `CMD "R"` (run DOS command), `DEFUSR[n] = addr`, `VARPTR`, `PEEK`, `POKE`, `INP`, `OUT`, `TRON`, `TROFF`.
- **Model 4 / TRSDOS 6.x & LS-DOS BASIC**: 80x24 text screen modes, `SOUND freq, dur`, `TIME$`, `DATE$`, `MEM`, `SYSTEM "DOS_CMD"`.
- **Third-Party DOS Extensions (NEWDOS/80, LDOS, DosPLUS, MULTIDOS)**:
  - `CMD "DIR"`, `CMD "KILL"`, `CMD "PROT"`, `CMD "SEEK"`, `CMD "COPY"`, `CMD "LOAD"`.

### 4.2 Tandy Color Computer (CoCo 1, 2, 3) & MC-10
- **Color BASIC & Extended Color BASIC (1.x–2.0)**:
  - Graphics: `PCLS [c]`, `SCREEN type, colorset`, `COLOR fg, bg`, `PSET(x, y, c)`, `PRESET(x, y)`, `LINE(x1,y1)-(x2,y2), PSET|PRESET|BF`, `CIRCLE(x,y), r, c, aspect, start, end`, `PAINT(x,y), c, border`, `GET(x1,y1)-(x2,y2), array, G`, `PUT(x1,y1), array, action`.
  - Sound: `SOUND freq, dur`, `PLAY music_string`, `AUDIO ON | OFF`, `MOTOR ON | OFF`.
  - Joystick: `JOYSTK(n)`.
- **Super Extended Color BASIC (SECB - CoCo 3)**:
  - Enhanced Graphics: `HSCREEN mode (1..4)`, `HCOLOR fg, bg`, `HCLS [c]`, `HLINE`, `HCIRCLE`, `HPAINT`, `HGET`, `HPUT`, `HPRINT`.
  - Hardware Control: `RGB`, `CMP`, `PALETTE slot, color`, `WIDTH 32|40|80`, `LOCATE col, row`, `ATTR fg, bg, blink, under`.

### 4.3 Tandy Model 100 / 102 / 200 Portable Handhelds
- **CMOS Portable Functions**: `TIME$`, `DATE$`, `DAY$`, `POWER [seconds]`, `POWER CONT`, `COM ON|OFF|STOP`, `KEY(n) ON|OFF|STOP`, `MOTOR ON|OFF`, `IPL "filename"`, `SCREEN 0, 0` (40x8 LCD display mode).

---

## 5. Sharp Pocket Computer Lineage

*Models: PC-1211 (PC-1), PC-1500 (PC-2), PC-1251 (PC-3), PC-1246 (PC-4), PC-1270 (PC-7), PC-1260 (PC-8)*
- **Angle Modes**: `DEGREE`, `RADIAN`, `GRAD` — controls argument units for trigonometric functions.
- **Trigonometric & Math**: `SIN`, `COS`, `TAN`, `ASN` (`ASIN`), `ACS` (`ACOS`), `ATN`, `LN`, `EXP`, `LOG`, `SQR`, `ABS`, `SGN`, `INT`, `PI`.
- **Display & Audio**: `BEEP pulses [, duration [, freq]]`, `PRINT #channel; expr`, `USING format_string`, `CURSOR pos`, `WAIT duration`, `PAUSE [expr]`.
- **Matrix & Memory Mapping**: `MEM$`, `STATUS`, `ARUN` (auto-run program on power-up).

---

## 6. Microsoft DOS & CP/M Compilers

### 6.1 IBM PC BASICA & GW-BASIC (Standard & Tandy 1000)
- **Tandy 1000 Extensions**: `SCREEN 11` (320x200 16-color TGA), `SCREEN 12` (640x200 16-color TGA), `SOUND voice, freq, dur, vol` (3-channel sound), `NOISE type, vol, dur`.
- **Event Trapping**: `ON TIMER(sec) GOSUB`, `ON KEY(n) GOSUB`, `ON COM(n) GOSUB`, `ON STRIG(n) GOSUB`, `ON PLAY(n) GOSUB`.

### 6.2 Microsoft QuickBASIC 1.0–4.5, PDS 7.0–7.1 & Visual Basic for DOS 1.0
- **Advanced Control Structures**: `DO WHILE|UNTIL ... LOOP WHILE|UNTIL`, `EXIT DO|FOR|SUB|FUNCTION|DEF`, `SELECT CASE expr ... CASE IS ... CASE a TO b ... END SELECT`.
- **Procedures & Scopes**: `SUB sub_name (args) STATIC ... END SUB`, `FUNCTION fn_name (args) ... END FUNCTION`, `STATIC`, `SHARED`, `COMMON SHARED`.
- **UDT Records & Dynamic Arrays**: `TYPE udt_name ... END TYPE`, `REDIM [PRESERVE] arr(dims)`.
- **Hardware Trapping**: `INP(port)`, `OUT port, byte`, `WAIT port, AND_mask [, XOR_mask]`.

### 6.3 Microsoft BASCOM (1.0–5.3) & MBASIC / BASIC-80 (CP/M)
- Compilation directives: `$INCLUDE: 'filename'`, `$DYNAMIC`, `$STATIC`, `$LINES`, `$DEBUG`.

---

## 7. Third-Party Compilers & Home Microcomputer Dialects

### 7.1 Compiler Systems / Digital Research CBASIC & CB-80
- **Commercial Compiler Model**: Structured multi-line functions (`DEF FNname(args) ... FEND`), 14-digit BCD decimal arithmetic, `EXTERNAL` functions, chained overlays with `CHAIN`.

### 7.2 Borland Turbo BASIC & PowerBASIC (Bob Zale)
- **Assembly Integration**: Inline 8086 machine code via `INLINE &H90, &HCC...` or `ASM ... END ASM`.
- **Bitwise Extensions**: `BIT(n, b)`, `SETBIT n, b`, `RESETBIT n, b`, `TOGGLEBIT n, b`, `SHL(n, b)`, `SHR(n, b)`, `ROL(n, b)`, `ROR(n, b)`.
- **Pointer Variables**: Pointer types `PTR` and dereferencing `@ptr`.

### 7.3 True BASIC (John Kemeny & Thomas Kurtz)
- Strict ANSI Full BASIC compliance: Modules, `DO WHILE ... LOOP`, `EXTYPE`, `MAT` matrix transformations, vector graphics.

### 7.4 GFA BASIC (Atari ST, Amiga, DOS)
- High-performance GUI and graphics language: `RC_COPY`, `BITBLT`, `PUT`, `GET`, `WIND_OPEN`, `MENU`, event handling via `ON MENU GOSUB`, structured loops `WHILE ... WEND`, `REPEAT ... UNTIL`, `FOR ... NEXT`.

### 7.5 Acorn BBC BASIC (BBC Micro, Master 128, Archimedes, RISC OS)
- **Inline ARM/6502 Assembler**: `[ OPT opt: MOV R0, #1: ... ]`
- **Memory Indirection Operators**: `?addr` (byte indirection), `!addr` (32-bit word indirection), `$addr` (null-terminated string indirection), `|addr` (floating-point indirection).
- **Graphics & Sound**: `VDU char, byte...`, `MODE n`, `PLOT mode, x, y`, `ENVELOPE`, `SOUND channel, vol, pitch, dur`.

### 7.6 Sinclair ZX81 & ZX Spectrum BASIC
- Keyword token entry, `PLOT x, y`, `UNPLOT x, y`, `DRAW x, y [, angle]`, `CIRCLE x, y, r`, `INK n`, `PAPER n`, `FLASH n`, `BRIGHT n`, `INVERSE n`, `BORDER n`, `BEEP dur, pitch`.

### 7.7 Commodore BASIC (V1, V2, V3.5, V4.0, V7.0, V10)
- **C64 / PET (V2/V4)**: Single-letter and two-letter variables, `PEEK(53280)`, `POKE 53281, 0`, `SYS addr`, `WAIT addr, mask`.
- **C128 / C65 (V7/V10)**: `GRAPHIC mode`, `BOX`, `CIRCLE`, `PAINT`, `DRAW`, `CHAR`, `COLOR`, `SOUND voice, freq, dur`, `ENVELOPE`, `SPRITE n, state, color...`, `FAST` (2 MHz mode), `SLOW` (1 MHz mode), `SLEEP sec`.

### 7.8 Apple Integer BASIC, AppleSoft BASIC & Apple /// Business BASIC
- **Apple Integer BASIC (Steve Wozniak)**: `GR`, `COLOR=c`, `PLOT x, y`, `HLIN x1, x2 AT y`, `VLIN y1, y2 AT x`, `TEXT`, `CALL addr`, `POKE`, `PEEK`, `PR# slot`, `IN# slot`.
- **AppleSoft BASIC (Microsoft)**: `HGR`, `HGR2`, `HCOLOR=c`, `HPLOT x, y [TO x2, y2]`, `DRAW shape AT x, y`, `XDRAW`, `ROT=n`, `SCALE=n`, `SHLOAD`, `SPEED=n`, `FLASH`, `INVERSE`, `NORMAL`.
- **Apple /// Business BASIC**: `EXCHANGE`, `VIEWPORT`, `PREFIX$`, `TYP`, `WINDOW`.

### 7.9 Atari 8-Bit BASIC & Turbo-BASIC XL
- **Display Modes**: `GRAPHICS mode [+16 full-screen] [+32 no-clear]`, `COLOR c`, `PLOT x, y`, `DRAWTO x, y`, `SETCOLOR reg, hue, lum`, `SOUND voice, pitch, dist, vol`, `STICK(n)`, `STRIG(n)`, `PADDLE(n)`, `PTRIG(n)`, `LOCATE x, y, var`.
- **Turbo-BASIC XL Extensions**: `REPEAT...UNTIL`, `WHILE...WEND`, `DO...LOOP`, `PROC name ... ENDPROC`, `EXEC name`, `BPUT`, `BGET`, `DPOKE`, `DPEEK`.

### 7.10 Texas Instruments TI-99/4A BASIC & Extended BASIC
- **Graphics & Sprites**: `CALL CHAR(char, pattern$)`, `CALL COLOR(set, fg, bg)`, `CALL CLEAR`, `CALL SCREEN(color)`, `CALL HCHAR(r, c, char [, count])`, `CALL VCHAR(r, c, char [, count])`, `CALL SPRITE(#n, char, col, r, c [, v_r, v_c])`, `CALL MOTION(#n, v_r, v_c)`, `CALL PATTERN(#n, char)`, `CALL MAGNIFY(factor)`, `CALL SOUND(dur, freq1, vol1 [, freq2, vol2...])`, `CALL KEY(unit, key_var, status_var)`.
- **Subprograms**: `SUB sub_name ... SUBEND` with `CALL sub_name`.

### 7.11 North Star BASIC (v6 / Advantage)
- Fixed precision BCD arithmetic (6, 8, 10, 12, 14 digits), string slicing `A$(start, end)` without string functions, `FREE(0)` memory reporting.

---

## 8. Missing Features & Dialect Conformance Classification Matrix

The following matrix categorizes all missing keywords, statements, and subsystems into development milestones:

| Identifier / Subsystem | Dialect Lineage | Category | Complexity | Target Milestone | Description |
|---|---|---|:---:|:---:|---|
| **`bppc` Compiler** | Modern / BASIC++ | Compiler | High | **v7.0.0** | Whole-program AOT compiler emitting freestanding C17 code. |
| **`trans` Transpiler** | Modern / Cross-Dialect | Tool | Medium | **v7.0.0** | AST-based dialect translator. |
| **`DEF SEG` / `vmem`** | QuickBASIC / Real Mode | VM / Hardware | Medium | **v6.6.0** | Segmented 20-bit address space mapping & BDA emulation. |
| **FujiNet `N:` SIO** | Atari / FujiNet | Virtual Hardware | Medium | **v6.6.0** | URL stream I/O and TNFS file mounting over `N:`. |
| **EDLIN / VI / WS** | Vintage DOS / Unix | Editors | Medium | **v6.6.0** | Multi-modal terminal editors (`--edlin`, `--vi`, `--ws`). |
| **Pick MultiValue** | PICK / UniVerse | String Engine | Medium | **v6.7.0** | Item/Field/Value/Subvalue delimiters & `EXTRACT`/`REPLACE`. |
| **TI-99 Sprite Suite** | TI-99/4A Ext BASIC | Graphics / Sprites | Medium | **v6.7.0** | `CALL SPRITE`, `CALL MOTION`, `CALL MAGNIFY`, `CALL PATTERN`. |
| **Commodore Sprites** | C128 BASIC v7.0 | Graphics / Sprites | Medium | **v6.7.0** | `SPRITE n, state, color`, `MOVSPR`, `COLLISION`. |
| **Acorn Indirection** | BBC BASIC | Memory Operators | Low | **v6.7.0** | `?addr` (byte), `!addr` (word), `$addr` (string), `|addr` (float). |
| **AppleSoft Shapes** | AppleSoft BASIC | Graphics | Medium | **v6.7.0** | `DRAW shape AT x,y`, `XDRAW`, `ROT=`, `SCALE=`, `SHLOAD`. |
| **HP Slicing `A$(s,e)`**| HP 2000 / 3000 | String Slicing | Low | **v6.6.0** | Direct substring indexing without `MID$` function. |
| **ANSI Exception Handlers**| ANSI Full / ECMA-116 | Control Flow | High | **v7.0.0** | `WHEN EXCEPTION USE` block structures and `RETRY`. |
| **ANSI Graphics Mod 1**| ANSI Full BASIC | Vector Graphics | Medium | **v6.8.0** | `SET WINDOW`, `SET VIEWPORT`, `PLOT POINTS`, `PLOT AREA`. |
| **DEC String Conversions**| DEC BASIC-PLUS-2 | String Math | Low | **v6.6.0** | `CVT$%(str$)`, `CVT%$`, `CVT$F`, `CVTF$`, `XLATE$`, `RAD$`. |
| **DEC Disk Arrays** | DEC BASIC-PLUS | Virtual Storage | High | **v6.8.0** | `DIM #ch, arr(d1, d2)` file-mapped memory array storage. |
| **Turbo Inline ASM** | Turbo BASIC / PowerBASIC | Compiler | High | **v7.0.0** | `INLINE` / `ASM` block machine code execution. |

---

*This document is maintained with strict 100% parity across `engine/PLANNED.md` and `engine/PLANNED.TXT`.*
