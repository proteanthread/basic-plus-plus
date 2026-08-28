# `SEG` / `DEF SEG` Memory Segment Statement

## 1. BASIC Usage and Keyword Definition

The `DEF SEG` (and alias `SEG`) statement defines the active base segment address used by subsequent `PEEK`, `POKE`, `BLOAD`, `BSAVE`, and `CALL ABSOLUTE` statements.

### Syntax Signatures:
```basic
DEF SEG = segment_address&
DEF SEG                         : REM Resets to default BASIC data segment
SEG = segment_address&
```

### Operational Rules:
- **Segmented Addressing**: An absolute physical memory address is formed as:
  $$\text{Address} = (\text{Segment} \times 16) + \text{Offset}$$
- **Default Reset**: Invoking `DEF SEG` without arguments resets the segment register to the default BASIC data segment (`DS`).
- **Standard PC Segments**:
  - `&H0040`: BIOS Data Area (BDA)
  - `&HA000`: EGA/VGA Graphics Framebuffer
  - `&HB000`: MDA Monochrome Text Buffer
  - `&HB800`: CGA/EGA/VGA Color Text Buffer

---

## 2. Code Examples

```basic
10 DEF SEG = &H0040 : REM BIOS Data Area
20 CURSOR_COL = PEEK(&H0050) : REM Cursor column of page 0
30 PRINT "BIOS Cursor Column: "; CURSOR_COL
40 DEF SEG : REM Always restore default segment
```
