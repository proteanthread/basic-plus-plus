CREATING AND USING MEMORY MAPS
==============================
Version 4.2.3

BASIC++ includes a pluggable memory map system that emulates
the 64 KB address space of classic 8-bit and 16-bit computers.
This guide covers using the built-in maps, understanding their
layouts in register-level detail, writing your own custom maps,
and blueprints for adding machines not yet included.


TABLE OF CONTENTS
=================

  1.  What Is MEMMAP?
  2.  Quick Reference: Available Maps
  3.  Detailed Built-In Maps (Deep Dive)
      3.1   NONE (Bare 64K)
      3.2   MSDOS (MS-DOS / IBM PC)
      3.3   C64 (Commodore 64)
      3.4   C128 (Commodore 128)
      3.5   VIC20 (Commodore VIC-20)
      3.6   PLUS4 (Commodore Plus/4)
      3.7   PET (Commodore PET 2001/4032/8032)
      3.8   ATARI8 (Atari 400/800/XL/XE)
      3.9   APPLE2 (Apple II / IIe)
      3.10  TRS80 (TRS-80 Model I/III)
      3.11  SPECTRUM (ZX Spectrum 48K)
      3.12  QL (Sinclair QL)
  4.  Creating a Custom Memory Map (Step-by-Step)
  5.  Blueprints for Uncovered Machines
      5.1   MSX / MSX2
      5.2   BBC Micro (Model B)
      5.3   Amstrad CPC 464/6128
      5.4   Dragon 32/64
      5.5   TRS-80 Color Computer (CoCo)
      5.6   Oric-1 / Oric Atmos
      5.7   SAM Coupe
      5.8   Acorn Electron
      5.9   Camputers Lynx
      5.10  Jupiter Ace
      5.11  Mattel Aquarius
      5.12  Sharp MZ-700/MZ-800
      5.13  Sord M5
      5.14  Tandy 1000
      5.15  Amstrad PCW
  6.  Memory Map Interactions (PEEK/POKE/INP/OUT/DEF SEG)
  7.  Magic Addresses and Side Effects
  8.  Combining MEMMAP + DIALECT
  9.  Advanced Techniques
  10. Reference Data Sources
  11. Tips for Map Authors


=====================================================================
1. WHAT IS MEMMAP?
=====================================================================

When you run MEMMAP, BASIC++ clears its 64 KB virtual memory
and pre-fills it with the values a real machine would have at
cold boot: ROM signatures, hardware register defaults, screen
memory, I/O chip shadows, and system pointers.

After that, PEEK and POKE operate on this virtual memory,
INP and OUT use the same address space, and your BASIC
programs can interact with the "hardware" just like they
would on the real machine.

  MEMMAP "C64"
  PRINT PEEK(53280)       ' 14 (light blue border)
  POKE 53280, 0           ' Change border to black
  PRINT PEEK(53280)       ' 0

No actual hardware is involved.  It is a 64K byte array that
mimics the memory layout a real program would see.

Key points:
  - Memory is exactly 65536 bytes (0-65535, $0000-$FFFF).
  - The array is zeroed BEFORE platform presets are applied.
  - Only non-zero values need to be set by init functions.
  - MEMMAP does NOT clear BASIC variables or program lines.
  - MEMMAP does NOT affect file I/O (which uses VDev layer).
  - Multiple MEMMAP calls are allowed; each re-initializes.


=====================================================================
2. QUICK REFERENCE: AVAILABLE MAPS
=====================================================================

  Code       Platform                 CPU    Key Features
  ----       --------                 ---    ------------
  NONE       Bare 64K                 any    All zeros, no presets
  MSDOS      MS-DOS / IBM PC          8086   BIOS Data Area, IVT, 640K
  C64        Commodore 64             6510   VIC-II, SID, CIA, Color RAM
  C128       Commodore 128            8502   C64 base + MMU, BASIC 7.0
  VIC20      Commodore VIC-20         6502   VIC chip, 22-col screen
  PLUS4      Commodore Plus/4         7501   TED chip, BASIC 3.5
  PET        Commodore PET            6502   PIA, 40x25 screen at $8000
  ATARI8     Atari 400/800/XL/XE      6502   ANTIC, POKEY, GTIA, PIA
  APPLE2     Apple II / IIe           6502   Soft switches, text pages
  TRS80      TRS-80 Model I/III       Z80    Video at $3C00, keyboard map
  SPECTRUM   ZX Spectrum 48K          Z80    Bitmap screen, attributes
  QL         Sinclair QL              68008  QDOS system variables, Screen RAM

Commands:
  MEMMAP "C64"             Switch to Commodore 64 map
  MEMMAP "NONE"            Switch to bare memory
  MEMMAP LIST              Show all available maps


=====================================================================
3. DETAILED BUILT-IN MAPS (DEEP DIVE)
=====================================================================

Each section documents every address that gets pre-filled,
the chip it belongs to, and what classic BASIC programs
expect to find there.


---------------------------------------------------------------------
3.1  NONE (Bare 64K)
---------------------------------------------------------------------

  All 65536 bytes set to $00.  No presets, no magic addresses.
  Use this as a blank canvas for custom applications, game
  consoles, or embedded device emulations.

  Example:
    MEMMAP "NONE"
    FOR I = 0 TO 255
      POKE I, I            ' Fill zero page with 0..255
    NEXT
    PRINT PEEK(42)         ' 42


---------------------------------------------------------------------
3.2  MSDOS (MS-DOS / IBM PC)
---------------------------------------------------------------------

  Emulates the memory layout of an IBM PC/XT/AT running MS-DOS.
  The BIOS Data Area (BDA) at $0400-$04FF contains hardware
  state.  The Interrupt Vector Table (IVT) at $0000-$03FF
  holds 256 x 4-byte far pointers (all zero/stubs here).

  CPU: Intel 8086/8088 (real mode, 20-bit addressing)
  Video: MDA/CGA/EGA/VGA text mode
  RAM: 640K conventional

  BIOS DATA AREA ($0400-$04FF):
  ---------------------------------------------------------------
  Address       Bytes  Value     Description
  -------       -----  -----     -----------
  $0400-$0401   2      varies    COM1 base port (e.g. $03F8)
  $0402-$0403   2      varies    COM2 base port
  $0408-$0409   2      varies    LPT1 base port (e.g. $0378)
  $0410-$0411   2      $0021     Equipment word
                                   Bit 0: floppy present
                                   Bits 1-2: x87 coprocessor
                                   Bits 4-5: initial video mode
                                     01 = 40-col color
                                     10 = 80-col color
                                     11 = monochrome
                                   Bits 6-7: floppy count - 1
  $0413-$0414   2      $0280     Conventional memory size (KB)
                                   $0280 = 640
  $0417         1      $00       Keyboard flags byte 1
                                   Bit 0: Right Shift
                                   Bit 1: Left Shift
                                   Bit 2: Ctrl
                                   Bit 3: Alt
                                   Bit 4: Scroll Lock active
                                   Bit 5: Num Lock active
                                   Bit 6: Caps Lock active
                                   Bit 7: Insert active
  $0418         1      $00       Keyboard flags byte 2
  $041A-$041B   2      $001E     Keyboard buffer head pointer
  $041C-$041D   2      $001E     Keyboard buffer tail pointer
  $041E-$043D   32     $00       Keyboard buffer (16 words)
  $0449         1      $03       Current video mode
                                   $00 = 40x25 B/W text
                                   $01 = 40x25 color text
                                   $02 = 80x25 B/W text
                                   $03 = 80x25 color text
                                   $04 = 320x200 4-color CGA
                                   $06 = 640x200 2-color CGA
                                   $07 = 80x25 monochrome
                                   $0D = 320x200 16-color EGA
                                   $0E = 640x200 16-color EGA
                                   $10 = 640x350 16-color EGA
                                   $12 = 640x480 16-color VGA
                                   $13 = 320x200 256-color VGA
  $044A-$044B   2      $0050     Screen columns (80)
  $044C-$044D   2      $1000     Video regen buffer size
  $044E-$044F   2      $0000     Current page start offset
  $0450-$045F   16     $00       Cursor positions (8 pages x 2)
  $0460-$0461   2      $0607     Cursor shape (start/end line)
  $0462         1      $00       Active video page (0)
  $0463-$0464   2      $03D4     CRT controller base I/O port
                                   $03D4 = color adapter
                                   $03B4 = monochrome adapter
  $0465         1      $29       Current mode select register
  $0466         1      $30       Current color palette
  $046C-$046F   4      $00       Timer tick counter (DWORD)
                                   Increments 18.2 times/sec
  $0470         1      $00       Timer overflow flag
  $0472-$0473   2      $0000     Warm boot flag
                                   $1234 = skip memory test
  $0475         1      $01       Number of hard drives
  $0484         1      $18       Rows on screen - 1 (24)
  $0485-$0486   2      $0010     Character height (16 scan lines)

  INTERRUPT VECTOR TABLE ($0000-$03FF):
  ---------------------------------------------------------------
  All 256 entries are zero (stubs).  Key interrupts on a real PC:

  INT $00 ($0000): Divide by zero
  INT $08 ($0020): Timer tick (IRQ 0)
  INT $09 ($0024): Keyboard (IRQ 1)
  INT $10 ($0040): Video services (BIOS)
  INT $13 ($004C): Disk services (BIOS)
  INT $16 ($0058): Keyboard services (BIOS)
  INT $19 ($0064): Bootstrap loader
  INT $1A ($0068): Time-of-day (BIOS)
  INT $21 ($0084): DOS services

  VIDEO MEMORY:
  ---------------------------------------------------------------
  On a real PC, video memory is at segment B800:0000 (CGA/EGA/VGA
  color text) or B000:0000 (monochrome).  Use DEF SEG:

    DEF SEG = &HB800
    POKE 0, 65              ' 'A' at row 0, col 0
    POKE 1, 7               ' White on black attribute

  Attribute byte format:
    Bit 7:    Blink
    Bits 4-6: Background (0-7)
    Bits 0-3: Foreground (0-15)

    0 = Black    4 = Red       8 = Dark Gray   12 = Light Red
    1 = Blue     5 = Magenta   9 = Light Blue  13 = Lt Magenta
    2 = Green    6 = Brown     10 = Lt Green   14 = Yellow
    3 = Cyan     7 = White     11 = Lt Cyan    15 = Bright White

  USEFUL PEEKS AND POKES:
  ---------------------------------------------------------------
    PEEK(&H0413)            Read conventional memory (KB)
    PEEK(&H0449)            Read current video mode
    POKE &H0417, flags      Set keyboard flags
    PEEK(&H046C)            Read timer ticks since midnight
    POKE &H0449, mode       Change video mode register
    POKE &H044A, cols       Change screen columns


