# BASIC++ v6.5.2 Game Libraries and Utilities

## 1. OVERVIEW

This document catalogs reusable subroutines and functions for game development in BASIC++. These routines can be MERGEd into game programs or stored as library files loaded with CHAIN MERGE.

## 2. RANDOM NUMBER UTILITIES

```basic
' Random integer in range [low, high]
10000 FUNCTION RandInt(low, high)
10010   RandInt = INT(RND * (high - low + 1)) + low
10020 END FUNCTION

' Random element from an array
10100 FUNCTION RandElement(arr(), count)
10110   RandElement = arr(RandInt(LBOUND(arr), LBOUND(arr) + count - 1))
10120 END FUNCTION

' Shuffle an array (Fisher-Yates)
10200 SUB Shuffle(arr(), count)
10210   FOR I = count - 1 TO 1 STEP -1
10220     J = RandInt(0, I)
10230     SWAP arr(LBOUND(arr) + I), arr(LBOUND(arr) + J)
10240   NEXT I
10250 END SUB
```

## 3. MATH UTILITIES

```basic
' Clamp value to range
10300 FUNCTION Clamp(value, lo, hi)
10310   IF value < lo THEN Clamp = lo
10320   ELSE IF value > hi THEN Clamp = hi
10330   ELSE Clamp = value
10340   END IF
10350 END FUNCTION

' Distance between two points
10400 FUNCTION Distance(x1, y1, x2, y2)
10410   Distance = SQR((x2-x1)^2 + (y2-y1)^2)
10420 END FUNCTION

' Linear interpolation
10500 FUNCTION Lerp(a, b, t)
10510   Lerp = a + (b - a) * t
10520 END FUNCTION

' Wrap value to range [0, max)
10600 FUNCTION Wrap(value, maxVal)
10610   Wrap = value - INT(value / maxVal) * maxVal
10620 END FUNCTION
```

## 4. STRING UTILITIES

```basic
' Center text on screen
10700 SUB CenterText(row, text$, fg, bg)
10710   Col = (80 - LEN(text$)) / 2 + 1
10720   COLOR fg, bg
10730   LOCATE row, Col : PRINT text$
10740 END SUB

' Pad string to fixed width
10800 FUNCTION PadRight$(s$, width)
10810   IF LEN(s$) >= width THEN PadRight$ = LEFT$(s$, width)
10820   ELSE PadRight$ = s$ + SPACE$(width - LEN(s$))
10830   END IF
10840 END FUNCTION

' Format number with leading zeros
10900 FUNCTION ZeroPad$(n, digits)
10910   S$ = STR$(n)
10920   S$ = LTRIM$(S$)
10930   WHILE LEN(S$) < digits : S$ = "0" + S$ : WEND
10940   ZeroPad$ = S$
10950 END FUNCTION
```

## 5. SCREEN UTILITIES

```basic
' Flash the screen (damage effect)
11000 SUB FlashScreen(count, delay_ms)
11010   FOR I = 1 TO count
11020     PALETTE 0, 63         ' Flash white
11030     DELAY delay_ms
11040     PALETTE 0, 0          ' Back to black
11050     DELAY delay_ms
11060   NEXT I
11070 END SUB

' Fade to black
11100 SUB FadeOut(steps, delay_ms)
11110   FOR S = steps TO 0 STEP -1
11120     FOR C = 0 TO 15
11130       ' Reduce each color channel proportionally
11140       PALETTE C, C * S / steps
11150     NEXT C
11160     DELAY delay_ms
11170   NEXT S
11180 END SUB

' Draw progress bar
11200 SUB ProgressBar(row, col, width, percent, fg, bg)
11210   Filled = INT(width * percent / 100)
11220   LOCATE row, col
11230   COLOR fg, bg
11240   PRINT STRING$(Filled, CHR$(219)); STRING$(width - Filled, CHR$(176))
11250 END SUB
```

## 6. DATA STRUCTURE UTILITIES

```basic
' Simple stack (using array)
11300 DIM StackData(100) : StackTop = 0
11310 SUB StackPush(value)
11320   SHARED StackData(), StackTop
11330   StackTop = StackTop + 1
11340   StackData(StackTop) = value
11350 END SUB
11360 FUNCTION StackPop()
11370   SHARED StackData(), StackTop
11380   StackPop = StackData(StackTop)
11390   StackTop = StackTop - 1
11400 END FUNCTION

' Simple queue (using circular buffer)
11500 DIM QueueData$(100) : QHead = 0 : QTail = 0
11510 SUB Enqueue(item$)
11520   SHARED QueueData$(), QTail
11530   QueueData$(QTail) = item$
11540   QTail = (QTail + 1) MOD 100
11550 END SUB
11560 FUNCTION Dequeue$()
11570   SHARED QueueData$(), QHead
11580   Dequeue$ = QueueData$(QHead)
11590   QHead = (QHead + 1) MOD 100
11600 END FUNCTION
```

## 7. HIGH SCORE TABLE

```basic
11700 SUB ShowHighScores(scores(), names$(), count)
11710   CLS
11720   CALL CenterText(3, "HIGH SCORES", 14, 0)
11730   FOR I = 1 TO count
11740     LOCATE 5 + I, 20
11750     PRINT ZeroPad$(I, 2); ". ";
11760     PRINT PadRight$(names$(I), 20); " ";
11770     PRINT ZeroPad$(scores(I), 8)
11780   NEXT I
11790 END SUB

11800 SUB SaveHighScores(filename$, scores(), names$(), count)
11810   OPEN filename$ FOR OUTPUT AS #9
11820   FOR I = 1 TO count
11830     WRITE #9, names$(I), scores(I)
11840   NEXT I
11850   CLOSE #9
11860 END SUB
```

## 8. USING THESE LIBRARIES

Save library routines in separate files (e.g., GAMELIB.BAS) and merge into your game:

```basic
10 MERGE "GAMELIB.BAS"
20 ' Your game code starts here...
```

Or use CHAIN MERGE at runtime to overlay the library while preserving your game's variables.
