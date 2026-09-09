<!--
Title:        TUI_Programming
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs)
Authority:    engine/src/device/vcon.c, console.c, bgi_text.c,
              engine/lib/platform/plat_console.c,
              engine/src/statements/ui/widgets/ (16 files),
              engine/src/eval/functions/ui/dialogs/,
              engine/src/editor/, engine/src/statements/system/mouse.c
Generated:    no, hand-written
Status:       current
-->

# Text User Interface Programming

Screens, windows, widgets and dialogs in text mode — and an honest answer to
"is there a development environment like Lazarus or IDLE?"

---

## 1. The layers

| Layer | Source | What it does |
|---|---|---|
| Host terminal | `engine/lib/platform/plat_console.c` (19 KB) | Raw and cooked modes, cursor, colour attributes, key reading, window size, on Windows and POSIX |
| Virtual console | `engine/src/device/vcon.c` | Character cells, 16-colour attributes, cursor coordinates, ANSI escape parsing. The `CON1:` through `CON8:` devices |
| Console statements | `engine/src/device/console.c` | `LOCATE`, `COLOR`, `CLS`, `WIDTH`, `VIEW PRINT` |
| Text graphics | `bgi_text.c` | BGI primitives rendered as characters |
| Widgets | `engine/src/statements/ui/widgets/` | Sixteen widget statements |
| Dialogs | `engine/src/eval/functions/ui/dialogs/` | `MSGBOX`, `INPUTBOX`, `FILEOPENBOX`, `FILESAVEBOX` |
| Editors | `engine/src/editor/` | Four full-screen editor personalities |

An earlier version of this document cited `engine/src/tui/` as the multiplexer.
That directory does not exist. `engine/src/editor/tui_multiplexer.c` exists and
is 538 bytes: two functions, `tui_multiplexer_init` and
`tui_multiplexer_shutdown`, each forwarding to the platform layer. There is no
split-pane or overlapping-window manager behind it.

---

## 2. The text grid

| Statement | Purpose |
|---|---|
| `SCREEN 0` | Select text mode |
| `WIDTH cols [, rows]` | Console dimensions: 40, 80 or 132 columns |
| `LOCATE row, col [, visible]` | Position the cursor, 1-based, and set visibility |
| `COLOR fg, bg` | 16-colour CGA/EGA/VGA attributes |
| `VIEW PRINT top TO bottom` | Bounded scrolling region, so headers and status lines stay put |
| `CLS [mode]` | Clear the screen or the viewport |
| `WINDOW` | Coordinate window |
| `PCOPY` | Copy a display page |
| `CSRLIN`, `POS(0)` | Read the cursor row and column |
| `INKEY$` | Non-blocking key read |
| `PRINT AT`, `TAB`, `SPC` | Positioned output |

`VIEW PRINT` is the workhorse for a full-screen program: fix rows 1 and 25 for
a title and a status line, let rows 2 to 24 scroll.

```basic
10 SCREEN 0 : WIDTH 80, 25 : CLS
20 COLOR 15, 1 : LOCATE 1, 1 : PRINT SPACE$(80);
30 LOCATE 1, 30 : PRINT "INVENTORY  v1.0";
40 COLOR 7, 0
50 VIEW PRINT 3 TO 23
60 CLS 2
70 REM rows 3-23 now scroll; rows 1, 2, 24, 25 do not
```

---

## 3. The widgets

Sixteen statements, one file each, in `engine/src/statements/ui/widgets/`:

`BUTTON`, `CHECKBOX`, `COMBOBOX`, `DIRLISTBOX`, `DRIVELISTBOX`,
`FILELISTBOX`, `FRAME`, `HSCROLLBAR`, `LABEL`, `LISTBOX`, `MENU`, `MSGBOX`,
`OPTIONBUTTON`, `TEXTBOX`, `TIMERCONTROL`, `VSCROLLBAR`.

The names are Visual Basic's, deliberately, so a VB DOS programmer recognises
them.

**What they actually do.** Each parses its arguments and writes a text
rendering through the virtual device. `BUTTON caption$` emits
`[ < caption > ]`. They are real code and they draw.

**What they do not do.** There is no event loop, no focus model, and no hit
testing. Nothing dispatches a click or a keypress to a widget. `TIMERCONTROL`
registers a timer; the rest are draw calls.

So the honest description is: **the widget statements draw a form, they do not
run one.** A program builds its own input loop with `INKEY$` and decides for
itself what a keystroke means.

```basic
10 SUB DrawForm
20   LOCATE 5, 10 : FRAME 40, 8
30   LOCATE 6, 12 : LABEL "Name:"
40   LOCATE 6, 20 : TEXTBOX Name$, 20
50   LOCATE 8, 12 : CHECKBOX "Active", Active%
60   LOCATE 10, 12 : BUTTON "OK"
70   LOCATE 10, 22 : BUTTON "Cancel"
80 END SUB
90 REM your own loop reads INKEY$ and moves the focus
```

