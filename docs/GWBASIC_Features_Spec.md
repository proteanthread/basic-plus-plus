# Phase 14 Specification — Classic GW-BASIC Extensions & BGI Palette Integration

## Executive Summary
This specification defines the design and implementation guidelines for **Phase 14 (Classic GW-BASIC Extensions & BGI Palette Integration)**. 
The goal of this phase is to restore compatibility for key interactive text layout functions, macro string integrations, and graphics palette operations from vintage GW-BASIC:
1. **Screen Cursor & Text Inspection**: `CSRLIN`, `POS(n)`, and `LPOS(n)`.
2. **Variable Address Pointers**: `VARPTR` and `VARPTR$`.
3. **Function Key Mapping & Trapping**: `KEY` statements and `ON KEY(n) GOSUB` events.
4. **Color Palette Manipulation**: `PALETTE` and `PALETTE USING` integrated into the BGI (BASIC++ Graphics Interface) layout.

---

## 1. Screen Cursor & Text Inspection

### 1.1: Cursor Row Position (`CSRLIN`)
- **Syntax**: `row = CSRLIN`
- **Description**: Returns the current line/row coordinate (1-indexed, usually 1–25 or 1-40 depending on screen size) of the console text cursor.
- **Implementation**: Queries row index directly from the active Virtual Console (`vdev_vcon.c`) or terminal state.

### 1.2: Cursor Column Position (`POS`)
- **Syntax**: `col = POS(n)`
- **Description**: Returns the current column coordinate (1-indexed, usually 1–80) of the console text cursor. The parameter `n` is a dummy argument (usually `0`) that is evaluated but has no effect.
- **Implementation**: Queries column index from the active Virtual Console.

### 1.3: Printer Column Position (`LPOS`)
- **Syntax**: `col = LPOS(n)`
- **Description**: Returns the current carriage column position of the virtual line printer (LPT1). The parameter `n` specifies the printer channel/port.
- **Implementation**: Tracks column offset updated during printer `LPRINT` stream writes.

---

## 2. Variable Address Pointers (Macro Integration)

### 2.1: Variable Address Offset (`VARPTR`)
- **Syntax**: `addr = VARPTR(var)`
- **Description**: Returns the physical memory address (cast to double/integer representation) of the specified variable's value structure.
- **Implementation**: Resolves the variable in the active symbol table and returns its pointer offset.

### 2.2: Variable Descriptor String (`VARPTR$`)
- **Syntax**: `desc$ = VARPTR$(var)`
- **Description**: Returns a 3-byte string representation of the variable's type and memory location offset.
  - Byte 1: Type descriptor code.
  - Bytes 2-3: 16-bit address offset of the variable.
- **Usage**: Used to pass variables by descriptor reference to macro strings in graphics/sound engines, e.g. `PLAY "O=" + VARPTR$(octave)`.

---

## 3. Function Key Mapping & Trapping

### 3.1: Function Key Configurations (`KEY`)
- **Syntax**:
  - `KEY ON`: Enables display of function key labels (F1 to F10) on the bottom (25th) row of the text mode screen.
  - `KEY OFF`: Hides/clears the function key labels.
  - `KEY LIST`: Prints a complete list of current function key assignments.
  - `KEY n, string$`: Programs function key `n` (1–10 or 1-12) to output `string$` when pressed.
- **Implementation**: Adds a key macro lookup table to `vdev_console` and registers a Row 25 overlay painter.

### 3.2: Key Trap Event (`ON KEY(n) GOSUB`)
- **Syntax**: `ON KEY(n) GOSUB line` / `KEY(n) ON` / `KEY(n) OFF` / `KEY(n) STOP`
- **Description**: Configures and enables an event-driven trap that executes a subroutine when function key `n` is pressed.

---

## 4. Color Palette Manipulation (BGI Integration)

### 4.1: Color Map Redirection (`PALETTE`)
- **Syntax**: `PALETTE [color_idx, actual_color]`
- **Description**: Maps a screen color index `color_idx` to `actual_color` from the active display mode's available hardware palette. Calling `PALETTE` without arguments restores the mode's default hardware color definitions.
- **BGI Integration**: Updates the color lookup table (LUT) registers in the BGI renderer (`bgi_modes.c` and `vdev_gfx.c`) so that subsequent drawing operations (like `LINE` or `CIRCLE`) immediately reflect the mapped color.

### 4.2: Bulk Palette Mapping (`PALETTE USING`)
- **Syntax**: `PALETTE USING integer_array(start_idx)`
- **Description**: Re-maps all palette indices simultaneously using values from an integer array starting at `start_idx`.
- **Implementation**: Traverses the array elements and populates the BGI color LUT.

---

## 5. Verification & Safety Guidelines
- **Unified Testing**: Create a new test suite [tests/system/test_gwbasic_ext.bas](file:///c:/Users/rtdos/GitHub/basic-plus-plus/tests/system/test_gwbasic_ext.bas) verifying `CSRLIN`, `POS`, `LPOS`, `VARPTR`, and `PALETTE`.
- **API Parity**: Document all new keywords in `help/` and `docs/` and register them in `help_data.h` and `catalog.TXT`.
