# Mock BIOS Porting Guide

The MockBiosContext within BASIC++ was designed as a modular, standalone C subsystem. While it currently serves as the backbone for the VirtualMachine architecture inside the interpreter, it is completely decoupled from the BASIC++ language parser and can be embedded into external emulators, C/C++ applications, DOSBox clones, or custom retro-computing projects.

## Architecture Overview

The MockBiosContext (located in source/standalone/mock_bios/) provides a simulated implementation of the IBM PC BIOS and DOS interrupt vectors (e.g., INT 10H for Video, INT 21H for DOS Services). It utilizes standard C17 conventions and maintains no dependencies on the Lexer, Parser, or Dialect structures of the host language.

The memory interaction is entirely abstracted through a SegmentedMemContext, meaning the BIOS can manipulate virtual RAM seamlessly without accessing physical memory.

## Exporting the Mock BIOS

To export the Mock BIOS to an external project, you need only the following components from the BASIC++ source tree:
*   mock_bios_core.c and mock_bios_core.h
*   segmented_mem.c and segmented_mem.h (for the memory abstraction layer)
*   The virtual device framework (dev.h, dev.c) if file I/O operations (INT 21H) are needed.

### Step 1: Initializing the Context

In your external application, include the core headers and allocate the contexts. The MockBiosContext requires a pointer to a SegmentedMemContext to operate.

`c
#include "mock_bios_core.h"
#include "segmented_mem.h"

// 1. Initialize a 1MB memory space
SegmentedMemContext mem;
segmem_init(&mem, 1024 * 1024);

// 2. Initialize the BIOS
MockBiosContext bios;
mock_bios_init(&bios, &mem, BIOS_IBM_PC);
`

### Step 2: Triggering Interrupts

The core function of the Mock BIOS is evaluating hardware and software interrupts as if it were a real CPU executing an INT instruction. You must map your emulator's virtual CPU registers into the BIOS execution function.

`c
// Example: Invoking INT 21H, AH=09H (Print String)
uint16_t ax = 0x0900;
uint16_t dx = 0x0100; // Offset of string in DS

// Execute the interrupt handler
mock_bios_execute_int(&bios, 0x21, &ax, /*bx=*/NULL, /*cx=*/NULL, &dx);
`

## Configuring DOS Handle Mapping

If you intend to support DOS file handles (INT 21H, AH=3DH/3EH/3FH), the MockBiosContext maps virtual handles via the VDevRegistry. In a standalone project, you must ensure that your external file manager hooks into the VFS or implements the standard dev_ioctl functions so the BIOS can translate virtual disk reads to physical host operations safely.
