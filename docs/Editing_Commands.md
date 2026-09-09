<!--
Title:        Editing_Commands
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/editor/, engine/include/statements/program/
Generated:    no, manual reference
Status:       current
-->

# BASIC++ v6.5.2 Editing Commands

## 1. THE LINE EDITOR

EDIT n opens line n in the built-in interactive line editor. The editor displays the line text and places the cursor at the beginning. You can modify the line using standard editing keys:

- **Left/Right arrows**: Move the cursor within the line.
- **Home/End**: Jump to the beginning or end of the line.
- **Insert**: Toggle between insert and overwrite mode.
- **Delete**: Delete the character under the cursor.
- **Backspace**: Delete the character before the cursor.
- **Enter**: Accept the changes and store the modified line.
- **Escape**: Cancel editing and restore the original line.

EDIT without a line number edits the last line that produced an error, allowing immediate correction of syntax or runtime errors.

## 2. AUTO LINE NUMBERING

AUTO starts automatic line numbering. Each time you press Enter after typing a statement, the next line number is generated automatically:

```basic
> AUTO
10 PRINT "LINE ONE"
20 PRINT "LINE TWO"
30 END
.
>
```

`AUTO start,step` specifies the starting number and increment: `AUTO 100,5` generates 100, 105, 110, etc. Type a period (`.`) as the first character on an empty line or press Ctrl+C to exit AUTO mode.

If AUTO generates a line number that already exists in memory, the existing line is displayed with an asterisk (`*`) prefix. You can press Enter to preserve the existing line or type a new statement to replace it.

## 3. LINE MANAGEMENT

LIST displays program lines from memory:
- `LIST`: Displays the entire program.
- `LIST n`: Shows line n only.
- `LIST n1-n2`: Shows lines within range n1 to n2 inclusive.
- `LIST -n`: Shows all lines from the start up to line n.
- `LIST n-`: Shows all lines from line n onward to the end.

LLIST sends the program listing to the printer device (`LPT1:` / `PRN:`).

DELETE removes lines from memory:
- `DELETE n`: Removes line n.
- `DELETE n1-n2`: Removes lines in the range n1 through n2.
- `DELETE -n`: Removes all lines from the start up to line n.
- `DELETE n-`: Removes all lines from line n onward.

RENUM renumbers the lines in the program:
- `RENUM`: Renumbers the entire program starting at line 10 with step 10.
- `RENUM new,old,step`: Renumbers lines starting from old line number `old`, assigning new numbers starting at `new` with increment `step`. All branch targets (`GOTO`, `GOSUB`, `ON...GOTO`, `ON...GOSUB`, `RESTORE`, `RESUME`, `RUN`) are automatically updated.

## 4. THE TUI EDITOR MULTIPLEXER

The `baspp` desktop edition includes a full-screen TUI (Text User Interface) editor multiplexer that provides a multi-window editing environment. The multiplexer supports multiple simultaneous editor instances, each in its own virtual terminal.

The editor is implemented in `engine/src/editor/` and is part of the `libstandard` library. It uses the Windows Console API on Windows and ncurses on Linux.

### Editor Personalities

The TUI editor supports four editing personalities:
- **EDIT mode**: The default BASIC++ full-screen editor with line-number-aware editing, syntax highlighting, and block operations.
- **EDLIN mode**: An MS-DOS EDLIN-compatible line editor supporting insert, delete, list, search, and replace commands.
- **VI mode**: A vi-compatible modal editor supporting normal, insert, and command modes with standard movement and ex commands (`:w`, `:q`, `:wq`).
- **WS mode**: A WordStar-compatible editor using Ctrl-key sequences for navigation and block manipulation.

### Multiplexer Commands

The editor multiplexer allows multiple files to be opened simultaneously in separate virtual terminals:
- `MUX NEW`: Open a new editor window.
- `MUX CLOSE`: Close the current editor window.
- `MUX NEXT` / `MUX PREV`: Switch between editor windows.
- `MUX LIST`: List all open editor windows.
- `CHVT n`: Switch directly to virtual terminal n.

## 5. REFORMAT

REFORMAT standardizes the formatting of the current program in memory without altering line numbers or execution logic. It normalizes keyword capitalization to uppercase (e.g., `PRINT`, not `print`), standardizes statement spacing, and formats block structure indentation (`IF/END IF`, `FOR/NEXT`, `SUB/END SUB`).

## 6. CLIPBOARD OPERATIONS

The TUI editor integrates with the host operating system clipboard through `engine/lib/platform/plat_clipboard.c`:
- **Copy**: Copies selected text to the host system clipboard.
- **Cut**: Cuts selected text and copies it to the system clipboard.
- **Paste**: Inserts text from the system clipboard at the cursor position.
