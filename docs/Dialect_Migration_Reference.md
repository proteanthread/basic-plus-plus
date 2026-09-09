<!--
Title:        Dialect_Migration_Reference
Tier:         1
Applies to:   BASIC++ v6.5.2, all targets
Authority:    docs/keywords/statements/ (371 entries),
              docs/keywords/functions/ (316 entries)
Generated:    no, hand-written
Status:       current
-->

# Dialect Migration Reference

How to bring a program written for a vintage BASIC into BASIC++, dialect by
dialect and construct by construct.

Rule #1 of this project is compatibility with vintage Microsoft BASICA,
GW-BASIC, QuickBASIC and Visual BASIC for DOS. This document is where that
rule is demonstrated rather than asserted: every row below names a construct
from a real machine and says what happens to it here.

## How to read the tables

Each row carries a **verdict**:

| Verdict | Means |
|---|---|
| **EXACT** | The same keyword exists and behaves the same way. Paste and run. |
| **NEAR** | A direct equivalent exists with a difference worth knowing. The note says what. |
| **REWRITE** | The intent is supported, the spelling is not. The note gives the replacement. |
| **NONE** | Nothing corresponds. The note says what to do instead. |
| **CHECK** | The keyword exists in BASIC++, but whether it carries this dialect's exact semantics has not been verified against the handler. Treat as NEAR until confirmed. |

CHECK is used honestly and often. A keyword sharing a name with a vintage one
is evidence, not proof, and this document would be worth less if it pretended
otherwise. Each CHECK row names the keyword page to read.

## Before you start

Three things are true of every migration and are not repeated in every table.

**Line numbers.** The interpreter expects them; the compiler and transpiler do
not. A vintage program loads as-is. Use `RENUM` to renumber, and note that
functions and subroutines are called by name regardless.

**Integer truth.** Vintage BASIC returns -1 for true and 0 for false, and
programs rely on it (`A = -(B > C)` is idiomatic). BASIC++ preserves this.

**`FOR` loop bounds.** A vintage Microsoft `FOR` evaluates its limit and step
expressions once, on entry, and tests the condition before the first
iteration, so a loop whose limit is already exceeded runs zero times.
Programs depend on both details. BASIC++ preserves them.

---

## 1. Microsoft lineage: GW-BASIC, BASICA, QuickBASIC, VB DOS

This is the baseline the whole project is built to. Expect EXACT almost
everywhere; the interesting rows are the exceptions.

### Core language

| Construct | Verdict | Note |
|---|---|---|
| `PRINT`, `PRINT USING` | EXACT | Full `USING` format engine |
| `INPUT`, `LINE INPUT`, `INPUT$` | EXACT | |
| `GOTO`, `GOSUB`, `RETURN` | EXACT | |
| `ON n GOTO`, `ON n GOSUB` | EXACT | |
| `IF ... THEN ... ELSE` | EXACT | Single-line and block forms |
| `FOR`/`NEXT`, `WHILE`/`WEND` | EXACT | |
| `DO`/`LOOP`, `EXIT` | EXACT | QuickBASIC forms |
| `SELECT CASE` | EXACT | `SELECT` |
| `DEF FN` | EXACT | `DEF_FN` |
| `SUB`, `FUNCTION`, `DECLARE` | EXACT | |
| `TYPE ... END TYPE` | EXACT | |
| `CONST`, `SHARED`, `STATIC`, `COMMON` | EXACT | |
| `DIM`, `REDIM`, `ERASE`, `OPTION BASE` | EXACT | `OPTION BASE 0/1` retained |
| `LBOUND`, `UBOUND` | EXACT | |
| `DEFINT`, `DEFSNG`, `DEFDBL`, `DEFSTR`, `DEFLNG` | EXACT | |
| `SWAP`, `LET`, `REM` | EXACT | |
| `ON ERROR GOTO`, `RESUME`, `ERR`, `ERL` | EXACT | |
| `DATA`, `READ`, `RESTORE` | EXACT | |
| `RANDOMIZE`, `RND` | EXACT | |

### Strings and conversion

