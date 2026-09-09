<!--
Title:        Sinclair_BASIC
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/gfx/, engine/src/audio/audio_synth.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Sinclair BASIC (ZX Spectrum) Compatibility Architecture

The authoritative specification for Sinclair ZX Spectrum BASIC language compatibility, attribute-based graphics, sound generation, and string slicing in BASIC++ v6.5.2.

---

## 1. History & Architectural Overview

Sinclair BASIC was the resident ROM programming language of the Sinclair ZX Spectrum series (1982-1992). It was renowned for its attribute-based color graphics model, single-statement sound synthesis (`BEEP`), and flexible string range slicing.

In BASIC++ v6.5.2, Sinclair BASIC features are integrated directly into the unified master dialect with zero dialect switches or mode gates (upholding the Zero DIALECT invariant). All Sinclair constructs parse and execute natively alongside standard QBASIC and GW-BASIC statements.

---

## 2. Color Attributes and Border Control

The ZX Spectrum graphical memory model separated character glyphs from color attributes. BASIC++ provides direct statements replicating this behavior across both console and graphical modes:

- **`BORDER color`**: Sets the display border color (0 to 7, corresponding to standard Sinclair color ordinals: Black, Blue, Red, Magenta, Green, Cyan, Yellow, White).
- **`INK color`**: Sets the active foreground drawing and text attribute color (0 to 7).
- **`PAPER color`**: Sets the active background drawing and text attribute color (0 to 7).
- **`BRIGHT flag`**: Toggles attribute high-intensity mode (`0` = normal, `1` = bright).
- **`FLASH flag`**: Toggles attribute flashing mode (`0` = steady, `1` = alternating foreground/background).
- **`INVERSE flag`**: Toggles video color inversion (`0` = standard, `1` = inverted).

---

## 3. Sound Generation (`BEEP`)

The Sinclair `BEEP` statement generates tone synthesis on the system sound generator:

```basic
BEEP duration, pitch
```

- **`duration`**: Floating-point duration of the tone in seconds.
- **`pitch`**: Signed semi-tone offset relative to Middle C (`0` = Middle C / 261.63 Hz, positive values ascend chromatically, negative values descend).

Implemented via the delayed-load audio HAL in `engine/src/audio/audio_synth.c`.

---

## 4. String Range Slicing

Sinclair BASIC introduced slice indexing using the `TO` operator. BASIC++ supports this syntax natively for string extraction and assignment:

- **`s$(start TO end)`**: Extracts substring from character `start` to `end` (1-based, inclusive).
- **`s$(start TO)`**: Extracts substring from character `start` to the end of the string.
- **`s$(TO end)`**: Extracts substring from beginning of string up to character `end`.

---

## 5. Example: Sinclair Color Attributes and Tone

```basic
10 REM Sinclair Attributes & Sound Demo
20 BORDER 1 : PAPER 7 : INK 2 : CLS
30 PRINT "ZX Spectrum Native Emulation"
40 BEEP 0.25, 0    ' Middle C
50 BEEP 0.25, 4    ' E above Middle C
60 BEEP 0.5, 7     ' G above Middle C
70 A$ = "BASIC++ LANGUAGE"
80 PRINT "Sliced: "; A$(1 TO 7)
```
