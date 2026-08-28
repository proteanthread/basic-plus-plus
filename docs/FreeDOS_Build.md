# BASIC++ v6.5.2 FreeDOS Build Guide

## 1. OVERVIEW

BASIC++ can be compiled for FreeDOS as a 16-bit real-mode executable. This build targets the BASIC_FREEDOS_16 profile with 640 KB conventional memory, 63-level stack depth, and direct BIOS INT 10h/13h/16h calls for I/O. The FreeDOS build produces a genuine DOS BASIC interpreter that runs on real hardware or in DOSBox.

## 2. TOOLCHAIN

The FreeDOS build uses the Open Watcom C/C++ compiler (version 2.0), which produces 16-bit DOS executables. Cross-compilation from a modern Linux or Windows host is fully supported.

Install Open Watcom: Download from https://github.com/open-watcom/open-watcom-v2

Set environment variables:
```bash
export WATCOM=/opt/watcom
export PATH=$WATCOM/binl64:$PATH
export INCLUDE=$WATCOM/h
```

## 3. BUILD CONFIGURATION

The FreeDOS build uses a separate CMake toolchain file:

```bash
mkdir build_dos
cd build_dos
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/dos16.cmake -DBASIC_FREEDOS_16=ON
cmake --build .
```

The BASIC_FREEDOS_16 flag enables the 16-bit memory profile and disables features that require 32/64-bit addresses or modern OS services.

## 4. MEMORY PROFILE

| Region | Size |
|--------|------|
| Program Memory | 32 KB |
| Variable Memory | 16 KB |
| String Heap | 16 KB |
| Scratch Area | 8 KB |

Stack depths: 63 (all stacks). Named variable limit: 128. DIM arrays: 32. Array elements: 2,048.

## 5. EXCLUDED FEATURES

The FreeDOS build excludes:

- SDL2 graphics (no SDL2 for DOS; BIOS INT 10h is used directly).
- Networking (no TCP/IP stack; FujiNet is unavailable).
- TUI editor multiplexer (no ncurses; line editor only).
- Background tasks (single-task DOS environment).
- Module system (no dynamic library loading).
- Segmented virtual memory (vmem).
- DAP debug server.
- Gemini protocol.

Attempting to use excluded features produces Error 73 (Advanced feature disabled).

## 6. BIOS DIRECT ACCESS

On FreeDOS, POKE, PEEK, INP, and OUT access real hardware registers and memory. The BIOS emulation layer is replaced by direct BIOS interrupt calls:

- INT 10h for video (CLS, LOCATE, COLOR, SCREEN modes).
- INT 13h for disk (not typically used; file I/O goes through DOS INT 21h).
- INT 16h for keyboard (INKEY$, INPUT).
- INT 1Ah for time (TIMER, TIME$, DATE$).

SCREEN mode changes use real BIOS mode setting. CGA modes (1, 2), EGA modes (7-10), and VGA modes (11-13) work on hardware that supports them.

## 7. FILE I/O

File I/O uses DOS INT 21h system calls through the platform abstraction layer. Path separators are backslashes. Filenames follow the 8.3 convention. Long filename support depends on the DOS version and LFN driver.

The maximum number of open files is limited by the DOS FILES= setting in CONFIG.SYS. BASIC++ requests up to 16 file handles.

## 8. RUNNING ON FREEDOS

Copy the compiled BASPP.EXE to the FreeDOS system. Run from the command prompt:

```
C:\> BASPP
BASIC++ Standard Edition v6.5.2
72 KB RAM Available.

Ok
>
```

The announced RAM is the available BASIC memory, not the total system memory. The actual banner shows the BASIC_FREEDOS_16 profile memory.

## 9. RUNNING IN DOSBOX

DOSBox provides a convenient way to test the FreeDOS build on modern systems:

```bash
dosbox -c "mount C /path/to/build" -c "C:" -c "BASPP.EXE"
```

DOSBox emulates CGA/EGA/VGA graphics modes, the PC speaker (SOUND, BEEP), and joystick input (STICK, STRIG).

## 10. GRAPHICS ON FREEDOS

SCREEN mode changes set real VGA registers. Mode 13 (320x200, 256 colors) writes directly to the VGA framebuffer at A000:0000. The BGI rasterizer operates on the real framebuffer rather than an SDL surface.

PALETTE changes write to the VGA DAC registers through port 3C8h/3C9h.