`LEFT$`, `RIGHT$`, `MID$` (function and statement), `LEN`, `INSTR`, `CHR$`,
`ASC`, `STR$`, `VAL`, `SPACE$`, `STRING$`, `LTRIM$`, `RTRIM$`, `UCASE$`,
`LCASE$`, `HEX$`, `OCT$`, `CVI`, `CVS`, `CVD`, `MKI$`, `MKS$`, `MKD$` — all
EXACT. The Microsoft Binary Format variants `CVSMBF`, `CVDMBF`, `MKSMBF$`,
`MKDMBF$` are present, which matters when reading files written by a
pre-IEEE Microsoft BASIC.

### File and device I/O

| Construct | Verdict | Note |
|---|---|---|
| `OPEN`, `CLOSE`, `PRINT#`, `INPUT#`, `WRITE#` | EXACT | |
| `GET`, `PUT`, `FIELD`, `LSET`, `RSET` | EXACT | Random-access records |
| `LOC`, `LOF`, `EOF`, `SEEK` | EXACT | |
| `LOCK`, `UNLOCK` | EXACT | |
| `BLOAD`, `BSAVE`, `BRUN` | EXACT | |
| `KILL`, `NAME`, `FILES`, `MKDIR`, `RMDIR`, `CHDIR` | EXACT | |
| `SHELL`, `ENVIRON$` | EXACT | |

### Graphics and sound

`SCREEN`, `COLOR`, `PSET`, `PRESET`, `LINE`, `CIRCLE`, `PAINT`, `DRAW`,
`PALETTE`, `POINT`, `PMAP`, `VIEW`, `WINDOW`, `GET`/`PUT` for sprites,
`PCOPY`, `SOUND`, `PLAY`, `BEEP` — all EXACT. See `Graphics_Modes` for which
modes each target supports.

### Systems access

`PEEK`, `POKE`, `INP`, `OUT`, `WAIT`, `VARPTR`, `DEF SEG`, `USR`, `CALL`,
`SYS` — EXACT. See `Systems_Programming`.

### Visual Basic for DOS controls

This is the surprise, and it is undocumented elsewhere. The VB DOS control
set is present as statements:

`FORM`, `FRAME`, `LABEL`, `TEXTBOX`, `BUTTON`, `CHECKBOX`, `OPTIONBUTTON`,
`LISTBOX`, `COMBOBOX`, `DIRLISTBOX`, `DRIVELISTBOX`, `FILELISTBOX`, `IMAGE`,
`PICTURE`, `MENU`, `HSCROLLBAR`, `VSCROLLBAR`, `TIMERCONTROL`

with the dialog functions `MSGBOX`, `INPUTBOX$`, `FILEOPENBOX$` and
`FILESAVEBOX$`.

Verdict: **CHECK** across the board. The keywords are registered and each has
a reference page, but whether the event model and property semantics match VB
DOS has not been verified handler by handler. Read the individual pages under
`docs/keywords/statements/` before porting a form-heavy program, and report
any divergence.

---

## 2. TRS-80 and Tandy

Level II BASIC and the Model III/4 line.

| TRS-80 construct | BASIC++ | Verdict | Note |
|---|---|---|---|
| `CLS` | `CLS` | EXACT | |
| `PRINT @ n, x` | `PRINT AT` | NEAR | `PRINT AT` is a distinct statement; the `@` sigil form needs rewriting |
| `SET(x,y)`, `RESET(x,y)` | `SET`, `RESET` | CHECK | Both keywords exist; verify they carry the block-graphics semantics rather than the general-purpose ones |
| `POINT(x,y)` | `POINT` | NEAR | Present; TRS-80 returns block-graphics state |
| `INKEY$` | `INKEY$` | EXACT | |
| `RANDOM` | `RANDOMIZE` | REWRITE | Same intent, Microsoft spelling |
| `LPRINT`, `LLIST` | `LPRINT`, `LLIST` | EXACT | |
| `DEFINT`/`DEFSNG`/`DEFDBL`/`DEFSTR` | same | EXACT | |
| `PEEK`, `POKE`, `USR` | same | EXACT | |
| `CMD"..."` | `SHELL` | REWRITE | The `CMD` string escapes vary by model; map each to its BASIC++ equivalent individually |
| `ERROR n`, `ON ERROR GOTO` | same | EXACT | |
| `MEM` | `FRE` | REWRITE | `MEM` exists but is the IoT memory statement; use `FRE` for free memory |

The `MEM` row is a genuine collision and the kind of thing that silently
breaks a port: both names exist, and they mean different things.

---

## 3. Commodore