---------------------------------------------------------------------
3.3  C64 (Commodore 64)
---------------------------------------------------------------------

  The Commodore 64 has the richest memory map of any 8-bit
  computer, with the 6510 CPU's bank-switching port at $0000-$0001
  controlling which of 5 configurations of ROM/RAM/I/O is visible.

  CPU: MOS 6510 (modified 6502 with built-in I/O port)
  Video: VIC-II (MOS 6567/6569), 40 columns, 16 colors
  Audio: SID (MOS 6581/8580), 3 voices + filter
  I/O: CIA 1 (MOS 6526) - keyboard/joystick
       CIA 2 (MOS 6526) - serial bus/VIC bank/RS-232
  RAM: 64K

  CPU PORT ($0000-$0001):
  ---------------------------------------------------------------
  Address  Value  Description
  $0000    $2F    Data Direction Register
                    Bit 0: LORAM direction (1=output)
                    Bit 1: HIRAM direction (1=output)
                    Bit 2: CHAREN direction (1=output)
                    Bit 3: Cassette write (1=output)
                    Bit 4: Cassette sense (0=input)
                    Bit 5: Cassette motor (1=output)
  $0001    $37    Port Register
                    Bit 0: LORAM (1=BASIC ROM visible)
                    Bit 1: HIRAM (1=KERNAL ROM visible)
                    Bit 2: CHAREN (1=I/O visible, 0=CHAR ROM)
                    Bit 3: Cassette write line
                    Bit 4: Cassette button sense (1=no button)
                    Bit 5: Cassette motor (0=motor on)

  Bank configurations by bits 2-0 of $0001:
    $37 (111): BASIC ROM + I/O + KERNAL ROM (default)
    $36 (110): BASIC ROM + CHAR ROM + KERNAL ROM
    $35 (101): RAM + I/O + RAM
    $34 (100): RAM + RAM + RAM (all 64K RAM)
    $33 (011): BASIC ROM + I/O + KERNAL ROM (cart ROM too)
    $31 (001): RAM + I/O + RAM

  BASIC ZERO-PAGE POINTERS:
  ---------------------------------------------------------------
  Address       Value     Description
  $002B-$002C   $0801     TXTTAB: Start of BASIC program text
  $002D-$002E   $0803     VARTAB: Start of BASIC variables
  $002F-$0030   $0803     ARYTAB: Start of BASIC arrays
  $0031-$0032   $0803     STREND: End of BASIC arrays
  $0033-$0034   $A000     FRETOP: Bottom of string storage
  $0037-$0038   $A000     MEMSIZ: Top of BASIC memory
  $0039-$003A   varies    Current BASIC line number
  $003B-$003C   varies    Previous BASIC line number
  $0041-$0042   varies    DATA line number
  $0043-$0044   varies    DATA pointer

  SYSTEM VARIABLES:
  ---------------------------------------------------------------
  $00BA         $08       Current device number
                            0 = Keyboard
                            1 = Cassette
                            2 = RS-232
                            3 = Screen
                            4-5 = Printer
                            8-15 = Disk drives
  $0286         $0E       Current cursor color (light blue)
  $028D         $00       Shift key flag
  $0291         $00       Shift-Commodore toggle flag
  $C6           $00       Keyboard buffer count (0 = empty)
  $0277-$0280   ---       Keyboard buffer (10 bytes)

  SCREEN MEMORY ($0400-$07E7):
  ---------------------------------------------------------------
  1000 bytes = 40 columns x 25 rows, filled with $20 (space)
  Screen codes (not PETSCII): $01='A', $02='B', etc.

  To put character at row R, column C:
    POKE 1024 + R * 40 + C, screencode

  COLOR RAM ($D800-$DBE7):
  ---------------------------------------------------------------
  1000 nybbles, one per screen character.  Default: $0E (lt blue)
  Only the low nybble is significant (0-15).

    POKE 55296 + R * 40 + C, color

  C64 color codes:
    0 = Black       4 = Purple      8 = Orange      12 = Med Gray
    1 = White       5 = Green       9 = Brown       13 = Lt Green
    2 = Red         6 = Blue        10 = Lt Red     14 = Lt Blue
    3 = Cyan        7 = Yellow      11 = Dk Gray    15 = Lt Gray

  VIC-II REGISTERS ($D000-$D02E):
  ---------------------------------------------------------------
  Address  Value  Register  Description
  $D000    $00    SP0X      Sprite 0 X position
  $D001    $00    SP0Y      Sprite 0 Y position
  $D002-$D00F     SP1-SP7   Sprite 1-7 X/Y pairs
  $D010    $00    MSIGX     Sprites 0-7 X MSB (bit 8)
  $D011    $1B    SCROLY    Screen control register 1
                              Bit 7: RST8 (raster bit 8)
                              Bit 6: ECM (extended color mode)
                              Bit 5: BMM (bitmap mode)
                              Bit 4: DEN (display enable)
                              Bit 3: RSEL (row select, 24/25)
                              Bits 0-2: YSCROLL (fine scroll Y)
  $D012    $00    RASTER    Raster line counter
  $D015    $00    SPENA     Sprite enable register
  $D016    $C8    SCROLX    Screen control register 2
                              Bit 4: MCM (multi-color mode)
                              Bit 3: CSEL (column select, 38/40)
                              Bits 0-2: XSCROLL (fine scroll X)
  $D017    $00    YXPAND    Sprite Y expansion
  $D018    $15    VMCSB     VIC memory control
                              Bits 4-7: Screen memory base
                                        (x 1024, within VIC bank)
                              Bits 1-3: Character set base
                                        (x 2048, within VIC bank)
  $D019    $00    VICIRQ    Interrupt register
  $D01A    $00    IRQMASK   Interrupt enable mask
  $D01B    $00    SPBGPR    Sprite-background priority
  $D01C    $00    SPMC      Sprite multi-color enable
  $D01D    $00    XXPAND    Sprite X expansion
  $D01E    $00    SSPRCO    Sprite-sprite collision
  $D01F    $00    SSBGCO    Sprite-background collision
  $D020    $0E    EXTCOL    Border color (light blue)
  $D021    $06    BGCOL0    Background color 0 (blue)
  $D022    $00    BGCOL1    Background color 1
  $D023    $00    BGCOL2    Background color 2
  $D024    $00    BGCOL3    Background color 3
  $D025    $00    SPMC0     Sprite multi-color 0
  $D026    $00    SPMC1     Sprite multi-color 1
  $D027    $01    SP0COL    Sprite 0 color (white)
  $D028-$D02E     SP1-SP7   Sprite 1-7 colors

  SID REGISTERS ($D400-$D41C):
  ---------------------------------------------------------------
  All SID registers boot at $00 (silence).  Key registers:

  $D400-$D401   Voice 1 frequency (16-bit, lo/hi)
  $D402-$D403   Voice 1 pulse width
  $D404         Voice 1 control register
                  Bit 0: Gate (1=start, 0=release)
                  Bit 1: Sync
                  Bit 2: Ring modulation
                  Bit 3: Test bit
                  Bit 4: Triangle waveform
                  Bit 5: Sawtooth waveform
                  Bit 6: Pulse waveform
                  Bit 7: Noise waveform
  $D405         Voice 1 Attack/Decay
                  Bits 4-7: Attack (0-15)
                  Bits 0-3: Decay (0-15)
  $D406         Voice 1 Sustain/Release
                  Bits 4-7: Sustain (0-15)
                  Bits 0-3: Release (0-15)
  $D407-$D40D   Voice 2 (same layout as Voice 1)
  $D40E-$D414   Voice 3 (same layout as Voice 1)
  $D415         Filter cutoff frequency low (bits 0-2)
  $D416         Filter cutoff frequency high (bits 0-7)
  $D417         Filter resonance/routing
                  Bits 4-7: Resonance (0-15)
                  Bit 0: Filter voice 1
                  Bit 1: Filter voice 2
                  Bit 2: Filter voice 3
                  Bit 3: Filter external input
  $D418         Volume and filter mode
                  Bits 4-7: Filter mode
                    Bit 4: Low-pass
                    Bit 5: Band-pass
                    Bit 6: High-pass
                    Bit 7: Mute voice 3
                  Bits 0-3: Master volume (0-15)
  $D419         Paddle X (read-only analog)
  $D41A         Paddle Y (read-only analog)
  $D41B         Voice 3 oscillator (read-only)
  $D41C         Voice 3 envelope (read-only)

  CIA 1 ($DC00-$DC0F):
  ---------------------------------------------------------------
  $DC00    $7F    PRA    Port A data (keyboard column output)
  $DC01    $FF    PRB    Port B data (keyboard row input)
  $DC02    $FF    DDRA   Data direction A (output)
  $DC03    $00    DDRB   Data direction B (input)
  $DC04-$DC05     Timer A (running, default varies)
  $DC06-$DC07     Timer B
  $DC08-$DC0B     Time-of-Day clock (BCD: 10ths, sec, min, hr)
  $DC0D    $00    ICR    Interrupt control register
  $DC0E    $00    CRA    Control register A
  $DC0F    $00    CRB    Control register B

  Reading keyboard: write column mask to $DC00, read rows at $DC01
  Reading joystick port 2: read $DC00 directly
    Bit 0: Up     (0 = pressed)
    Bit 1: Down
    Bit 2: Left
    Bit 3: Right
    Bit 4: Fire button

  CIA 2 ($DD00-$DD0F):
  ---------------------------------------------------------------
  $DD00    $97    PRA    Port A data
                           Bits 0-1: VIC bank select
                             00 = Bank 3: $C000-$FFFF
                             01 = Bank 2: $8000-$BFFF
                             10 = Bank 1: $4000-$7FFF
                             11 = Bank 0: $0000-$3FFF (default)
                           Bit 2: RS-232 TXD
                           Bits 3-5: Serial bus (ATN, CLK, DATA)
                           Bits 6-7: Serial bus input

  ROM SIGNATURES:
  ---------------------------------------------------------------
  $A004-$A00B   "CBMBASIC"   BASIC ROM signature
  $FF80         $03          KERNAL revision 3

  HARDWARE VECTORS:
  ---------------------------------------------------------------
  $FFFA-$FFFB   NMI vector   (KERNAL default: $FE43)
  $FFFC-$FFFD   $FCE2        RESET vector (cold start)
  $FFFE-$FFFF   $FF48        IRQ vector

  CLASSIC C64 POKES:
  ---------------------------------------------------------------
    POKE 53280, color      Border color (0-15)
    POKE 53281, color      Background color (0-15)
    POKE 53272, 23         Switch to uppercase/graphics
    POKE 53272, 21         Switch to upper/lowercase
    POKE 808, 225          Disable RUN/STOP
    POKE 808, 237          Restore RUN/STOP
    POKE 646, color        Change cursor color
    POKE 53265, 11         Blank screen (speed up)
    POKE 53265, 27         Unblank screen
    POKE 788, 52           Disable RESTORE key
    POKE 1024+pos, char    Put character on screen
    POKE 55296+pos, col    Set color at screen position
    PEEK(56320)            Read joystick port 2
    PEEK(197)              Last key pressed (matrix code)
    PEEK(653)              Shift/CTRL/Commodore key status

  SID SOUND EXAMPLE:
  ---------------------------------------------------------------
    POKE 54296, 15         ' Master volume max
    POKE 54277, 9          ' Attack=0, Decay=9
    POKE 54278, 0          ' Sustain=0, Release=0
    POKE 54273, 28         ' Frequency high byte
    POKE 54272, 49         ' Frequency low byte
    POKE 54276, 33         ' Gate on, Triangle wave


---------------------------------------------------------------------
3.4  C128 (Commodore 128)
---------------------------------------------------------------------

  The C128 inherits ALL C64 memory map values, then adds:

  CPU: MOS 8502 (enhanced 6510), also has Z80 for CP/M
  Video: VIC-IIe (enhanced VIC-II) + VDC (80-column)
  RAM: 128K (two 64K banks)

  MMU ($FF00-$FF04):
  ---------------------------------------------------------------
  $FF00    $3E    Configuration register
                    Bit 0: I/O block select
                    Bit 1: ROM low select
                    Bit 2: ROM mid select
                    Bit 3: ROM high select
                    Bits 4-5: RAM bank (0-3)
                    Bit 6: Common RAM size
  $FF01    ---    Load configuration register
  $FF02    ---    Configuration register preset A
  $FF03    ---    Configuration register preset B
  $FF04    ---    Mode configuration register

  ADDITIONAL REGISTERS:
  ---------------------------------------------------------------
  $FF80    $07    KERNAL version 7 (BASIC 7.0)
  $0033-$0034     $FF00 (extended string space for 128K)

  VDC 80-COLUMN CHIP (accessed via $D600-$D601):
  ---------------------------------------------------------------
  The VDC is register-indirect: write register number to $D600,
  read/write data at $D601.

  VDC Register 0: Horizontal total chars
  VDC Register 1: Horizontal displayed
  VDC Register 6: Vertical displayed rows
  VDC Register 12-13: Display start address (hi/lo)
  VDC Register 24: Block/reverse screen fill
  VDC Register 26: Color (foreground/background)
  VDC Register 28: Character set address
  VDC Register 31: Data register (read/write VDC RAM)


---------------------------------------------------------------------
3.5  VIC20 (Commodore VIC-20)
---------------------------------------------------------------------

  CPU: MOS 6502
  Video: VIC chip (MOS 6560/6561), 22x23 characters
  Audio: Built into VIC chip, 4 voices
  I/O: VIA 1 (6522) + VIA 2 (6522)
  RAM: 5K unexpanded (max 32K with cartridges)

  CPU PORT:
  ---------------------------------------------------------------
  $0000    $2F    Data Direction Register
  $0001    $37    Bank configuration

  BASIC POINTERS:
  ---------------------------------------------------------------
  $002B-$002C   $1001     Start of BASIC text
  $0037-$0038   $1E00     Top of BASIC memory
  $002D-$002E   varies    Start of variables
  $0033-$0034   varies    Bottom of strings

  SCREEN MEMORY ($1E00-$1FF9):
  ---------------------------------------------------------------
  506 bytes (22 columns x 23 rows), filled with $20 (space)
  Note: VIC-20 screen RAM location depends on memory expansion:
    Unexpanded: $1E00
    3K expanded: $1E00
    8K+ expanded: $1000

  COLOR MEMORY ($9600-$97F9):
  ---------------------------------------------------------------
  506 nybbles, default $01 (white)

  VIC CHIP REGISTERS ($9000-$900F):
  ---------------------------------------------------------------
  $9000    $0C    Screen origin X (horizontal)
  $9001    $26    Screen origin Y (vertical)
  $9002    $96    Number of video columns
                    Bits 0-6: columns (22)
                    Bit 7: screen memory address bit 9
  $9003    $2E    Number of video rows
                    Bit 0: raster bit 0
                    Bits 1-6: rows x 2 (46 = 23 rows)
                    Bit 7: character size (8x8 or 8x16)
  $9004    ---    Current raster line (bits 8-1)
  $9005    $F0    Character/video address
                    Bits 4-7: character memory (x 1024)
                    Bits 0-3: video memory (x 512)
  $9006-$9007     Light pen X/Y
  $9008-$9009     Paddle X/Y
  $900A    $00    Voice 1 frequency (bass)
  $900B    $00    Voice 2 frequency (alto)
  $900C    $00    Voice 3 frequency (soprano)
  $900D    $00    Noise frequency
  $900E    $00    Auxiliary color + volume
                    Bits 4-7: Auxiliary color
                    Bits 0-3: Volume (0-15)
  $900F    $1B    Screen/border color
                    Bits 4-7: Background color
                    Bits 0-2: Border color
                    Bit 3: Inverse mode (1=inverse)

  VIA REGISTERS ($9110-$912F):
  ---------------------------------------------------------------
  VIA 1 ($9110-$911F):
    $9110    Port B (joystick + light pen)
    $9111    Port A (serial bus)
    $9112    Data direction B
    $9113    Data direction A
    $9114-$9115  Timer 1
    $9116-$9117  Timer 2
  VIA 2 ($9120-$912F):
    $9120    Port B (keyboard column)
    $9121    Port A (keyboard row)

  KERNAL:
  ---------------------------------------------------------------
  $FF80    $02    KERNAL revision 2
  $FFFC-$FFFD    $FD22    RESET vector

  VIC-20 COLOR CODES (same as C64):
  ---------------------------------------------------------------
    0=Black 1=White 2=Red 3=Cyan 4=Purple 5=Green 6=Blue 7=Yellow
    8-15: Same colors with multi-color flag


