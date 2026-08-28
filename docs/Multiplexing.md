# BASIC++ v6.5.2 Multiplexing

## 1. I/O MULTIPLEXING

BASIC++ provides multiplexing statements for routing data between multiple channels, variables, and expressions. The MUX (multiplexer) and DEMUX (demultiplexer) statements are implemented in engine/src/runtime/mux.c.

## 2. MUX (MULTIPLEXER)

MUX selects one of several input sources based on a selector value and assigns the result to a target variable:

```basic
10 Selector = 2
20 MUX Selector, Result, Source1, Source2, Source3
30 PRINT Result    ' Prints the value of Source2
```

MUX evaluates the selector (1-based), picks the corresponding source value from the list, and assigns it to the result variable. If the selector is out of range, the result is set to zero (numeric) or empty string (string).

MUX works with both numeric and string values:

```basic
10 Mode = 3
20 MUX Mode, Label$, "Low", "Medium", "High", "Critical"
30 PRINT Label$    ' Prints "High"
```

## 3. DEMUX (DEMULTIPLEXER)

DEMUX routes a single input value to one of several target variables based on a selector:

```basic
10 Selector = 1
20 Value = 42
30 DEMUX Selector, Value, Target1, Target2, Target3
40 PRINT Target1   ' Prints 42 (others unchanged)
```

DEMUX evaluates the selector, picks the corresponding target variable, and assigns the input value to it. Other target variables are not modified.

## 4. BITMUX (BIT MULTIPLEXER)

BITMUX selects individual bits from multiple source values and combines them into a single result:

```basic
10 A = &HFF        ' 11111111
20 B = &H00        ' 00000000
30 Mask = &HAA     ' 10101010  (selects bits from A where 1, B where 0)
40 BITMUX Mask, Result, A, B
50 PRINT HEX$(Result)    ' Prints "AA"
```

BITMUX uses a bitmask to select which source provides each bit of the result. Where the mask bit is 1, the corresponding bit comes from the first source. Where the mask bit is 0, it comes from the second source.

## 5. STREAM MULTIPLEXING

STREAM.MUX combines output from multiple file channels into a single output stream:

```basic
10 OPEN "LOG1.TXT" FOR INPUT AS #1
20 OPEN "LOG2.TXT" FOR INPUT AS #2
30 OPEN "COMBINED.TXT" FOR OUTPUT AS #3
40 STREAM.MUX #3, #1, #2     ' Interleave lines from #1 and #2 into #3
50 CLOSE #1, #2, #3
```

STREAM.MUX reads lines alternately from the input channels and writes them to the output channel. When one input reaches EOF, remaining lines come from the other input.

## 6. DEVICE MULTIPLEXING

The device multiplexer (engine/src/device/mux.c) manages multiple virtual console instances within the TUI editor multiplexer. Each virtual terminal has its own cursor position, color state, and scroll region. The multiplexer routes keyboard input to the active terminal and renders each terminal's output to the appropriate screen region.

CHVT n switches the active virtual terminal. MUX in the device context refers to the virtual terminal multiplexer, which is distinct from the data MUX statement.

## 7. USE CASES

### Configuration Selection

```basic
10 INPUT "Environment (1=dev, 2=staging, 3=prod): "; Env
20 MUX Env, Server$, "localhost", "staging.example.com", "prod.example.com"
30 MUX Env, Port, 8080, 8443, 443
40 PRINT "Connecting to "; Server$; ":"; Port
```

### Dispatch Tables

```basic
10 INPUT "Operation (1=add, 2=sub, 3=mul, 4=div): "; Op
20 INPUT "A, B: "; A, B
30 MUX Op, Result, A+B, A-B, A*B, A/B
40 PRINT "Result:"; Result
```

### Bit Field Extraction

```basic
10 Status = &HB7                  ' 10110111
20 BITMUX &H0F, LowNibble, Status, 0
30 PRINT "Low nibble:"; HEX$(LowNibble)    ' "7"
```