PET, VIC-20, C64, C128 BASIC 2.0 and 7.0.

| Commodore construct | BASIC++ | Verdict | Note |
|---|---|---|---|
| `POKE`, `PEEK` | same | EXACT | Address space is virtualised; see below |
| `SYS addr` | `SYS` | NEAR | Present as a function; the C64 sense is a call to machine code at an address |
| `GET A$` | `INKEY$` | REWRITE | Commodore `GET` is non-blocking single-key; BASIC++ `GET` is the record-I/O statement |
| `PRINT#`, `INPUT#`, `CMD` | `PRINT#`, `INPUT#` | NEAR | `CMD` redirection has no direct equivalent; use `OPEN` to the device |
| `OPEN 1,8,15,"..."` | `OPEN` | REWRITE | The device/secondary-address form maps onto the virtual device layer; see `Virtual_Devices` |
| `SCRATCH`, `RENAME` | `SCRATCH`, `RENAME` | CHECK | Both keywords exist. Commodore's are disk-command strings; verify the BASIC++ semantics |
| `VERIFY` | `VERIFY` | CHECK | Exists as both statement and function |
| `SOUND`, `VOL` | `SOUND`, `VOICE` | NEAR | SID-specific registers have no equivalent; use `SOUND` or `POKE` |
| `COLOR`, `GRAPHIC` (7.0) | `COLOR`, `GRAPHICS` | NEAR | |
| `DRAW`, `CIRCLE`, `BOX`, `PAINT` (7.0) | `DRAW`, `CIRCLE`, `PAINT` | NEAR | No `BOX`; use `LINE` with the box option |
| PETSCII characters | — | NONE | Character set differs from ASCII. See note below |

**PETSCII.** Commodore's character set is not ASCII, and screen-poke programs
depend on the screen-code mapping rather than the PETSCII one. There is no
translation layer today. A program that writes text by poking screen memory
will need its codes remapped by hand.

---

## 4. Atari 8-bit

Atari BASIC, 400/800/XL/XE.

| Atari construct | BASIC++ | Verdict | Note |
|---|---|---|---|
| `GRAPHICS n` | `GRAPHICS` | NEAR | Mode numbers differ from the IBM `SCREEN` modes; see `Graphics_Modes` |
| `PLOT x,y` | `PLOT` | EXACT | |
| `DRAWTO x,y` | `DRAWTO` | EXACT | |
| `COLOR n` | `COLOR` | NEAR | Atari `COLOR` selects a register, not an RGB value |
| `SETCOLOR r,h,l` | `PALETTE` | REWRITE | Palette register assignment |
| `POSITION x,y` | `LOCATE` or `AT` | REWRITE | |
| `XIO cmd,#ch,aux1,aux2,"dev"` | `XIO` | CHECK | The keyword exists; verify the CIO command numbers are honoured |
| `SOUND v,p,d,vol` | `SOUND` | NEAR | Four-channel POKEY form differs from the PC speaker form |
| `OPEN #1,4,0,"D:FILE"` | `OPEN` | REWRITE | Device prefixes map onto the virtual device layer |
| `PRINT #6;` | `PRINT#` | NEAR | |
| `USR(addr,args)` | `USR` | NEAR | |
| `PEEK`, `POKE` | same | EXACT | |
| String slicing `A$(3,5)` | `MID$` | REWRITE | Atari uses substring-by-range on the variable itself. See below |

**Atari string slicing.** `A$(3,5)` in Atari BASIC is a substring, not an
array element. BASIC++ uses `MID$(A$,3,3)`. Note that BASIC++ *does* support
bracket slicing in the HP style, `A$[3,5]`, documented in
`Timeshare_Mainframe_Dialects` — but the Atari parenthesis form collides with
array indexing and is not accepted.

---

## 5. Apple II

Applesoft BASIC and Integer BASIC.

