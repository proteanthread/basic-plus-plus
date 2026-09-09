<!--
Title:        Multiplexing
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/mux.c, engine/src/device/mux.c
Generated:    no, manual reference
Status:       current
-->

# BASIC++ v6.5.2 Multiplexing Reference

## 1. I/O & DATA MULTIPLEXING OVERVIEW

BASIC++ provides native multiplexing statements for routing data between multiple channels, variable targets, and streams. The core multiplexer (`MUX`), demultiplexer (`DEMUX`), and bitwise multiplexer (`BITMUX`) statements are implemented in `engine/src/runtime/mux.c`. Virtual terminal multiplexing for multi-window console editing is implemented in `engine/src/device/mux.c`.

## 2. THE MUX STATEMENT (MULTIPLEXER)

`MUX selector, result_var, source1, source2 [, source3, ...]` selects one value from an input list based on a 1-based selector and assigns it to `result_var`:

```basic
10 Selector = 2
20 MUX Selector, Result, 100, 200, 300, 400
30 PRINT Result    ' Prints 200
```

`MUX` evaluates the selector:
- If `selector = 1`, `source1` is assigned to `result_var`.
- If `selector = 2`, `source2` is assigned to `result_var`.
- If `selector` is out of bounds (less than 1 or greater than the number of sources), `result_var` is cleared (0 for numeric, `""` for string).

`MUX` operates transparently on both numeric and string values:

```basic
10 Priority% = 3
20 MUX Priority%, Label$, "Low", "Normal", "High", "Critical"
30 PRINT "Status: "; Label$    ' Prints "High"
```

## 3. THE DEMUX STATEMENT (DEMULTIPLEXER)

`DEMUX selector, input_val, target1, target2 [, target3, ...]` routes an input value to one of several target variables based on a 1-based selector:

```basic
10 Selector = 1
20 Value = 99.5
30 DEMUX Selector, Value, SensorA, SensorB, SensorC
40 PRINT SensorA   ' Prints 99.5 (SensorB and SensorC remain unchanged)
```

Target variables not selected retain their prior values without modification.

## 4. THE BITMUX STATEMENT (BIT MULTIPLEXER)

`BITMUX mask, result_var, sourceA, sourceB` performs bitwise selection between two integer values according to a bitmask:

```basic
10 A = &HFF        ' Binary: 11111111
20 B = &H00        ' Binary: 00000000
30 Mask = &HAA     ' Binary: 10101010
40 BITMUX Mask, Result, A, B
50 PRINT HEX$(Result)    ' Prints "AA"
```

For each bit position:
- Where the bit in `mask` is 1, the corresponding bit is taken from `sourceA`.
- Where the bit in `mask` is 0, the corresponding bit is taken from `sourceB`.

## 5. STREAM MULTIPLEXING

`STREAM.MUX` interleaves line data from multiple file channels into a designated output channel:

```basic
10 OPEN "LOG_A.TXT" FOR INPUT AS #1
20 OPEN "LOG_B.TXT" FOR INPUT AS #2
30 OPEN "MERGED.TXT" FOR OUTPUT AS #3
40 STREAM.MUX #3, #1, #2     ' Interleave lines into #3
50 CLOSE #1, #2, #3
```

Lines are read sequentially from input channels in round-robin fashion until all inputs reach EOF.

## 6. VIRTUAL TERMINAL MULTIPLEXING

The device multiplexer (`engine/src/device/mux.c`) manages multiple concurrent virtual terminal instances in the desktop TUI environment:
- Each virtual terminal retains independent cursor coordinates, color attributes, and scrolling regions.
- `CHVT n`: Switches active display focus directly to virtual terminal `n`.
- `MUX NEW`, `MUX CLOSE`, `MUX NEXT`, `MUX PREV`, `MUX LIST`: Manage interactive editor windows within the TUI multiplexer.

## 7. PRACTICAL USAGE PATTERNS

### Configuration Dispatch
```basic
10 INPUT "Select Profile (1=Dev, 2=Staging, 3=Prod): ", P%
20 MUX P%, Host$, "127.0.0.1", "staging.api.net", "api.company.com"
30 MUX P%, Port%, 8080, 8443, 443
40 PRINT "Connecting to "; Host$; ":"; Port%
```

### Operation Lookup
```basic
10 INPUT "Operation (1=Add, 2=Sub, 3=Mul, 4=Div): ", Op%
20 INPUT "Operands X, Y: ", X, Y
30 MUX Op%, Ans, X + Y, X - Y, X * Y, X / Y
40 PRINT "Result = "; Ans
```