---------------------------------------------------------------------
3.6  PLUS4 (Commodore Plus/4)
---------------------------------------------------------------------

  CPU: MOS 7501/8501
  Video/Audio: TED (MOS 7360), 40 columns, 121 colors, 2 voices
  RAM: 64K
  ROM: BASIC 3.5 (16K) + KERNAL (16K) + built-in software

  CPU PORT:
  ---------------------------------------------------------------
  $0000    $2F    Data Direction Register
  $0001    $37    Bank configuration

  BASIC POINTERS:
  ---------------------------------------------------------------
  $002B-$002C   $1001     Start of BASIC text
  $0037-$0038   $8000     Top of BASIC memory

  SCREEN MEMORY ($0C00-$0FE7):
  ---------------------------------------------------------------
  1000 bytes (40 x 25), filled with $20

  TED REGISTERS ($FF00-$FF3E):
  ---------------------------------------------------------------
  $FF06    $00    Horizontal position / control
                    Bits 0-2: Fine horizontal scroll
                    Bit 3: Column select (38/40)
                    Bit 4: Multi-color mode
                    Bit 5: Freeze
                    Bit 6: Enhanced color mode (ECM)
                    Bit 7: Test
  $FF07    $00    Vertical position / control
                    Bits 0-2: Fine vertical scroll
                    Bit 3: Row select (24/25)
                    Bit 4: Screen blank
                    Bit 5: Bitmap mode
                    Bit 6: Reverse mode
                    Bit 7: PAL/NTSC select
  $FF08    ---    Keyboard latch
  $FF09    ---    IRQ status
  $FF0A    ---    IRQ enable mask
  $FF0B    ---    Raster counter high
  $FF0C-$FF0D    Cursor position
  $FF0E-$FF0F    Voice 1 frequency (10-bit)
  $FF10-$FF11    Voice 2 frequency (10-bit)
  $FF12    ---    Voice control
  $FF13    ---    Character set / video address
  $FF14    ---    Video base address
  $FF15    $00    Background color 0
  $FF16    $00    Background color 1
  $FF17    ---    Background color 2
  $FF18    ---    Background color 3
  $FF19    $00    Border color
  $FF1A    ---    Character position high
  $FF1B    ---    Character position low
  $FF1C    ---    Cursor blink
  $FF1D-$FF1E    Raster line counter
  $FF1F    ---    Attribute / flash control
  $FF3E    ---    ROM select
  $FF3F    ---    RAM select

  TED COLOR ENCODING:
  ---------------------------------------------------------------
  The TED uses a luminance + hue encoding.  121 unique colors:
    High nybble = hue (0-15, 0=black)
    Low nybble  = luminance (0-7)
  Example: $71 = blue, luminance 1

  $FF80    $35    BASIC 3.5 signature
  $FFFC-$FFFD    $E000    RESET vector


---------------------------------------------------------------------
3.7  PET (Commodore PET 2001/4032/8032)
---------------------------------------------------------------------

  CPU: MOS 6502
  Video: Built-in CRTC (6845 on later models), 40x25 or 80x25
  Audio: Single speaker (via VIA)
  I/O: PIA 6520 x 2, VIA 6522
  RAM: 8K-96K depending on model

  BASIC POINTERS:
  ---------------------------------------------------------------
  $0028-$0029   $0401     Start of BASIC text
  $0034-$0035   $8000     Top of memory

  SCREEN MEMORY ($8000-$83E7):
  ---------------------------------------------------------------
  1000 bytes (40 x 25), filled with $20 (space)
  8032 model uses $8000-$87CF (2000 bytes, 80x25)

  PIA REGISTERS:
  ---------------------------------------------------------------
  $E810    $FF    PIA 1 Port A (keyboard row)
  $E811    $FF    PIA 1 Control Register A
  $E812    $FF    PIA 1 Port B (keyboard column)
  $E813    $FF    PIA 1 Control Register B
  $E820    $FF    PIA 2 Port A (IEEE-488 data)
  $E821    $FF    PIA 2 Control Register A
  $E822    $FF    PIA 2 Port B (IEEE-488 control)
  $E823    $FF    PIA 2 Control Register B

  VIA ($E840-$E84F):
  ---------------------------------------------------------------
  $E840    Port B (cassette, screen blank, speaker)
  $E84B    Auxiliary control (shift register, timers)

  ROM:
  ---------------------------------------------------------------
  $FF80    $04    BASIC 4.0 marker
  $FFFC-$FFFD    $FD16    RESET vector

  PET MEMORY MAP OVERVIEW:
  ---------------------------------------------------------------
    $0000-$00FF: Zero page
    $0100-$01FF: CPU stack
    $0200-$03FF: System variables
    $0400-$7FFF: BASIC RAM (max 32K on PET 4032)
    $8000-$83E7: Screen RAM
    $8800-$8FFF: ROM expansion
    $9000-$AFFF: Spare ROM space
    $B000-$DFFF: BASIC 4.0 ROM (12K)
    $E000-$E7FF: Editor ROM (2K)
    $E800-$EFFF: I/O chips
    $F000-$FFFF: KERNAL ROM (4K)


---------------------------------------------------------------------
3.8  ATARI8 (Atari 400/800/XL/XE)
---------------------------------------------------------------------

  CPU: MOS 6502 (6502C in XL/XE)
  Video: ANTIC (display list processor) + GTIA (color/player)
  Audio: POKEY (4 channels, also handles keyboard + serial)
  I/O: PIA (6520 - joystick ports + bank select)
  RAM: 16K-128K (320K with upgrades)

  ZERO PAGE OS VARIABLES:
  ---------------------------------------------------------------
  $0010-$0011     DOSVEC: DOS entry point
  $0012-$0013     DOSINI: DOS init address
  $0040-$0041     WARMST: Warm start flag
  $0044-$0045     MEMTOP: Top of available RAM
  $0052          LMARGN: Left margin
  $0053          RMARGN: Right margin
  $0054          ROWCRS: Cursor row
  $0055-$0056    COLCRS: Cursor column
  $0058-$0059    $9C40   SAVMSC: Screen memory address

  BASIC ZERO PAGE:
  ---------------------------------------------------------------
  $0080-$0081    $A000   LOMEM: Lowest address for variables
  $0082-$0083    ---     VNTP: Variable name table pointer
  $0084-$0085    ---     VVTP: Variable value table pointer
  $0086-$0087    ---     STMTAB: Statement table pointer
  $0088-$0089    ---     STMCUR: Current statement pointer
  $008A-$008B    ---     STARP: String/array table pointer
  $008C-$008D    ---     RUNSTK: Runtime stack pointer
  $0090-$0091    $BC1F   MEMTOP: Highest address for BASIC

  COLOR REGISTER SHADOWS ($02C0-$02C8):
  ---------------------------------------------------------------
  $02C0    PCOLR0: Player 0 color
  $02C1    PCOLR1: Player 1 color
  $02C2    PCOLR2: Player 2 color
  $02C3    PCOLR3: Player 3 color
  $02C4    $28     COLPF0: Playfield color 0
  $02C5    $CA     COLPF1: Playfield color 1
  $02C6    $94     COLPF2: Playfield color 2 (text background)
  $02C7    $46     COLPF3: Playfield color 3
  $02C8    $00     COLBK:  Background color (border)

  Atari color encoding: High nybble = hue (0-15),
                         Low nybble = luminance (even: 0,2,4..14)

  ANTIC REGISTERS ($D400-$D40F):
  ---------------------------------------------------------------
  $D400    $22     DMACTL: DMA control
                     Bit 0: Narrow playfield (128 color clocks)
                     Bit 1: Normal playfield (160 color clocks)
                     Bit 2: Wide playfield (192 color clocks)
                     Bit 3: Missile DMA enable
                     Bit 4: Player DMA enable
                     Bit 5: Instruction DMA enable
  $D401    $02     CHACTL: Character control
                     Bit 0: Video inverse
                     Bit 1: Video blank
  $D402-$D403      $9C20  DLISTL/DLISTH: Display list address
  $D404-$D405      ---    HSCROL/VSCROL: Fine scroll
  $D406    ---     PMBASE: Player-missile base address / 256
  $D407    ---     CHBASE: Character set base / 256
  $D409    ---     NMIST: NMI status
  $D40A    ---     NMIRES: NMI reset
  $D40E    ---     NMIEN: NMI enable
  $D40F    ---     WSYNC: Wait for horizontal sync

  ANTIC DISPLAY LIST MODES:
  ---------------------------------------------------------------
    Mode 2:  40 chars, 1.5 color (GR.0 text, default)
    Mode 4:  40 chars, 4 colors (GR.12 multicolor text)
    Mode 6:  20 chars, 5 colors (GR.1 large text)
    Mode 7:  20 chars, 5 colors (GR.2 larger text)
    Mode 8:  40 pixels, 4 colors (GR.3)
    Mode 9:  80 pixels, 2 colors (GR.4)
    Mode 10: 80 pixels, 4 colors (GR.5)
    Mode 11: 160 pixels, 2 colors (GR.6)
    Mode 12: 160 pixels, 2 colors (GR.14, double-height)
    Mode 13: 160 pixels, 4 colors (GR.7)
    Mode 14: 160 pixels, 4 colors (GR.15, double-height)
    Mode 15: 320 pixels, 2 colors (GR.8, hi-res)

  GTIA REGISTERS ($D000-$D01F):
  ---------------------------------------------------------------
  $D000-$D003     HPOSP0-3: Player horizontal positions
  $D004-$D007     HPOSM0-3: Missile horizontal positions
  $D008-$D00B     SIZEP0-3: Player widths (0,1,2,3 = 1x,2x,1x,4x)
  $D00C           SIZEM: Missile widths
  $D00D-$D010     GRAFP0-3: Player graphics data
  $D011           GRAFM: Missile graphics data
  $D012-$D015     COLPM0-3: Player/missile colors
  $D016-$D019     COLPF0-3: Playfield colors (hardware)
  $D01A    $02    PRIOR: Priority/GTIA mode
                     Bits 0-3: Player/playfield priority
                     Bit 4: 5th player enable
                     Bit 5: Multi-color player enable
                     Bits 6-7: GTIA mode select
                       00 = Normal
                       01 = 16 shade mode (GR.9)
                       10 = 16 hue mode (GR.10)
                       11 = 16 hue+shade mode (GR.11)
  $D01B    $00    COLBK: Background color (hardware)
  $D01F    $00    CONSOL: Console keys / speaker

  POKEY REGISTERS ($D200-$D20F):
  ---------------------------------------------------------------
  $D200    AUDF1: Audio frequency 1
  $D201    AUDC1: Audio control 1
                    Bits 4-7: Volume (or dist control)
                    Bit 3: Volume only mode
                    Bits 0-2: Distortion (noise type)
  $D202    AUDF2: Audio frequency 2
  $D203    AUDC2: Audio control 2
  $D204    AUDF3: Audio frequency 3
  $D205    AUDC3: Audio control 3
  $D206    AUDF4: Audio frequency 4
  $D207    AUDC4: Audio control 4
  $D208    AUDCTL: Audio control
                    Bit 0: 17-bit poly (vs 9-bit)
                    Bit 1: Clock ch1 at 1.79 MHz
                    Bit 2: Clock ch3 at 1.79 MHz
                    Bit 3: Join ch1+ch2 (16-bit)
                    Bit 4: Join ch3+ch4 (16-bit)
                    Bit 5: Clock ch2 from ch1
                    Bit 6: Clock ch4 from ch3
                    Bit 7: 9-bit poly (vs 17-bit)
  $D209    STIMER: Start timers
  $D20A    RANDOM: Random number generator
  $D20D    SERIN: Serial input register
  $D20E    $FF    IRQEN: IRQ enable
  $D20F    $03    SKSTAT: Serial port status

  PIA ($D300-$D303):
  ---------------------------------------------------------------
  $D300    $FF    PORTA: Joystick port A
                    Bit 0: Stick 0 up    (0=pressed)
                    Bit 1: Stick 0 down
                    Bit 2: Stick 0 left
                    Bit 3: Stick 0 right
                    Bit 4: Stick 1 up
                    Bit 5: Stick 1 down
                    Bit 6: Stick 1 left
                    Bit 7: Stick 1 right
  $D301    $FF    PORTB: Memory bank select (XL/XE)
                    Bit 0: OS ROM (0=enabled)
                    Bit 1: BASIC ROM (0=enabled)
                    Bit 2: Self-test ROM
                    Bits 3-5: Extended RAM bank
  $D302    $3C    PACTL: Port A control
  $D303    $3C    PBCTL: Port B control

  SCREEN MEMORY ($9C40-$9FFF):
  ---------------------------------------------------------------
  960 bytes (40 x 24), filled with $00 (space in ATASCII)

  ROM:
  ---------------------------------------------------------------
  $FFF7    $02    OS version (02 = XL/XE)
  $FFFC-$FFFD    $E477  RESET vector

  CLASSIC ATARI POKES:
  ---------------------------------------------------------------
    POKE 710, color        Playfield 2 / text background
    POKE 712, color        Background/border
    POKE 559, 0            DMA off (dramatic speed increase)
    POKE 559, 34           DMA on (normal)
    POKE 82, 0             Left margin
    POKE 83, 39            Right margin
    POKE 764, 255          Clear keyboard buffer
    POKE 106, PEEK(106)-8  Reserve memory for P/M graphics
    POKE 53248, hpos       Move player 0 horizontally