| Applesoft construct | BASIC++ | Verdict | Note |
|---|---|---|---|
| `HOME` | `HOME` | EXACT | |
| `TEXT` | `TEXT` | EXACT | |
| `HTAB`, `VTAB` | `LOCATE` or `AT` | REWRITE | No `HTAB`/`VTAB` keyword |
| `INVERSE`, `FLASH`, `NORMAL` | `INVERSE`, `FLASH` | NEAR | No `NORMAL`; reset attributes explicitly |
| `GR`, `HGR`, `HGR2` | `GRAPHICS` | REWRITE | Low-res and hi-res select through `GRAPHICS` |
| `PLOT`, `HPLOT` | `PLOT` | NEAR | One statement covers both; resolution follows the mode |
| `COLOR=`, `HCOLOR=` | `COLOR` | REWRITE | Drop the `=` |
| `IN#`, `PR#` | `IN#`, `PR#` | EXACT | Both keywords are present |
| `ONERR GOTO`, `RESUME` | `ON ERROR GOTO`, `RESUME` | REWRITE | Microsoft spelling |
| `PEEK`, `POKE`, `CALL` | same | EXACT | |
| `GET A$` | `INKEY$` | REWRITE | As Commodore |
| `DEL`, `LIST`, `NEW`, `RUN` | `DELETE`, `LIST`, `NEW`, `RUN` | NEAR | `DEL` becomes `DELETE` |
| `&` machine-language hook | `CALL` or `EXTERN` | REWRITE | See `Extension_Guide` |
| Integer BASIC `#` types | — | NONE | Integer BASIC is a separate dialect; use `DEFINT` |

---

## 6. What BASIC++ adds that no vintage dialect had

Worth knowing before you port, because some of it removes the reason a vintage
program was written the way it was.

**Matrix operations.** `MAT`, `MAT READ`, `MAT PRINT`, `MAT INPUT`,
`MAT WRITE`, with `MAT_IDN`, `MAT_INV`, `MAT_TRN`, `MAT_RND`, `DET`, `DOT`
and `CROSS`. Dartmouth and HP heritage. A program that hand-rolls matrix
multiplication in nested `FOR` loops can usually be shortened to one line.

**Pick MultiValue.** `EXTRACT`, `INSERT$`, `REPLACE$`, `DELETE$`,
`DYNARRAY$`, `PARSE_DYNARRAY`, plus `READU`, `WRITEU`, `RELEASE`, `LOCKED`
and the `SEEKEQ`/`SEEKGE`/`SEEKGT` index verbs. See
`Set_Based_Object_Model` section 6.

**Structured error handling.** `TRY`, `RAISE`, `RETRY`, `HANDLER`, `WHEN`,
`WHENEVER` alongside the vintage `ON ERROR GOTO`.

**Modules and scope.** `MODULE`, `IMPORT`, `PUBLIC`, `GLOBAL`, `SCOPE`,
`NAMESPACE`-style organisation. See `Module_Guide`.

**Sets and groups.** The object model in `Set_Based_Object_Model`.

**Complex numbers.** `COMPLEX`, `REAL`, `IMAG`, `CONJ`, `CABS`, `CARG`,
`CEXP`, `CLOG`, `CPOW`, `CSQR`.

**Financial functions.** `PV`, `FV`, `PMT`, `IPMT`, `PPMT`, `NPER`, `RATE`,
`IRR`, `NPV`.

---

## 7. How to verify a port

1. Load the program and `LIST` it. Syntax the lexer rejects shows up here.
2. `RENUM` if you intend to compile rather than interpret.
3. Run against the vintage output you already have. Character-for-character
   comparison catches the `PRINT` zone and `USING` differences that nothing
   else will.
4. Check every `PEEK`, `POKE`, `CALL` and `USR`. These are the constructs most
   likely to have depended on a specific machine, and the ones a translation
   table cannot help with.
5. Check every string-slicing expression against section 4 and section 5.

## 8. What is not covered here

The dialects in this document are the six the project was asked to support
first. `BASIC_Dialects_Keyword_Reference.xlsx` in `project notes/` covers 127
dialects and is the wider reference; `Historical_Dialect_Feature_Import_Catalogue`
and `Vintage_Ecosystems_Reference` carry the surrounding history.

Sinclair ZX Spectrum users: `INK`, `PAPER`, `BRIGHT`, `FLASH`, `OVER`,
`INVERSE`, `BORDER`, `AT` and `PLOT` are all present. See `Sinclair_BASIC`.

## See also

- `Legacy_BASIC_Modernization_Guide` for line numbers and modernisation
- `VBDOS_QuickBASIC_PDS` for the Microsoft dialects in depth
- `Timeshare_Mainframe_Dialects` for HP, DEC and the mainframe lineage
- `Systems_Programming` for `PEEK`, `POKE`, `INP` and `OUT`