Making them interactive — mouse, focus, hit testing, an event loop — is
phase 3.7 of the 7.0.0 plan.

---

## 4. Dialogs

These are functions rather than statements, and they do return a value:

| Function | Returns |
|---|---|
| `MSGBOX(prompt$ [, buttons%] [, title$])` | The button chosen |
| `INPUTBOX(prompt$ [, title$] [, default$])` | The text entered |
| `FILEOPENBOX([filter$] [, title$])` | A path, or empty if cancelled |
| `FILESAVEBOX([filter$] [, title$])` | A path, or empty if cancelled |
| `DOEVENTS` | Yields to pending events |

`MSGBOX` also exists as a statement.

---

## 5. Mouse input does not work

Stated here because a TUI is where a reader will look for it.

All six mouse handlers in `engine/src/statements/system/mouse.c` — `MOUSE`,
`MOUSE INPUT`, `MOUSE SHOW`, `MOUSE HIDE`, `HMOUSE`, `VMOUSE` — are
`(void)vm; (void)lex; return err;`. The statement parses and does nothing.
`PEN`, `STICK` and `STRIG` are the same.

The keyboard is the only working input device. Design accordingly, and see
`Physical_Device_Access` section 5.

---

## 6. Is there a development environment?

The question is usually asked as "something like Lazarus, or like IDLE". Those
are two quite different bars, and BASIC++ clears one of them.

### What exists

**Four editor personalities**, in `engine/src/editor/`, each also built as a
standalone binary: `EDIT` (the QBASIC-style full-screen editor), `VI`, `WS`
(WordStar keybindings), and `EDLIN` (line editor). `EDIT` is the one a QBASIC
programmer will recognise.

**Program management from the prompt**, which is the vintage IDE model and is
complete: `LIST`, `LLIST`, `AUTO`, `DELETE`, `RENUM`, `MERGE`, `LOAD`, `SAVE`,
`NEW`, `RUN`, `CONT`, `EDIT`.

**Refactoring tools**, which most vintage BASICs never had: `RENUM` (32 KB of
implementation, handling every referencing construct), `RENAME` (29 KB,
variable and procedure renaming), `REFORMAT` (indentation, unpacking of
multi-statement lines, analysis and reporting), and `semantic_harvester.c`
(32 KB) behind the modernisation path. See `Program_Modernization_Tutorial`.

**A debugger**: `TRON`, `TROFF`, `TRACE`, `DEBUG`, `STOP`, `CONT`,
breakpoints, `CHECK` and `VERIFY` static analysis, `SELFTEST`, and `TEST` for
assertions. See `Debugging_And_Testing`.

**Introspection at the prompt**: `HELP`, `KEYWORD`, `CATEGORY`, `CATALOG`,
`VARS`, `SCOPE`, `DEVICES`, `MOUNTS`, `MEMMAP`, `VERSION`. You can ask the
running system what it knows about itself.

### What does not exist

- No form designer. Nothing lays out widgets visually.
- No event-driven form runtime, for the reasons in section 3.
- No project model — no notion of a multi-file project with build settings.
- No integrated build-and-run of the compiler target from inside the editor.
- No mouse, so no click-to-position-cursor and no menus that drop down.
- No syntax-highlighted editing pane.

### The answer

**BASIC++ has an IDE in the QBASIC or IDLE sense: an integrated
edit-run-debug-introspect loop at a prompt.** That loop is genuinely good —
better than QBASIC's, because of the refactoring and analysis tools.

**It does not have an IDE in the Lazarus or Visual Basic sense**, because that
means a form designer plus an event-driven runtime, and neither exists. The
widget statements look like the start of one, and phase 3.7 of the 7.0.0 plan
is what would turn them into it.

If you want the Lazarus experience today, the practical route is to write the
interface in Free Pascal or C and embed BASIC++ as the scripting engine
underneath. See `Language_Interop` and `C_Programmers_Guide`.

---

## 7. Portability

`plat_console.c` covers Windows and POSIX. On Linux and the BSDs the build
links ncurses when `STANDALONE_EDITOR` is not defined. On FreeDOS the console
is direct BIOS and video memory. On a bare-metal or UEFI target the console is
whatever `SysConsole` is bound to; on UEFI that would be `ConOut` and `ConIn`,
which is part of what phase 5.1 of the 7.0.0 plan has to finish.

The 40-column width exists for vintage targets and the 132-column width for
terminals and line printers. A TUI that must run everywhere should assume 80
by 24 and query with `WIDTH` rather than hardcoding.

---

## See also

- `Screen_And_Console` for the console statements in detail
- `Virtual_Consoles` for the `CON1:` through `CON8:` device family
- `Editing_Commands` for the four editor personalities
- `Graphics_Modes` and `BGI_Graphics_And_SDL2_Decomposition` for graphical
  output
- `Physical_Device_Access` for what input hardware works
- `Debugging_And_Testing` for the debugger
- `Game_Development_Guide` for text-mode games
