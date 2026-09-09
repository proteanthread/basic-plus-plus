<!--
Title:        Graphics_Modes
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, SDL2, BGI)
Authority:    engine/src/device/bgi/
Generated:    no, manual reference
Status:       current
-->

# BASIC++ v6.5.2 Graphics Modes

## 1. THE BGI GRAPHICS SYSTEM

BASIC++ utilizes the Borland Graphics Interface (BGI) rasterizer subsystem (`engine/src/device/bgi/`) for 2D graphics rendering. BGI is packaged within `libhardware` and integrates with `libadvanced` (SDL2) for hardware-accelerated window display. For headless text terminals and SSH sessions, an ASCII-art fallback engine (AAlib, `engine/src/device/bgi/aalib/aalib.c`) renders graphics frames directly in terminal text cells.

## 2. STANDARD SCREEN MODES

The `SCREEN` statement configures the active display mode:

`SCREEN mode [, [colorswitch] [, [active_page] [, visual_page]]]`

| Mode | Resolution | Colors | Text Format | Video Adapter Emulated |
| :--- | :--- | :--- | :--- | :--- |
| **0** | Text Only | 16 | 80x25 / 40x25 | MDPA / CGA / EGA / VGA Text |
| **1** | 320x200 | 4 | 40x25 | CGA 4-Color Graphics |
| **2** | 640x200 | 2 | 80x25 | CGA High-Resolution Mono |
| **3** | 720x348 | 2 | 80x25 | Hercules Graphics Card (HGC) |
| **4** | 320x200 | 4 | 40x25 | EGA 4-Color Mode |
| **7** | 320x200 | 16 | 40x25 | EGA 16-Color Graphics |
| **8** | 640x200 | 16 | 80x25 | EGA High-Res 16-Color |
| **9** | 640x350 | 16 | 80x25 | EGA High-Res 350-Line |
| **10** | 640x350 | 4 | 80x25 | EGA Monochrome Graphics |
| **11** | 640x480 | 2 | 80x30 | VGA Two-Color Graphics |
| **12** | 640x480 | 16 | 80x30 | VGA 16-Color Standard |
| **13** | 320x200 | 256 | 40x25 | VGA / MCGA 256-Color (Mode 13h) |

Modern high-resolution modes:
- `SCREEN 14`: 800x600 SVGA (16/256 colors)
- `SCREEN 15`: 1024x768 XGA (256/16M colors)
- `SCREEN 16`: 1280x720 720p HD
- `SCREEN 17`: 1280x1024 SXGA
- `SCREEN 18`: 1920x1080 1080p Full HD
- `SCREEN 19`: 2560x1440 1440p QHD
- `SCREEN 20`: 3840x2160 4K UHD

## 3. CUSTOM RESOLUTION MODES

BASIC++ permits arbitrary window sizes and split-screen configurations:
- `SET SCREEN width, height [, bpp [, text_cols [, text_rows [, fps]]]]`: Full-canvas graphics mode (default 8 bpp, 60 fps).
- `SET GRAPHICS width, height [, bpp [, text_cols [, text_rows [, fps]]]]`: Split-screen mode with dedicated text scrolling window at the bottom.
- `SET MODE n`: Selects from 35 predefined heritage computer display profiles.

## 4. RETRO HARDWARE MODES

The heritage mode descriptor table in `engine/src/device/bgi/bgi_modes.c` emulates vintage computing displays:

| ID | Name | Resolution | Colors | System Emulated |
| :--- | :--- | :--- | :--- | :--- |
| **0** | `CGA SCREEN 1` | 320x200 | 4 | IBM CGA 4-Color |
| **1** | `CGA SCREEN 2` | 640x200 | 2 | IBM CGA Monochrome |
| **2** | `Tandy SCREEN 3` | 160x200 | 16 | Tandy 1000 / PCjr |
| **3** | `EGA SCREEN 9` | 640x350 | 16 | IBM EGA 16-Color |
| **4** | `EGA SCREEN 10` | 640x350 | 2 | IBM EGA Monochrome |
| **5** | `VGA SCREEN 12` | 640x480 | 16 | IBM VGA 16-Color |
| **6** | `VGA Mode 13h` | 320x200 | 256 | IBM VGA 256-Color |
| **7** | `Hercules HGC` | 720x348 | 2 | Hercules Graphics Card |
| **8** | `IBM MDA` | 720x350 | 2 | IBM Monochrome Display |
| **9** | `C64 Hi-Res` | 320x200 | 16 | Commodore 64 High-Res |
| **10** | `C64 Multicolor` | 160x200 | 4 | Commodore 64 Multicolor |
| **11** | `ZX Spectrum` | 256x192 | 16 | Sinclair ZX Spectrum |
| **12** | `Atari 8-bit GR.8` | 320x192 | 2 | Atari 400/800 Graphics 8 |
| **13** | `Atari 8-bit GR.7` | 160x96 | 4 | Atari 400/800 Graphics 7 |
| **14** | `Atari ST Low` | 320x200 | 16 | Atari ST Low-Res |
| **15** | `Atari ST Medium` | 640x200 | 4 | Atari ST Medium-Res |
| **16** | `Atari ST High` | 640x400 | 2 | Atari ST High-Res Mono |
| **17** | `Apple II Hi-Res` | 280x192 | 6 | Apple II High-Resolution |
| **18** | `Amiga OCS 320` | 320x256 | 32 | Commodore Amiga 500 Low |
| **19** | `Amiga OCS 640` | 640x256 | 16 | Commodore Amiga 500 High |
| **20** | `MSX1 TMS9918` | 256x212 | 16 | MSX1 Home Computer |
| **21** | `Amstrad CPC M0` | 160x200 | 16 | Amstrad CPC Mode 0 |
| **22** | `BBC Micro Mode 1` | 160x256 | 8 | Acorn BBC Micro Mode 1 |
| **23** | `NES PPU NTSC` | 256x240 | 54 | Nintendo Entertainment System |
| **24** | `SNES PPU` | 256x224 | 256 | Super Nintendo Entertainment System |
| **25** | `Genesis NTSC` | 320x224 | 256 | Sega Genesis / Mega Drive |
| **26** | `Master System` | 256x192 | 64 | Sega Master System |
| **27** | `Game Boy Color` | 160x144 | 56 | Nintendo Game Boy Color |
| **28** | `Game Gear` | 160x144 | 32 | Sega Game Gear |
| **29** | `TurboGrafx-16` | 256x239 | 256 | NEC PC Engine / TurboGrafx |
| **30** | `Intellivision` | 159x96 | 16 | Mattel Intellivision |
| **31** | `ColecoVision` | 256x192 | 16 | ColecoVision |
| **32** | `Atari 2600 TIA` | 160x192 | 128 | Atari 2600 Video |
| **33** | `Atari 7800 MARIA`| 160x240 | 256 | Atari 7800 ProSystem |
| **34** | `Game Boy` | 160x144 | 4 | Nintendo Game Boy Mono |

## 5. GRAPHICS CONTROL STATEMENTS

- `PALETTE [attribute, color]`: Modifies color palette registers.
- `COLOR [foreground] [, [background] [, border]]`: Sets drawing and text color attributes.
- `PCOPY source_page, destination_page`: Copies video memory pages for double buffering.
- `VIEW [[SCREEN] (x1, y1)-(x2, y2) [, [fill_color] [, border_color]]]`: Restricts drawing output to a rectangular viewport.
- `WINDOW [[SCREEN] (x1, y1)-(x2, y2)]`: Configures Cartesian world coordinate scaling.