---------------------------------------------------------------------
3.9  APPLE2 (Apple II / IIe)
---------------------------------------------------------------------

  CPU: MOS 6502 (65C02 in IIe Enhanced)
  Video: Custom Apple video circuitry
         Text: 40x24, HGR: 280x192, DHGR: 560x192 (IIe)
  Audio: 1-bit speaker toggle
  I/O: Memory-mapped soft switches, no I/O chips per se
  RAM: 48K-128K (auxiliary bank on IIe)

  BASIC ZERO-PAGE POINTERS:
  ---------------------------------------------------------------
  $0067-$0068   $0801     TXTTAB: Start of Applesoft program
  $0069-$006A   $0803     VARTAB: Start of variables
  $006B-$006C   $0803     ARYTAB: Start of arrays
  $006D-$006E   $0803     STREND: End of arrays
  $006F-$0070   varies    FRETOP: Bottom of string storage
  $0073-$0074   $9600     HIMEM: Highest available address
  $0076-$0077   varies    CURLIN: Current BASIC line number
  $0079-$007A   varies    OLDLIN: Previous BASIC line number
  $009D-$009E   varies    LINNUM: Parsed line number
  $00AF-$00B0   $9600     MEMSIZ: Top of string space

  TEXT PAGE 1 ($0400-$07FF):
  ---------------------------------------------------------------
  1024 bytes, filled with $A0 (space with high bit set)
  Apple II text uses ASCII with high bit set for normal video.
  Screen layout is NOT linear — uses interleaved row addressing:

    Row 0:  $0400-$0427 (columns 0-39)
    Row 1:  $0480-$04A7
    Row 2:  $0500-$0527
    Row 3:  $0580-$05A7
    Row 4:  $0600-$0627
    Row 5:  $0680-$06A7
    Row 6:  $0700-$0727
    Row 7:  $0780-$07A7
    Row 8:  $0428-$044F
    Row 9:  $04A8-$04CF
    Row 10: $0528-$054F
    Row 11: $05A8-$05CF
    Row 12: $0628-$064F
    Row 13: $06A8-$06CF
    Row 14: $0728-$074F
    Row 15: $07A8-$07CF
    Row 16: $0450-$0477
    Row 17: $04D0-$04F7
    Row 18: $0550-$0577
    Row 19: $05D0-$05F7
    Row 20: $0650-$0677
    Row 21: $06D0-$06F7
    Row 22: $0750-$0777
    Row 23: $07D0-$07F7

  Formula: base = $0400 + (row%8)*$80 + (row/8)*$28
           address = base + column

  TEXT PAGE 2 ($0800-$0BFF):
  ---------------------------------------------------------------
  Secondary text page, same interleaved layout.
  Activated by reading $C055 (Page 2).

  HGR PAGES:
  ---------------------------------------------------------------
  $2000-$3FFF: HGR page 1 (8K)
  $4000-$5FFF: HGR page 2 (8K)
  Also use interleaved row addressing.

  SOFT SWITCHES ($C000-$C0FF):
  ---------------------------------------------------------------
  Address  Value  Description
  $C000    $00    Keyboard data (read: last key, bit 7 = strobe)
  $C001    ---    80STORE off (IIe)
  $C002    ---    Read main memory (IIe)
  $C003    ---    Read aux memory (IIe)
  $C004    ---    Write main memory (IIe)
  $C005    ---    Write aux memory (IIe)
  $C006    ---    Select internal ROM (IIe)
  $C007    ---    Select slot ROM (IIe)
  $C008    ---    Main zero page/stack (IIe)
  $C009    ---    Aux zero page/stack (IIe)
  $C00C    ---    80-column mode off (IIe)
  $C00D    ---    80-column mode on (IIe)
  $C00E    ---    Alternate char set off (IIe)
  $C00F    ---    Alternate char set on (IIe)
  $C010    $00    Clear keyboard strobe (write any value)
  $C020    ---    Cassette output toggle
  $C030    $00    Speaker click (read to toggle)
  $C050    $00    Switch to text mode (read)
  $C051    ---    Switch to graphics mode
  $C052    ---    Full screen (no text window)
  $C053    ---    Mixed mode (4 lines text at bottom)
  $C054    $00    Display page 1 (read)
  $C055    ---    Display page 2
  $C056    ---    Low-res graphics
  $C057    ---    Hi-res graphics
  $C058-$C05F     Annunciator outputs (game I/O)
  $C060    ---    Cassette input
  $C061    ---    Open-Apple key / button 0
  $C062    ---    Closed-Apple key / button 1
  $C064    ---    Paddle 0 (analog)
  $C065    ---    Paddle 1
  $C070    ---    Trigger paddle timer

  SLOT I/O ($C0n0-$C0nF where n = slot 1-7):
  ---------------------------------------------------------------
  $C090-$C09F     Slot 1 (printer)
  $C0A0-$C0AF     Slot 2 (serial)
  $C0E0-$C0EF     Slot 6 (disk controller)

  ROM IDENTIFICATION:
  ---------------------------------------------------------------
  $FBB3    $06    Apple IIe identifier
  $FBC0    $EA    Apple IIe secondary ID
  $FF69    $4C    Monitor JMP instruction (entry point)
  $FFFC-$FFFD    $FA62    RESET vector

  Additional ROM IDs for model detection:
    $FBB3 = $38: Apple II (original)
    $FBB3 = $EA: Apple II Plus
    $FBB3 = $06: Apple IIe
    $FBC0 = $EA: Standard IIe
    $FBC0 = $E0: Enhanced IIe (65C02)

  CLASSIC APPLE II PEEKS/POKES:
  ---------------------------------------------------------------
    PEEK(49152)            Last keypress (bit 7 set = valid)
    POKE 49168, 0          Clear keyboard strobe
    PEEK(49200)            Toggle speaker (click)
    PEEK(49152) - 128      ASCII value of last key
    POKE 33, width         Set text window width
    POKE 34, top           Set text window top row
    POKE 35, bottom        Set text window bottom row
    POKE 36, col           Set cursor column
    POKE 37, row           Set cursor row
    CALL -936              Clear to end of screen (HOME)
    CALL -958              Clear line
    CALL -868              Clear to end of line


---------------------------------------------------------------------
3.10  TRS80 (TRS-80 Model I/III)
---------------------------------------------------------------------

  CPU: Zilog Z80 (1.77 MHz Model I, 2.03 MHz Model III)
  Video: Character-mapped, 64x16 (Model I) or 64x16 (Model III)
  Audio: None (cassette port can be used)
  I/O: Memory-mapped keyboard, direct video memory
  RAM: 16K-48K

  ROM ($0000-$2FFF):
  ---------------------------------------------------------------
  $0000    $F3    DI instruction (first ROM byte)
  $0001    $AF    XOR A (clear accumulator)
  $0062    $52    'R' for Radio Shack identification

  Model I ROM contains Level II BASIC (12K).
  Model III ROM is larger (14K) with additional routines.

  KEYBOARD MEMORY ($3800-$38FF):
  ---------------------------------------------------------------
  256 bytes, all $00 at boot (all keys released)
  The keyboard is memory-mapped in 8 rows:

    $3801: Row 0: @ A B C D E F G
    $3802: Row 1: H I J K L M N O
    $3804: Row 2: P Q R S T U V W
    $3808: Row 3: X Y Z
    $3810: Row 4: 0 1 2 3 4 5 6 7
    $3820: Row 5: 8 9 : ; , - . /
    $3840: Row 6: ENTER CLEAR BREAK UP DOWN LEFT RIGHT SPACE
    $3880: Row 7: SHIFT (right)

  Each bit in the byte represents one key in that row.
  Bit = 1 means key is pressed.

  VIDEO RAM ($3C00-$3FFF):
  ---------------------------------------------------------------
  1024 bytes (64 columns x 16 rows), filled with $20 (space)

  Position = $3C00 + row * 64 + col

  Character codes:
    $20-$7F: Standard ASCII printable
    $80-$BF: Block graphics characters (2x3 block mosaic)
      Each block graphic is 6 pixels arranged in a 2x3 grid:
        Bit 0: Top-left
        Bit 1: Top-right
        Bit 2: Middle-left
        Bit 3: Middle-right
        Bit 4: Bottom-left
        Bit 5: Bottom-right
      Add $80 to the bit pattern to get the character code.

  SYSTEM VARIABLES:
  ---------------------------------------------------------------
  $4000-$4001   $4200     Start of BASIC text
  $4002-$4003   $7FFF     Top of memory (32K system)

  I/O PORTS (via IN/OUT on Z80):
  ---------------------------------------------------------------
  Port $FF: Cassette / memory size
  Port $F8: Printer (TRS-80 Model I)

  CLASSIC TRS-80 POKES:
  ---------------------------------------------------------------
    POKE 15360 + pos, char     Screen character
    PEEK(14336 + row * 256)    Keyboard scan row
    POKE 16526, lo : POKE 16527, hi  Set USR() address


---------------------------------------------------------------------
3.11  SPECTRUM (ZX Spectrum 48K)
---------------------------------------------------------------------

  CPU: Zilog Z80A (3.5 MHz)
  Video: Custom ULA, 256x192 bitmap + 32x24 attributes
  Audio: 1-bit beeper (128K: AY-3-8912 also)
  I/O: ULA-based (keyboard, tape, border)
  RAM: 48K (128K in later models)

  ROM ($0000-$3FFF):
  ---------------------------------------------------------------
  $0000    $F3    DI instruction
  $0001    $AF    XOR A
  $0002    $11    LD DE,...
  
  The 16K ROM contains Spectrum BASIC, system routines, and
  the character set at $3D00-$3FFF.

  SCREEN BITMAP ($4000-$57FF):
  ---------------------------------------------------------------
  6144 bytes, cleared to $00 (all black/empty)

  The bitmap is organized in a complex interleaved layout:
  The screen is divided into 3 thirds (0,1,2), each 8 rows tall.
  Within each third, lines are interleaved.

  Address formula:
    addr = $4000 + (third * 2048) + (charrow * 256) +
           (pixline * 32) + charcol

  Where:
    third = row / 8          (0-2)
    charrow = row % 8        (0-7, character row within third)
    pixline = pixel line within character (0-7)
    charcol = column / 8     (0-31)

  ATTRIBUTE MEMORY ($5800-$5AFF):
  ---------------------------------------------------------------
  768 bytes (32 x 24), filled with $38

  Attribute format: FBPPPIII
    Bit 7: Flash (0=off, 1=flashing)
    Bit 6: Bright (0=normal, 1=bright)
    Bits 3-5: Paper color (0-7)
    Bits 0-2: Ink color (0-7)

  Spectrum color codes:
    0 = Black    2 = Red     4 = Green    6 = Yellow
    1 = Blue     3 = Magenta 5 = Cyan     7 = White

  With BRIGHT bit set, all colors become brighter versions.
  $38 = Paper 7 (white), Ink 0 (black), no flash, no bright

  SYSTEM VARIABLES ($5C00-$5CFF):
  ---------------------------------------------------------------
  Address       Value     Variable   Description
  $5C00         ---       KSTATE     Keyboard state (8 bytes)
  $5C08         ---       LAST-K     Last key pressed
  $5C09         ---       REPDEL     Key repeat delay
  $5C0A         ---       REPPER     Key repeat period
  $5C0B-$5C0C   ---       DEFADD    Address of DEF FN arg
  $5C0D         ---       K-DATA    Second byte of key code
  $5C0E-$5C0F   ---       TVDATA    Bytes for TV system
  $5C10-$5C12   ---       STRMS     Stream addresses
  $5C36-$5C37   ---       CHARS     Character set address - 256
  $5C38         ---       RASP      Rasp length (error buzz)
  $5C39         ---       PIP       Pip length (key click)
  $5C3A         ---       ERR_NR    Error number - 1
  $5C3B         ---       FLAGS     Various flags
  $5C3C         ---       TV_FLAG   TV flags
  $5C3D-$5C3E   ---       ERR_SP    Error stack pointer
  $5C3F-$5C40   ---       LIST_SP   List stack pointer
  $5C41         ---       MODE      Input mode (K/L/C/E/G)
  $5C44-$5C45   ---       SPL_LOC   Spare location
  $5C48         $01       BORDCR    Border color * 8 + border
  $5C49-$5C4A   ---       E_PPC     Current edit line
  $5C4B-$5C4C   $5CCB     VARS      Start of variables
  $5C4D-$5C4E   ---       DEST      Variable for assignment
  $5C4F-$5C50   ---       CHANS     Channel data area
  $5C51-$5C52   ---       CURCHL    Current channel
  $5C53-$5C54   $5CCB     PROG      Start of BASIC program
  $5C55-$5C56   ---       NXTLIN    Next line to execute
  $5C57-$5C58   ---       DATADD    DATA pointer
  $5C59-$5C5A   $5CCB     E_LINE    Edit line address
  $5C5B-$5C5C   ---       K_CUR     Cursor address
  $5C5D-$5C5E   ---       CH_ADD    Character address
  $5C5F-$5C60   ---       X_PTR     Error character address
  $5C61-$5C62   $FF57     STKEND    Calculator stack end
  $5C63-$5C64   ---       BREG      Calculator B register
  $5C65-$5C66   $FF58     STKBOT    Calculator stack bottom
  $5C67-$5C68   ---       MEM       Calculator memory area
  $5C69-$5C6A   ---       DF_SZ     Display file size
  $5C6B-$5C6C   ---       S_TOP     Top program line
  $5C6D-$5C6E   ---       OLDPPC    Old program counter
  $5C6F         ---       FLAGX     Various flags
  $5C70-$5C71   ---       STRLEN    String length
  $5C72-$5C73   ---       T_ADDR    Syntax table address
  $5C74-$5C75   ---       SEED      Random seed
  $5C76-$5C78   ---       FRAMES    Frame counter (3 bytes!)
  $5C79-$5C7A   ---       UDG       User-defined graphics addr
  $5C7B-$5C7C   ---       COORDS    X coordinate of PLOT
  $5C7D         ---       P_POSN    Printer column position
  $5C7E-$5C7F   ---       PR_CC     Print position
  $5C80-$5C81   ---       ECHO_E    Echo position
  $5C82-$5C83   ---       DF_CC     Display file position
  $5C84-$5C85   ---       DFCCL     Lower display file pos
  $5C86-$5C87   ---       S_POSN    Screen position
  $5C88-$5C89   ---       SPOSNL    Lower screen position
  $5C8A         ---       SCR_CT    Scroll count
  $5C8B         ---       ATTR_T    Current temp attribute
  $5C8C         ---       MASK_T    Current temp attribute mask
  $5C8D         $07       ATTR_P    Permanent attribute value
  $5C8E         ---       ATTR_MASK Attribute mask
  $5C8F         $38       MASK_P    Permanent mask
  $5C90-$5C91   ---       P_FLAG    Print flag / OVER / INVERSE

  I/O PORTS (via Z80 IN/OUT):
  ---------------------------------------------------------------
  Port $FE (ULA):
    Write:
      Bits 0-2: Border color (0-7)
      Bit 3:    MIC output (cassette)
      Bit 4:    EAR output / beeper
    Read (address low byte selects keyboard half-row):
      Bit 0-4: Key states (0=pressed)
      Bit 6:   EAR input (cassette)

  Keyboard half-rows (selected by address A8-A15):
    $FEFE: Shift, Z, X, C, V
    $FDFE: A, S, D, F, G
    $FBFE: Q, W, E, R, T
    $F7FE: 1, 2, 3, 4, 5
    $EFFE: 0, 9, 8, 7, 6
    $DFFE: P, O, I, U, Y
    $BFFE: Enter, L, K, J, H
    $7FFE: Space, Symbol, M, N, B

  HARDWARE VECTORS:
  ---------------------------------------------------------------
  $FFFC-$FFFD    $0000    RESET vector

  CLASSIC SPECTRUM POKES:
  ---------------------------------------------------------------
    POKE 23693, attr       Set permanent attributes
    POKE 23624, 0          Reset frame counter
    POKE 23609, 0          Disable key click
    POKE 23658, 8          Caps Lock on
    POKE 23756, bright     Set default bright mode
    OUT 254, border*8      Set border color (via ULA port)


---------------------------------------------------------------------
3.12  QL (Sinclair QL)
---------------------------------------------------------------------

  CPU: Motorola 68008 (7.5 MHz)
  Video: Custom ZX8301, 512x256 (4 colors) or 256x256 (8 colors)
  Audio: Custom ZX8302
  RAM: 128 KB base (expandable to 640 KB)

  Since Sinclair QL uses a Motorola 68008 CPU with flat addressing,
  BASIC++ maps key memory markers within the 64 KB address space:

  ROM VECTOR AREA ($0000-$0100):
  ---------------------------------------------------------------
  $0000-$0003   $0008    Reset Supervisor Stack Pointer
  $0004-$0007   $0030    Reset Program Counter

  QDOS SYSTEM VARIABLES ($0100-$015F):
  ---------------------------------------------------------------
  $0100         $02      SV.IDENT: QL type identifier
  $0102-$0103   $0001    SV.VERSN: QDOS version major/minor
  $0104-$0105   128      Available RAM size in KB
  $0110         $00      Display mode (0 = mode 4, 8 = mode 8)

  KEYBOARD & INPUT STATUS ($0140-$015F):
  ---------------------------------------------------------------
  $0140         $FF      IPC (8049) keyboard controller status
  $0160-$017F   $00      Microdrive control block area

  SCREEN RAM ($2000-$3FFF):
  ---------------------------------------------------------------
  $2000-$3FFF   $00      Screen display buffer (partial emulation)


=====================================================================
4. CREATING A CUSTOM MEMORY MAP (STEP-BY-STEP)
=====================================================================

To add a new platform to BASIC++, you modify two files:
memmap.h and memmap.c.  Follow these five steps exactly.


Step 1: Add the Enum (memmap.h)
-------------------------------

  In memmap.h, add your platform to MemMapType before
  MMAP_COUNT:

    typedef enum MemMapType {
        MMAP_NONE = 0,
        MMAP_MSDOS,
        MMAP_C64,
        /* ... existing entries ... */
        MMAP_SPECTRUM,
        MMAP_MSX,         /* <-- NEW */
        MMAP_COUNT
    } MemMapType;


Step 2: Write the Init Function (memmap.c)
-------------------------------------------

  Create a static initialization function in memmap.c:

    static void memmap_init_msx(unsigned char *mem)
    {
        /* Pre-fill MSX memory layout */
        mem_set(mem, 0xFFFF, 0x00);        /* Slot select */
        mem_fill(mem, 0x1800, 768, 0x20);  /* Name table */
        mem_fill(mem, 0x2000, 32, 0xF1);   /* Color table */
        mem_set16(mem, 0xF676, 0x8001);    /* TXTTAB */
        mem_set16(mem, 0xFC4A, 0xF380);    /* HIMEM */
        mem_set(mem, 0x002D, 0x01);        /* MSX version */
    }

  Helper functions available:
    mem_set(mem, address, value)      Set one byte
    mem_set16(mem, address, value)    Set 16-bit little-endian word
    mem_fill(mem, start, len, value)  Fill range with byte value

  Rules:
    A. The 64K array is zeroed BEFORE your function runs.
       Only set non-zero values.
    B. Focus on values BASIC programs actually PEEK for.
    C. Don't fill entire ROM ranges — just key identification
       bytes and entry points.


Step 3: Register in the Switch (memmap.c)
------------------------------------------

  In memmap_init(), add your case:

    case MMAP_MSX: memmap_init_msx(mem); break;


Step 4: Add to the Name Table (memmap.c)
-----------------------------------------

  In the memmap_table[] array:

    { "MSX", "MSX Standard", MMAP_MSX },


Step 5: Rebuild and Test
-------------------------

  Recompile BASIC++ and test:

    > MEMMAP "MSX"
    Memory map: MSX
    > PRINT PEEK(&H002D)
         1
    > MEMMAP LIST
    Available memory maps:
      NONE       Bare 64K, no presets
      MSDOS      MS-DOS (BIOS Data Area, 640K)
      ...
      MSX        MSX Standard


=====================================================================
5. BLUEPRINTS FOR UNCOVERED MACHINES
=====================================================================

The following sections provide complete blueprints for adding
platforms not currently built into BASIC++.  Each blueprint
includes the full memory map, recommended init values, and a
ready-to-paste code skeleton.


---------------------------------------------------------------------
5.1  MSX / MSX2
---------------------------------------------------------------------

  CPU: Zilog Z80A (3.58 MHz)
  Video: TMS9918A (MSX1) / V9938 (MSX2)
  Audio: AY-3-8910 (PSG, 3 channels + noise)
  I/O: PPI 8255A, slots
  RAM: 16K-128K (MSX1), 64K-512K (MSX2)

  MSX uses a SLOT-based architecture.  The 64K address space
  is divided into 4 x 16K pages.  Each page maps to one of
  4 primary slots (each optionally expanded to 4 sub-slots).

  DEFAULT SLOT LAYOUT (MSX1):
    Page 0 ($0000-$3FFF): Slot 0 - BIOS ROM
    Page 1 ($4000-$7FFF): Slot 0 - BASIC ROM (or cartridge)
    Page 2 ($8000-$BFFF): Slot 3 - RAM
    Page 3 ($C000-$FFFF): Slot 3 - RAM

  BIOS WORK AREA ($F380-$FFF7):
  ---------------------------------------------------------------
  $F380    HIMEM: Top of free RAM
  $F41F    FORCLR: Foreground color (15 = white)
  $F420    BAKCLR: Background color (4 = dark blue)
  $F421    BDRCLR: Border color (4 = dark blue)
  $F3AE    LINL40: Screen width for SCREEN 0 (39)
  $F3AF    LINL32: Screen width for SCREEN 1 (29)
  $F3B0    LINLEN: Current screen width
  $F3B1    CRTCNT: Number of rows (24)
  $F3DC    CLIKSW: Key click switch (1=on)
  $F3E9-$F3EA    EXPTBL: Slot expanded table
  $F676-$F677    TXTTAB: Start of BASIC text ($8001)
  $F6C2-$F6C3    VARTAB: Start of variables
  $F6C4-$F6C5    ARYTAB: Start of arrays
  $F6C6-$F6C7    STREND: End of arrays
  $FC4A-$FC4B    HIMEM: Top of BASIC memory ($F380)

  VDP I/O (accessed via I/O ports, not memory-mapped):
  ---------------------------------------------------------------
  Port $98: VDP data read/write
  Port $99: VDP register write (+ status read)

  VDP REGISTERS (TMS9918A):
    R0: Mode control 1
    R1: Mode control 2
        Bit 0: Magnified sprites
        Bit 1: 16x16 sprites (vs 8x8)
        Bit 3: Mode 2 (multicolor)
        Bit 4: Mode 1 (text)
        Bit 5: Interrupt enable
        Bit 6: Screen on/off
        Bit 7: 16K VRAM
    R2: Name table base address / 1024
    R3: Color table base address / 64
    R4: Pattern generator base / 2048
    R5: Sprite attribute table base / 128
    R6: Sprite pattern generator base / 2048
    R7: Text/border color

  VDP VRAM LAYOUT (SCREEN 1, MSX1):
    $0000-$07FF: Pattern generator table (2K)
    $1800-$1AFF: Name table (768 bytes, 32x24)
    $1B00-$1B7F: Sprite attribute table
    $2000-$201F: Color table (32 bytes, 8 chars per entry)
    $3800-$3FFF: Sprite pattern generator

  PSG (AY-3-8910, via I/O ports):
  ---------------------------------------------------------------
  Port $A0: Register select
  Port $A1: Data write
  Port $A2: Data read

  AY Registers:
    R0-R1:   Channel A frequency (12-bit)
    R2-R3:   Channel B frequency
    R4-R5:   Channel C frequency
    R6:      Noise frequency (5-bit)
    R7:      Mixer control
               Bits 0-2: Tone enable (A, B, C, 0=on)
               Bits 3-5: Noise enable (A, B, C, 0=on)
               Bit 6:    Port A direction
               Bit 7:    Port B direction
    R8:      Channel A amplitude (4-bit, or envelope)
    R9:      Channel B amplitude
    R10:     Channel C amplitude
    R11-R12: Envelope period (16-bit)
    R13:     Envelope shape (0-15)
    R14:     Port A data (joystick + cassette)
    R15:     Port B data

  PPI (8255A):
  ---------------------------------------------------------------
  Port $A8: Primary slot select register
               Bits 0-1: Page 0 slot ($0000-$3FFF)
               Bits 2-3: Page 1 slot ($4000-$7FFF)
               Bits 4-5: Page 2 slot ($8000-$BFFF)
               Bits 6-7: Page 3 slot ($C000-$FFFF)
  Port $A9: Keyboard matrix row input
  Port $AA: Keyboard column select + cassette + beep
  Port $AB: PPI mode register

  $FFFF: Secondary slot select register (if expanded)

  SLOT SELECT ($FFFF):
  ---------------------------------------------------------------
  $FFFF    $00    Read: complement of sub-slot register
                  Write: sub-slot select (if slot is expanded)

  MSX VERSION ($002D):
  ---------------------------------------------------------------
  $002D    $00    MSX version: 0=MSX1, 1=MSX2, 2=MSX2+, 3=turboR

  RECOMMENDED INIT CODE:
  ---------------------------------------------------------------
    static void memmap_init_msx(unsigned char *mem)
    {
        /* Slot select */
        mem_set(mem, 0xFFFF, 0x00);

        /* MSX BIOS hook addresses (RST vectors) */
        mem_set(mem, 0x0000, 0xC3);  /* JP RESET */
        mem_set(mem, 0x0008, 0xC3);  /* JP SYNCHR */
        mem_set(mem, 0x0010, 0xC3);  /* JP RDSLT */
        mem_set(mem, 0x0018, 0xC3);  /* JP CALLF */
        mem_set(mem, 0x0020, 0xC3);  /* JP DCOMPR */
        mem_set(mem, 0x0030, 0xC3);  /* JP CALLF */

        /* BIOS identification */
        mem_set(mem, 0x002B, 0x00);  /* MSX BIOS flag */
        mem_set(mem, 0x002D, 0x00);  /* MSX1 version */

        /* VDP data port addresses */
        mem_set(mem, 0x0006, 0x98);  /* VDP data port */
        mem_set(mem, 0x0007, 0x99);  /* VDP register port */

        /* VRAM: Name table spaces (SCREEN 1) */
        mem_fill(mem, 0x1800, 768, 0x20);

        /* VRAM: Color table - white on dark blue */
        mem_fill(mem, 0x2000, 32, 0xF4);

        /* BASIC work area */
        mem_set16(mem, 0xF676, 0x8001);  /* TXTTAB */
        mem_set16(mem, 0xFC4A, 0xF380);  /* HIMEM */

        /* Screen settings */
        mem_set(mem, 0xF3AE, 39);        /* LINL40 */
        mem_set(mem, 0xF3AF, 29);        /* LINL32 */
        mem_set(mem, 0xF3B0, 39);        /* LINLEN */
        mem_set(mem, 0xF3B1, 24);        /* CRTCNT */
        mem_set(mem, 0xF41F, 15);        /* FORCLR white */
        mem_set(mem, 0xF420, 4);         /* BAKCLR dark blue */
        mem_set(mem, 0xF421, 4);         /* BDRCLR dark blue */
        mem_set(mem, 0xF3DC, 1);         /* CLIKSW key click */
    }


---------------------------------------------------------------------
5.2  BBC Micro (Model B)
---------------------------------------------------------------------

  CPU: MOS 6502A (2 MHz)
  Video: Motorola 6845 CRTC + custom ULA, 8 display modes
  Audio: TI SN76489 (3 tone + 1 noise channel)
  I/O: VIA 6522 x 2 (System VIA + User VIA)
  RAM: 32K + 20K video RAM (sideways ROM banks)
  OS: Acorn MOS (Machine Operating System)

  MEMORY LAYOUT:
  ---------------------------------------------------------------
    $0000-$00FF: Zero page (OS + BASIC workspace)
    $0100-$01FF: CPU stack
    $0200-$02FF: OS workspace
    $0300-$03FF: VDU workspace + OS vectors
    $0400-$07FF: BASIC workspace
    $0800-$7FFF: User RAM (31K max, less in hi-res modes)
    $8000-$BFFF: Sideways ROM/RAM (paged, 16 slots)
    $C000-$DFFF: OS ROM (8K)
    $FC00-$FCFF: FRED: 1 MHz bus devices
    $FD00-$FDFF: JIM: 1 MHz bus paged memory
    $FE00-$FEFF: SHEILA: Hardware registers
    $FF00-$FFFF: OS ROM (continued)

  VIDEO MODES:
    Mode 0: 640x256, 2 colors, 20K RAM
    Mode 1: 320x256, 4 colors, 20K RAM
    Mode 2: 160x256, 8 colors, 20K RAM
    Mode 3: 80x25 text, 2 colors, 16K RAM
    Mode 4: 320x256, 2 colors, 10K RAM
    Mode 5: 160x256, 4 colors, 10K RAM
    Mode 6: 40x25 text, 2 colors, 8K RAM
    Mode 7: Teletext, 40x25, 1K RAM (SAA5050)

  KEY ZERO-PAGE VARIABLES:
  ---------------------------------------------------------------
  $0018          Current screen mode
  $0028-$0029    BASIC PAGE (start of program, mode-dependent)
  $0002-$0003    HIMEM (top of user memory)
  $0006-$0007    LOMEM (start of variables)

  SHEILA: HARDWARE REGISTERS ($FE00-$FEFF):
  ---------------------------------------------------------------
  $FE00-$FE07    6845 CRTC
                   $FE00: Register select
                   $FE01: Register data
  $FE08-$FE0F    ACIA (serial, 6850)
  $FE10-$FE1F    Serial ULA
  $FE20-$FE2F    Video ULA
                   $FE20: Control register
                   $FE21: Palette register
  $FE30-$FE3F    Paged ROM select
                   $FE30: Write = ROM bank number (0-15)
  $FE40-$FE5F    System VIA (6522)
                   $FE40: Port B (keyboard, sound, LEDs)
                   $FE41: Port A (keyboard data)
                   $FE42: DDR B
                   $FE43: DDR A
                   $FE44-$FE45: Timer 1
                   $FE46-$FE47: Timer 1 latch
                   $FE48-$FE49: Timer 2
                   $FE4A: Shift register
                   $FE4B: Auxiliary control
                   $FE4C: Peripheral control
                   $FE4D: Interrupt flags
                   $FE4E: Interrupt enable
  $FE60-$FE7F    User VIA (6522) - same register layout
  $FE80-$FE9F    Floppy disk controller (8271)
  $FEA0-$FEBF    Econet (68B54)
  $FEC0-$FEDF    ADC (uPD7002)
  $FEE0-$FEFF    Tube ULA (co-processor interface)

  RECOMMENDED INIT:
  ---------------------------------------------------------------
    static void memmap_init_bbc(unsigned char *mem)
    {
        /* Screen mode 7 (Teletext, default) */
        mem_set(mem, 0x0018, 0x07);

        /* PAGE: start of BASIC program */
        mem_set16(mem, 0x0028, 0x1900);  /* Mode 7 PAGE */

        /* HIMEM */
        mem_set16(mem, 0x0002, 0x7C00);

        /* Screen memory: Mode 7 at $7C00 */
        mem_fill(mem, 0x7C00, 1000, 0x20);

        /* OS identification */
        mem_set(mem, 0xFFF7, 0x01);  /* MOS 1.20 */

        /* System VIA defaults */
        mem_set(mem, 0xFE40, 0x00);  /* Port B */
        mem_set(mem, 0xFE42, 0x00);  /* DDR B */

        /* RESET vector */
        mem_set16(mem, 0xFFFC, 0xD9CD);
    }


---------------------------------------------------------------------
5.3  Amstrad CPC 464/6128
---------------------------------------------------------------------

  CPU: Zilog Z80A (4 MHz)
  Video: Motorola 6845 CRTC + Gate Array, 27 colors
  Audio: AY-3-8912 (3 channels + noise)
  I/O: PPI 8255A
  RAM: 64K (CPC 464) / 128K (CPC 6128)
  OS: AMSDOS + Locomotive BASIC

  MEMORY LAYOUT:
  ---------------------------------------------------------------
    $0000-$003F: Restart vectors and interrupt table
    $0040-$00FF: Firmware workspace
    $0100-$0169: Jumpblock: system call entry points
    $0170-$A67B: User RAM (BASIC program + variables)
    $A67C-$BFFF: Screen memory (if 16K mode)
    $C000-$FFFF: Screen memory (default, 16K)

  CPC SCREEN MODES:
    Mode 0: 160x200, 16 colors
    Mode 1: 320x200, 4 colors (default)
    Mode 2: 640x200, 2 colors

  GATE ARRAY (I/O port $7Fxx):
  ---------------------------------------------------------------
  Selected by OUT to port $7F:
    Bits 6-7 = function:
      00 = Pen select (bits 0-4 = pen number, 0-15 or 16=border)
      01 = Ink assign (bits 0-4 = hardware color 0-26)
      10 = ROM/mode control
           Bit 0-1: Screen mode (0-2)
           Bit 2: ROM mapping (lower ROM on/off)
           Bit 3: ROM mapping (upper ROM on/off)
           Bit 4: Interrupt control
      11 = RAM banking (6128 only)

  CPC HARDWARE COLORS (27 total):
    0: Black         9: Bright Red     18: Bright Green
    1: Blue          10: Bright Yellow  19: Pastel Green
    2: Bright Blue   11: Pastel Yellow  20: Bright Cyan
    3: Red           12: Dark Green     21: Pastel Cyan
    4: Magenta       13: Sea Green      22: Bright White
    5: Mauve         14: Bright Cyan    23: Pastel White
    6: Bright Red    15: Lime Green     24: Bright Magenta
    7: Purple        16: Pastel Magenta 25: Pastel Magenta
    8: Bright Magenta 17: Orange        26: Bright Yellow

  FIRMWARE JUMPBLOCK ($BB00-$BD5C):
  ---------------------------------------------------------------
  $BB00: KM INITIALISE     Keyboard init
  $BB06: KM WAIT CHAR      Wait for keypress
  $BB09: KM READ CHAR      Read key (no wait)
  $BB5A: TXT OUTPUT         Print character
  $BB5D: GRA PLOT           Plot pixel
  $BB60: GRA LINE           Draw line
  $BBDE: MC WAIT FLYBACK    Wait for vsync
  $BD19: CAS IN OPEN        Open file for reading
  $BD1C: CAS IN CLOSE       Close input file
  $BD1F: CAS IN CHAR        Read character
  $BD22: CAS OUT OPEN       Open file for writing
  $BD25: CAS OUT CLOSE      Close output file
  $BD28: CAS OUT CHAR       Write character

  PPI 8255A:
  ---------------------------------------------------------------
  Port $F4xx: Port A (PSG data)
  Port $F5xx: Port B (vsync, tape, printer, brand)
               Bit 0: VSYNC from CRTC
               Bit 1-3: Brand (Amstrad, Schneider, etc.)
               Bit 4: 50/60 Hz
               Bit 6: Printer busy
               Bit 7: Cassette data input
  Port $F6xx: Port C (PSG control, cassette, keyboard)
  Port $F7xx: PPI control register

  RECOMMENDED INIT:
  ---------------------------------------------------------------
    static void memmap_init_cpc(unsigned char *mem)
    {
        /* BASIC workspace */
        mem_set16(mem, 0x0040, 0x0170);  /* BASIC start */

        /* Screen memory: Mode 1 default at $C000 */
        mem_fill(mem, 0xC000, 16384, 0x00);

        /* Firmware jumpblock - first byte = JP (C3) */
        mem_set(mem, 0xBB00, 0xC3);
        mem_set(mem, 0xBB5A, 0xC3);

        /* PPI defaults */
        mem_set(mem, 0xF500, 0x1E);  /* Port B: brand + vsync */

        /* ROM identification */
        mem_set(mem, 0x0006, 0x01);  /* CPC 464 marker */
    }


---------------------------------------------------------------------
5.4  Dragon 32/64
---------------------------------------------------------------------

  CPU: Motorola 6809E (0.89 MHz)
  Video: Motorola 6847 VDG
  Audio: 1-bit DAC + cassette
  I/O: SAM (MC6883), PIA (MC6821 x 2)
  RAM: 32K / 64K

  MEMORY LAYOUT:
  ---------------------------------------------------------------
    $0000-$03FF: Direct page (system variables)
    $0400-$05FF: Screen RAM (text: 32x16 = 512 bytes)
    $0600-$7FFF: User RAM (BASIC program)
    $8000-$9FFF: Extended BASIC ROM (8K)
    $A000-$BFFF: BASIC ROM (8K)
    $C000-$FEFF: Cartridge space / unused
    $FF00-$FF03: PIA 0 (keyboard + joystick)
    $FF04-$FF07: Deprecated PIA (active on CoCo)
    $FF20-$FF23: PIA 1 (serial, cassette, DAC, VDG)
    $FFC0-$FFDF: SAM registers
    $FFF0-$FFFF: 6809 vectors

  6847 VDG MODES:
    Internal Alphanumeric: 32x16 green/orange text
    Semigraphics 4: 64x32 block graphics
    Semigraphics 6: 64x48 block graphics
    CG1: 64x64, 4 colors
    CG2: 128x64, 4 colors
    CG3: 128x96, 4 colors
    CG6: 256x192, 4 colors
    RG1: 128x64, 2 colors
    RG2: 128x96, 2 colors
    RG3: 128x192, 2 colors
    RG6: 256x192, 2 colors

  SAM REGISTERS ($FFC0-$FFDF):
  ---------------------------------------------------------------
  Written in pairs (clear/set for each bit):
    $FFC0/$FFC1: Display offset bit 0
    $FFC2/$FFC3: Display offset bit 1
    $FFC4/$FFC5: Display offset bit 2
    $FFC6/$FFC7: Display offset bit 3
    $FFC8/$FFC9: Display offset bit 4
    $FFCA/$FFCB: Display offset bit 5
    $FFCC/$FFCD: Display offset bit 6
    $FFCE/$FFCF: Page select bit 0
    $FFD0/$FFD1: Page select bit 1
    $FFD2/$FFD3: VDG mode bit 0
    $FFD4/$FFD5: VDG mode bit 1
    $FFD6/$FFD7: VDG mode bit 2
    $FFD8/$FFD9: Memory size bit 0
    $FFDA/$FFDB: Memory size bit 1
    $FFDC/$FFDD: Clock speed (0=normal, 1=high)
    $FFDE/$FFDF: Map type (0=ROM, 1=RAM at top 32K)

  6809 VECTORS ($FFF0-$FFFF):
  ---------------------------------------------------------------
    $FFF0-$FFF1: Reserved
    $FFF2-$FFF3: SWI3
    $FFF4-$FFF5: SWI2
    $FFF6-$FFF7: FIRQ
    $FFF8-$FFF9: IRQ
    $FFFA-$FFFB: SWI
    $FFFC-$FFFD: NMI
    $FFFE-$FFFF: RESET

  RECOMMENDED INIT:
  ---------------------------------------------------------------
    static void memmap_init_dragon(unsigned char *mem)
    {
        /* Screen RAM: 32x16 = 512 spaces */
        mem_fill(mem, 0x0400, 512, 0x60);  /* $60 = space */

        /* BASIC pointers */
        mem_set16(mem, 0x0019, 0x0600);  /* TXTTAB */
        mem_set16(mem, 0x0027, 0x7FFF);  /* MEMSIZ */

        /* PIA defaults */
        mem_set(mem, 0xFF00, 0xFF);  /* PIA 0 Port A */
        mem_set(mem, 0xFF01, 0x34);  /* PIA 0 CRA */
        mem_set(mem, 0xFF02, 0xFF);  /* PIA 0 Port B */
        mem_set(mem, 0xFF03, 0x34);  /* PIA 0 CRB */
        mem_set(mem, 0xFF20, 0x00);  /* PIA 1 Port A */
        mem_set(mem, 0xFF21, 0x34);  /* PIA 1 CRA */
        mem_set(mem, 0xFF22, 0x00);  /* PIA 1 Port B */
        mem_set(mem, 0xFF23, 0x3C);  /* PIA 1 CRB */

        /* RESET vector */
        mem_set16(mem, 0xFFFE, 0xB3B4);
    }


---------------------------------------------------------------------
5.5  TRS-80 Color Computer (CoCo)
---------------------------------------------------------------------

  CPU: Motorola 6809E (0.89 MHz, CoCo 1/2), 6809E (CoCo 3)
  Video: 6847 VDG (CoCo 1/2), GIME (CoCo 3)
  Audio: 6-bit DAC
  I/O: SAM (MC6883), PIA (MC6821 x 2)
  RAM: 4K-512K
  BASIC: Color BASIC / Extended Color BASIC / Disk BASIC

  Nearly identical to Dragon 32/64 but with different
  ROM entry points and BASIC pointers.

  KEY DIFFERENCES FROM DRAGON:
  ---------------------------------------------------------------
    PIA 1 at $FF20-$FF23 used for cassette + DAC
    Sound output on PIA 1 Port A bits 2-7 (6-bit DAC)
    ROM identification differs

  BASIC POINTERS (DECB):
  ---------------------------------------------------------------
  $0019-$001A    Start of BASIC text
  $001B-$001C    Start of variables
  $001D-$001E    Start of arrays
  $001F-$0020    End of arrays
  $0027-$0028    Top of memory string space

  SCREEN ($0400-$05FF):
  ---------------------------------------------------------------
  512 bytes (32 x 16), filled with $60 (space in SG4 mode)

  RECOMMENDED INIT:
  ---------------------------------------------------------------
    static void memmap_init_coco(unsigned char *mem)
    {
        mem_fill(mem, 0x0400, 512, 0x60);
        mem_set16(mem, 0x0019, 0x2601);  /* TXTTAB */
        mem_set16(mem, 0x0027, 0x7FFF);  /* MEMSIZ */
        mem_set(mem, 0xFF00, 0xFF);      /* PIA 0A */
        mem_set(mem, 0xFF02, 0xFF);      /* PIA 0B */
        mem_set(mem, 0xFF22, 0x00);      /* PIA 1B: VDG mode */
        mem_set16(mem, 0xFFFE, 0xA027);  /* RESET */
    }


---------------------------------------------------------------------
5.6  Oric-1 / Oric Atmos
---------------------------------------------------------------------

  CPU: MOS 6502A (1 MHz)
  Video: Custom ULA, 40x28 text or 240x200 hi-res
  Audio: AY-3-8912 (3 channels + noise)
  I/O: VIA 6522
  RAM: 48K

  MEMORY LAYOUT:
  ---------------------------------------------------------------
    $0000-$00FF: Zero page
    $0100-$01FF: Stack
    $0200-$02FF: System variables
    $0300-$03FF: User vectors
    $0400-$04FF: Disk workspace
    $0500-$97FF: User RAM
    $9800-$9FFF: Alternate character set
    $A000-$B7FF: Hi-res screen (6K)
    $B800-$BFDF: Text screen (1120 bytes, 40x28)
    $BFE0-$BFFF: System / video control
    $C000-$FFFF: ROM (16K)

  TEXT SCREEN ($BB80-$BFE0):
  ---------------------------------------------------------------
  1120 bytes (40 columns x 28 rows)
  Top 3 rows ($BB80-$BBF7) are status area.

  Oric uses attribute bytes inline in the screen data:
    $00: Ink black     $10: Paper black
    $01: Ink red       $11: Paper red
    $02: Ink green     $12: Paper green
    $03: Ink yellow    $13: Paper yellow
    $04: Ink blue      $14: Paper blue
    $05: Ink magenta   $15: Paper magenta
    $06: Ink cyan      $16: Paper cyan
    $07: Ink white     $17: Paper white

  VIA 6522 ($0300-$030F):
  ---------------------------------------------------------------
  $0300: Port B (keyboard column + printer)
  $0301: Port A (PSG data + keyboard row)
  $030F: Port A no-handshake

  RECOMMENDED INIT:
  ---------------------------------------------------------------
    static void memmap_init_oric(unsigned char *mem)
    {
        mem_fill(mem, 0xBB80, 1120, 0x20);   /* Screen spaces */
        mem_set16(mem, 0x009A, 0x0501);       /* TXTTAB */
        mem_set16(mem, 0x009C, 0x97FF);       /* HIMEM */
        mem_set(mem, 0x0300, 0xFF);           /* VIA Port B */
        mem_set16(mem, 0xFFFC, 0xF88F);       /* RESET */
    }


---------------------------------------------------------------------
5.7  SAM Coupe
---------------------------------------------------------------------

  CPU: Zilog Z80B (6 MHz)
  Video: ASIC, 256x192 (Spectrum compatible) + new modes
  Audio: SAA1099 (6 channels stereo)
  I/O: ASIC handles most I/O
  RAM: 256K-512K (paged in 16K banks)

  MEMORY LAYOUT (4 x 16K pages):
  ---------------------------------------------------------------
    Section A ($0000-$3FFF): Page from bank 0-31
    Section B ($4000-$7FFF): Page from bank 0-31
    Section C ($8000-$BFFF): Page from bank 0-31
    Section D ($C000-$FFFF): Page from bank 0-31

  Default: ROM in Section A/B, RAM in Section C/D

  DISPLAY MODES:
    Mode 1: 256x192, 16 colors (Spectrum compatible)
    Mode 2: 256x192, 16 colors (per-pixel, 24K VRAM)
    Mode 3: 512x192, 4 colors
    Mode 4: 256x192, 16 colors (mode 2 variant)

  KEY I/O PORTS:
  ---------------------------------------------------------------
  Port $FE:     Border color + beeper (Spectrum compatible)
  Port $F8:     Keyboard (extended)
  Port $FC:     LMPR (Low Memory Page Register)
  Port $FD:     HMPR (High Memory Page Register)
  Port $FE:     VMPR (Video Memory Page Register)
  Port $FF:     CLUT (Color Look-Up Table select)
  Port $01FE:   STAT (status line interrupt)
  Port $00FF:   SAA1099 data
  Port $01FF:   SAA1099 address

  RECOMMENDED INIT:
  ---------------------------------------------------------------
    static void memmap_init_samcoupe(unsigned char *mem)
    {
        /* Spectrum-compatible screen at $4000 */
        mem_fill(mem, 0x4000, 6144, 0x00);
        mem_fill(mem, 0x5800, 768, 0x38);

        /* SAM-specific system vars */
        mem_set(mem, 0x5C48, 0x01);  /* BORDCR */
        mem_set(mem, 0x5C8D, 0x07);  /* ATTR_P */

        mem_set16(mem, 0xFFFC, 0x0000);  /* RESET */
    }


---------------------------------------------------------------------
5.8  Acorn Electron
---------------------------------------------------------------------

  CPU: MOS 6502A (2 MHz, halved to 1 MHz during display)
  Video: ULA (same modes as BBC Micro, minus Mode 7)
  Audio: 1-bit beeper via ULA
  RAM: 32K (shared with display)

  Same display modes as BBC Micro except no Mode 7 (Teletext).
  Reduced I/O compared to BBC Micro.

  KEY ADDRESSES:
  ---------------------------------------------------------------
  $FE05: ULA control (display mode, cassette, interrupts)
  $FE06: ULA palette (4-bit data for each ink)
  $FE07: ULA auxiliary
  $FC00-$FCFF: Cartridge slot

  RECOMMENDED INIT:
  ---------------------------------------------------------------
    static void memmap_init_electron(unsigned char *mem)
    {
        mem_set(mem, 0x0018, 0x06);          /* Mode 6 */
        mem_set16(mem, 0x0028, 0x1900);      /* PAGE */
        mem_set16(mem, 0x0002, 0x7C00);      /* HIMEM */
        mem_fill(mem, 0x7C00, 1000, 0x20);   /* Screen */
        mem_set16(mem, 0xFFFC, 0xD9CD);      /* RESET */
    }


---------------------------------------------------------------------
5.9  Camputers Lynx
---------------------------------------------------------------------

  CPU: Zilog Z80A (4 MHz)
  Video: Custom, 256x252 hi-res, 8 colors
  Audio: 1-bit beeper
  RAM: 48K / 96K / 192K

  MEMORY LAYOUT:
    $0000-$1FFF: System workspace
    $2000-$3FFF: Screen RAM (8K)
    $4000-$5FFF: Green bitmap
    $6000-$7FFF: Red bitmap
    $8000-$9FFF: Blue bitmap
    $A000-$BFFF: Alternate screen
    $C000-$DFFF: ROM (8K)
    $E000-$FFFF: RAM / ROM bank


---------------------------------------------------------------------
5.10  Jupiter Ace
---------------------------------------------------------------------

  CPU: Zilog Z80A (3.25 MHz)
  Video: Custom, 32x24 characters, attributes
  Audio: 1-bit beeper
  RAM: 3K (expandable to 51K)
  Language: Forth (not BASIC!)

  MEMORY LAYOUT:
    $0000-$1FFF: ROM (8K Forth kernel)
    $2000-$23FF: Video RAM (1K, 32x32 characters)
    $2400-$27FF: Character definitions (128 x 8 bytes)
    $2800-$2BFF: Unused
    $2C00-$2FFF: System variables + PAD
    $3000-$3BFF: RAM (3K base)
    $3C00+:      Expansion RAM


---------------------------------------------------------------------
5.11  Mattel Aquarius
---------------------------------------------------------------------

  CPU: Zilog Z80A (3.58 MHz)
  Video: TMS9918A-compatible (Texas Instruments)
  Audio: 1-bit beeper + AY-3-8910 (with expansion)
  RAM: 4K (expandable to 52K)

  MEMORY LAYOUT:
    $0000-$1FFF: ROM (8K, Microsoft BASIC)
    $2000-$2FFF: ROM (4K, character generator)
    $3000-$33FF: Screen RAM (1K, 40x24)
    $3400-$37FF: Color RAM (1K, 40x24)
    $3800-$3FFF: System RAM
    $4000-$FFFF: Expansion RAM

  CHARACTER/COLOR RAM:
    Screen: 40 columns x 24 rows = 960 characters
    Color:  Each byte = foreground (low nybble) + background
    Default: White on blue

  RECOMMENDED INIT:
  ---------------------------------------------------------------
    static void memmap_init_aquarius(unsigned char *mem)
    {
        mem_fill(mem, 0x3000, 960, 0x20);    /* Screen: spaces */
        mem_fill(mem, 0x3400, 960, 0x17);    /* Color: white/blue */
        mem_set16(mem, 0x3800, 0x3900);      /* TXTTAB */
        mem_set16(mem, 0x3802, 0xFFFF);      /* MEMSIZ */
    }


---------------------------------------------------------------------
5.12  Sharp MZ-700 / MZ-800
---------------------------------------------------------------------

  CPU: Zilog Z80A (3.58 MHz)
  Video: Character display, 40x25, 8 colors
  Audio: Built-in beeper
  RAM: 64K
  Unique: "Clean computer" - no built-in BASIC (loaded from tape)

  MEMORY LAYOUT:
    $0000-$0FFF: Monitor ROM (4K, switchable)
    $1000-$1FFF: VRAM (character, 40x25 = 1000 bytes)
    $1800-$1FFF: Color attribute RAM
    $2000-$7FFF: User RAM
    $8000-$9FFF: VRAM / RAM (MZ-800)
    $D000-$DFFF: Character generator RAM
    $E000-$FFFF: Monitor ROM (8K, switchable)


---------------------------------------------------------------------
5.13  Sord M5
---------------------------------------------------------------------

  CPU: Zilog Z80A (3.58 MHz)
  Video: TMS9929A (PAL variant of TMS9918A)
  Audio: SN76489AN (3 tone + noise)
  RAM: 4K (expandable to 36K)
  BASIC: Sord BASIC-I, BASIC-G, BASIC-F

  MEMORY LAYOUT:
    $0000-$1FFF: ROM (8K BIOS)
    $2000-$6FFF: Cartridge ROM (20K)
    $7000-$73FF: System RAM (1K)
    $7400-$7FFF: Unused
    $8000-$FFFF: Expansion RAM (32K)


---------------------------------------------------------------------
5.14  Tandy 1000
---------------------------------------------------------------------

  CPU: Intel 8088 (4.77 MHz)
  Video: TGA (Tandy Graphics Adapter, PCjr compatible)
  Audio: TI SN76496 (3 tone + noise)
  RAM: 128K-640K

  Essentially an IBM PC compatible with enhanced graphics
  and sound.  The MSDOS map covers most of this; the Tandy
  1000 adds:

    Video RAM at $B8000 (segment $B800)
    TGA registers at I/O ports $03DA-$03DF
    SN76496 at I/O port $00C0

  For BASIC++ purposes, use MEMMAP "MSDOS" as the base
  and POKE the Tandy-specific registers manually.


---------------------------------------------------------------------
5.15  Amstrad PCW (PCW 8256/8512/9512)
---------------------------------------------------------------------

  CPU: Zilog Z80A (4 MHz)
  Video: Custom (720x256 monochrome, text 90x32)
  Audio: Beeper
  RAM: 256K-512K (paged in 16K blocks)
  OS: CP/M Plus (CP/M 3.0)
  No built-in BASIC (Mallard BASIC available)

  MEMORY LAYOUT:
    4 x 16K pages, banked from 256K/512K total
    Video RAM: ~32K (roller RAM)
    Default: CP/M TPA at $0100-$BDFF

  For BASIC++ purposes, emulate the Mallard BASIC workspace:
    Start of program: $0100
    HIMEM: $BDFF


=====================================================================
6. MEMORY MAP INTERACTIONS (PEEK/POKE/INP/OUT/DEF SEG)
=====================================================================

6.1  PEEK and POKE
-------------------
  These directly read/write the 64K virtual memory:

    POKE address, value     Write byte (0-255)
    X = PEEK(address)       Read byte

  Addresses are 0-65535 ($0000-$FFFF).

6.2  INP and OUT
-----------------
  INP and OUT use the same virtual memory:

    OUT port, value         Same as POKE port, value
    X = INP(port)           Same as PEEK(port)

  In the MSDOS map, the convention is that ports $0000-$03FF
  are "I/O space" (overlapping the IVT).  In other maps,
  INP/OUT are simply aliases for PEEK/POKE.

6.3  DEF SEG
--------------
  DEF SEG sets a segment base address.  PEEK and POKE then
  use SEGMENT * 16 + OFFSET:

    DEF SEG = &HB800        Video memory segment
    POKE 0, 65              'A' at top-left
    POKE 1, 7               White on black attribute

    DEF SEG = 0             Reset to default

  Note: On non-x86 platforms, DEF SEG still works but the
  concept of segments doesn't apply to real hardware.  The
  effective address is simply (DEF SEG * 16 + offset) MOD 65536.

6.4  MEMMAP Does NOT Clear Variables
--------------------------------------
  Switching maps replaces the 64K memory contents but does
  NOT affect BASIC variables, arrays, or program lines.
  Those are stored separately in the interpreter's own memory.

6.5  MEMMAP Does NOT Affect File I/O
--------------------------------------
  File channels, screen output, and keyboard input all go
  through the VDev layer, not through virtual memory.
  Files opened with OPEN remain valid after MEMMAP switches.


=====================================================================
7. MAGIC ADDRESSES AND SIDE EFFECTS
=====================================================================

Some addresses trigger side effects when POKEd, depending
on the active memory map:

  C64:
    POKE 53280, n  -> Updates virtual border color register
    POKE 53281, n  -> Updates virtual background color register
    POKE 53272, n  -> Updates VIC memory setup (charset/screen)

  MSDOS:
    POKE &H449, n  -> Updates video mode state register

  ATARI8:
    POKE 559, n    -> Updates DMACTL (screen display control)
    POKE 710, n    -> Updates playfield color 2

  SPECTRUM:
    OUT 254, n     -> Updates border color (bits 0-2)

  General:
    Most addresses are passive (store/retrieve only).
    "Magic" triggering for full hardware emulation is a
    planned enhancement.  Currently, all addresses are
    plain read/write with pre-filled defaults.


=====================================================================
8. COMBINING MEMMAP + DIALECT
=====================================================================

For the most authentic experience, pair the memory map with
the matching dialect:

  DIALECT "COCO" : MEMMAP "TRS80"     ' CoCo BASIC + TRS-80 memory
  DIALECT "GWBS" : MEMMAP "MSDOS"     ' GW-BASIC + PC memory
  DIALECT "BPP"  : MEMMAP "C64"       ' Full BASIC++ + C64 memory
  DIALECT "C64B" : MEMMAP "C64"       ' CBM BASIC V2 + C64 memory
  DIALECT "SINC" : MEMMAP "SPECTRUM"  ' Sinclair BASIC + Spectrum memory
  DIALECT "MSX"  : MEMMAP "MSX"       ' MSX-BASIC + MSX memory
  DIALECT "BBC"  : MEMMAP "BBC"       ' BBC BASIC + Acorn memory

The dialect controls SYNTAX; the memmap controls MEMORY.
They are independent and can be freely combined.

Mixed example:
  DIALECT "BPP" : MEMMAP "C64"
  ' Use BASIC++ enhanced syntax but read C64 hardware registers
  PRINT PEEK(53280)    ' 14 (light blue border)
  POKE 53280, 0        ' Change to black
  ' Use BASIC++ string functions not available in CBM BASIC:
  PRINT LEFT$(BIN$(PEEK(&HD020)), 4)


=====================================================================
9. ADVANCED TECHNIQUES
=====================================================================

9.1  Reading ROM Signatures for Platform Detection
----------------------------------------------------
  Your BASIC program can detect which map is active:

    IF PEEK(&HA004) = ASC("C") THEN
      PRINT "Commodore 64 detected"
    ELSEIF PEEK(&HFBB3) = 6 THEN
      PRINT "Apple IIe detected"
    ELSEIF PEEK(&H0062) = ASC("R") THEN
      PRINT "TRS-80 detected"
    END IF

9.2  Cross-Platform Programs
------------------------------
  Write programs that adapt to the active memory map:

    MAP$ = ""
    IF PEEK(&HD020) = 14 THEN MAP$ = "C64"
    IF PEEK(&H02C8) = 0 AND PEEK(&HD400) = &H22 THEN MAP$ = "ATARI"
    IF PEEK(&HC000) = 0 AND PEEK(&HFBB3) = 6 THEN MAP$ = "APPLE2"
    IF MAP$ = "" THEN MAP$ = "UNKNOWN"
    PRINT "Running on: "; MAP$

9.3  Snapshot and Restore
---------------------------
  Save the current memory state and restore it later:

    ' Save
    DIM SNAP(65535) AS INTEGER
    FOR I = 0 TO 65535 : SNAP(I) = PEEK(I) : NEXT

    ' ... do stuff ...

    ' Restore
    FOR I = 0 TO 65535 : POKE I, SNAP(I) : NEXT

9.4  Partial Memory Dumps
---------------------------
  Inspect a range of memory:

    DEF FN HX$(N) = RIGHT$("0" + HEX$(N), 2)
    ADDR = &HD000 : ' VIC-II registers
    FOR ROW = 0 TO 15
      PRINT HEX$(ADDR + ROW * 16); ": ";
      FOR COL = 0 TO 15
        PRINT FN HX$(PEEK(ADDR + ROW * 16 + COL)); " ";
      NEXT COL
      PRINT
    NEXT ROW

9.5  Using Memory Maps for Regression Testing
-----------------------------------------------
  Load a known map and verify values:

    MEMMAP "C64"
    ASSERT PEEK(&HD020) = &H0E, "Border should be light blue"
    ASSERT PEEK(&HD021) = &H06, "Background should be blue"
    ASSERT PEEK(&H0001) = &H37, "Bank config should be $37"
    ASSERT PEEK(&HFF80) = &H03, "KERNAL should be rev 3"
    PRINT "C64 memory map: ALL TESTS PASSED"

9.6  Building a Multi-Platform Demo
--------------------------------------
    FOR EACH M$ IN ("C64", "APPLE2", "SPECTRUM", "ATARI8")
      MEMMAP M$
      PRINT M$; " - RESET vector: ";
      PRINT HEX$(PEEK(&HFFFC) + PEEK(&HFFFD) * 256)
    NEXT


=====================================================================
10. REFERENCE DATA SOURCES
=====================================================================

When building a new map, you need to know what values the
real hardware has at cold boot.  Here are definitive references:

  COMMODORE MACHINES:
    "Mapping the Commodore 64" by Sheldon Leemon
    "Commodore 64 Programmer's Reference Guide" (Commodore)
    "The Commodore 128 Programmer's Reference" (Commodore)
    "VIC-20 Programmer's Reference Guide" (Commodore)
    "Commodore PET/CBM Personal Computer Guide" (Osborne)

  ATARI 8-BIT:
    "Mapping the Atari" by Ian Chadwick
    "De Re Atari" (Atari, Inc.)
    "Atari 400/800 Technical Reference Notes"
    "The Atari BASIC Source Book" (COMPUTE! Books)

  APPLE II:
    "Apple II Reference Manual" (Apple Computer)
    "Beneath Apple DOS" (Quality Software)
    "Understanding the Apple II" by Jim Sather
    "Apple IIe Technical Reference Manual"

  ZX SPECTRUM:
    "The Complete Spectrum ROM Disassembly" by Dr. Ian Logan
    "Sinclair ZX Spectrum BASIC Programming" (Sinclair)
    "The ZX Spectrum ULA" by Chris Smith

  TRS-80:
    "TRS-80 Technical Reference Manual" (Radio Shack)
    "TRS-80 Model I/III ROM Listing"
    "Going Forth on the TRS-80" (Model I/III)

  MSX:
    "MSX Technical Data Book" (ASCII Corporation)
    "MSX Red Book" (Avalon Software)
    "MSX2 Technical Handbook"

  BBC MICRO:
    "BBC Micro Advanced User Guide" (Cambridge Micro)
    "The BBC Microcomputer User Guide" (BBC)
    "Acorn MOS Operating System Manual"

  AMSTRAD CPC:
    "CPC Firmware Guide" (Soft 968/Amstrad)
    "Amstrad CPC 464/664/6128 Firmware ROM Routines"
    "The Amstrad CPC Program Development Toolkit"

  DRAGON / COCO:
    "Dragon Data Technical Manual"
    "TRS-80 Color Computer Technical Reference Manual"
    "Inside the Dragon" (Inside Series)

  ORIC:
    "The Oric Advanced User Guide"
    "Oric Atmos Hardware Manual"

  GENERAL:
    "8-Bit Microprocessor Handbook" by Adam Osborne
    "Inside the Machine" by Jon Stokes
    MAME/MESS source code (machine drivers)


=====================================================================
11. TIPS FOR MAP AUTHORS
=====================================================================

  1. Study the real machine's boot sequence to know what
     values appear in memory at the READY/OK prompt.

  2. Focus on the values that BASIC programs actually PEEK:
     screen memory, color registers, system pointers, ROM
     signatures, and I/O chip defaults.

  3. Don't fill the entire ROM range with data — just the
     key identification bytes and entry points.

  4. Test with real programs from the target platform.
     Load a Commodore 64 type-in listing and verify that
     the PEEKs return expected values.

  5. Use MEMMAP "NONE" as a starting point for completely
     custom systems (game consoles, embedded devices, etc.)

  6. The 64K limit is fixed.  For systems with banked memory
     (C128, MSX, CPC 6128, SAM Coupe), represent the default
     bank 0 layout — the view the CPU sees at cold boot.

  7. For Z80 systems, remember that hardware I/O uses ports
     (IN/OUT instructions), not memory-mapped I/O.  BASIC++
     maps INP/OUT to the same 64K array, so use the port
     addresses if you want INP(port) to return useful values.

  8. For systems with multiple video modes (BBC, CPC, Atari),
     initialize screen memory for the DEFAULT boot mode only.

  9. Document your map!  Add a comment block at the top of
     your init function listing every address you set and why.

 10. Consider adding your map to this manual (U_Memory_Maps.txt)
     so other users can benefit from your work.

 11. When in doubt, use MAME/MESS to boot the real machine
     and dump memory at the BASIC prompt.  That dump IS your
     init function's specification.

 12. For 16-bit machines (Tandy 1000, IBM PC), the 64K window
     is limited.  Focus on the first 64K (segment 0000) which
     contains the IVT and BIOS Data Area.  Video memory lives
     at higher segments and requires DEF SEG to access.


=====================================================================
END OF DOCUMENT
=====================================================================
