# BASIC++ Architectural Flowcharts & Execution Reference

This document maps out the end-to-end execution flow, state management, virtualization subsystems, and peripheral routing pipelines of the BASIC++ interpreter and compiler.

---

## 1. System Core Overview (User Space vs. System Space)

This diagram shows the structural layers of BASIC++. It maps how a user interacts with the front-end interface, how the interpreter engine processes that interaction, and how the core virtualization layers bridge BASIC code to physical hardware.

```mermaid
graph TD
    subgraph user_view ["What the User Sees"]
        A1["CLI Console Banner / Prompt"]
        A2["Interactive REPL Command Line"]
        A3["Dialect Ready Prompts (Ok, READY, >, &#93; )"]
    end

    subgraph interpreter_view ["What the System Sees"]
        B1["Main Dispatch (core/main.c)"]
        B2["Memory Pools (core/memory.c)"]
        B3["Dialect Configurations (dialect/dialect.c)"]
        B4["Security Sandbox (core/security.c)"]
    end

    subgraph hardware_virtualization ["Virtual Device Layer"]
        C1["VDev Registry (virtual/vdev.c)"]
        C2["VFS Sandboxed Filesystem (io/vfs.c)"]
        C3["MEMMAP Emulated Layouts (memory/memmap.c)"]
        C4["USB & Serial Controllers (modules/mod_usb.c)"]
    end

    A2 -->|Inputs Line| B1
    B1 -->|Allocates State| B2
    B1 -->|Applies Filter| B3
    B1 -->|Gates Access| B4
    B1 -->|Accesses Hardware| C1
    C1 -->|Files| C2
    C1 -->|Memory PEEK/POKE| C3
    C1 -->|USB HID/Serial| C4
```

---

## 2. Boot & Initialization (What the System Sees)

When `basicpp.exe` launches, it bootstraps the system state before opening the REPL loop. This sequence describes the initialization of static memory pools, the loading of configuration directives, and the registry of standard libraries.

```mermaid
graph TD
    A["System Start: main()"] --> B["Initialize Platform Info (core/platform.c)"]
    B --> C["Allocate Fixed Memory Pools (core/memory.c)"]
    C --> D["Set Security Level 0 (core/security.c)"]
    D --> E["Load Configuration File: basicpp.cfg"]
    E --> F["Register Standard Functions Table (functions/builtins.c)"]
    F --> G["Register Core Subsystem Modules (modules/module.c)"]
    G --> H["Initialize Default Dialect (GWBS / BPP)"]
    H --> I["Start REPL Event Loop / Command Wait"]
```

---

## 3. Interactive REPL Loop (What the User Sees)

This diagram maps how the interpreter parses, stores, or executes line statements entered by the user in real-time.

```mermaid
graph TD
    A["User Enters Line"] --> B["Tokenizer Scan (lexer/lexer.c)"]
    B --> C{"Does Line Start with a Digit?"}
    
    C -->|Yes: Stored Mode| D["Validate Target Line Number Syntax"]
    D --> E["Insert/Overwrite line in Program Storage Pool"]
    E --> F["Re-sort program by line number order"]
    F --> G["Return READY prompt to user"]
    
    C -->|No: Immediate Mode| H["Statement Parser (parser/parser.c)"]
    H --> I["Evaluate & Execute Statement immediately"]
    I --> J{"Does command result in error?"}
    J -->|Yes| K["Raise non-fatal error status (core/errors.c)"]
    J -->|No| L["Output result (if PRINT)"]
    K --> G
    L --> G
```

---

## 4. Flow Control & Event Trapping

BASIC++ handles program flow structures via nested control frames stored in the execution stack. Hardware interrupts are trapped and handled in-between statements.

```mermaid
graph TD
    A["Execute Next Statement"] --> B{"Is Keyword Flow Control?"}
    
    B -->|GOTO / GOSUB| C["Push return pointer to Call Stack (if GOSUB)"]
    C --> D["Update program counter index to target line number"]
    
    B -->|FOR / WHILE / DO| E["Push Loop Control Frame to stack"]
    E --> F["Check loop condition bounds"]
    F -->|Satisfied| G["Execute loop body statements"]
    F -->|Limit Exceeded| H["Pop Loop Frame and jump past loop terminator"]
    
    B -->|Standard Statement| I["Execute statement core logic"]
    I --> J["Poll System Event Interrupt Flags"]
    J --> K{"Is Trap Triggered? (TIMER/KEY)"}
    K -->|Yes| L["Lookup handler pointer (ON TIMER GOSUB)"]
    L --> M["Push current pointer, execute trap handler line"]
    K -->|No| N["Advance program counter to next statement"]
```

---

## 5. Dialect Selection & Feature Gating

BASIC++ supports 16 dialects. It dynamically modifies the parser's active keyword tokens and separators based on the configured profile.

```mermaid
graph TD
    A["Select Dialect (e.g. DIALECT 'PATB')"] --> B["Lookup DialectConfig struct (dialect/dialect.c)"]
    B --> C["Update Ready Prompt Layout"]
    C --> D["Set print zone column boundaries"]
    D --> E["Set statement separator (colons vs semicolons)"]
    E --> F{"Is STRICT Mode Enabled?"}
    
    F -->|Yes| G["Mask off all keywords not native to the dialect"]
    F -->|No: Union Mode| H["Expose the combined 223-keyword set"]
    G --> I["Update tokenizer active keyword index table"]
    H --> I
```

---

## 6. Virtual Device (VDev) Registry & Hardware Routing

The Virtual Device Layer (`VDev`) acts as the driver layer of BASIC++. It maps standard I/O commands to physical drivers.

```mermaid
graph TD
    subgraph basic_io ["BASIC I/O Commands"]
        A1["OPEN 'device:' FOR mode AS #ch"]
        A2["PRINT #ch, 'data'"]
        A3["GET #ch, record"]
        A4["CLOSE #ch"]
    end

    subgraph vdev_slot ["VDev Slot Management"]
        B1["Device File Descriptor Mapping (vdev.c)"]
        B2["Find registered driver by name prefix"]
    end

    subgraph active_drivers ["Active Driver Callbacks"]
        C1["dev_con: Console (Standard I/O)"]
        C2["dev_file: Virtual Filesystem (VFS)"]
        C3["dev_net: Sockets (TCP/UDP, NTP)"]
        C4["dev_lpt: Printer Output"]
        C5["dev_usb: Serial / HID Gamepads"]
    end

    A1 --> B1
    A2 --> B1
    A3 --> B1
    A4 --> B1
    B1 --> B2
    B2 -->|'CON:'| C1
    B2 -->|'FILE:'| C2
    B2 -->|'TCP:' / 'UDP:' / 'CLOCK:'| C3
    B2 -->|'LPT:'| C4
    B2 -->|'USB:' / 'HID:'| C5
```

---

## 7. USB Device Controller & HID Support

The optional USB module interfaces the VDev layer to physical USB serial ports and game controllers.

```mermaid
graph TD
    A["OPEN 'USB:COM3' / 'HID:1' AS #ch"] --> B["MODULE 'USB' Active Check (mod_usb.c)"]
    B --> C["Identify USB controller interface class"]
    C --> D{"Is HID or Serial Device?"}
    
    D -->|USB Serial: FTDI / Arduino| E["Map read/write callbacks to hardware COM port APIs"]
    E --> F["PRINT #ch writes TX packet | INPUT #ch reads RX queue"]
    
    D -->|USB HID: Gamepad / Joystick| G["Register joystick event polling callbacks"]
    G --> H["dev_ioctl reads axis positions, button bitmasks"]
```

---

## 8. Virtual Filesystem (VFS) & Sandboxing

All local file interactions are routed through a sandboxed Virtual Filesystem (`VFS`) to enforce path security rules.

```mermaid
graph TD
    A["File System Access (e.g. LOAD 'C:\TEMP\PROG.BAS')"] --> B["Get current Security Level (core/security.c)"]
    B --> C{"Check Security Gating"}
    
    C -->|Level 2: RESTRICTED| D["Raise 'Permission Denied' runtime error"]
    C -->|Level 1 / 0| E["Run path validation logic (io/vfs.c)"]
    
    E --> F{"Does path exit project root?"}
    F -->|Yes| G["Enforce directory jail: raise 'File not found' / 'Access Denied'"]
    F -->|No| H["Call host OS native file system handles (fopen)"]
```

---

## 9. Virtual Memory Maps (MEMMAP Presets)

The PEEK/POKE system operates inside a segmented memory model. Setting `MEMMAP` configures virtual memory locations to simulate legacy hardware platforms.

```mermaid
graph TD
    A["POKE address, value"] --> B["Resolve active Segment (set via DEF SEG)"]
    B --> C["Locate Offset in Virtual Memory Array (memory/memmap.c)"]
    C --> D{"Which MEMMAP is active?"}
    
    D -->|MSDOS| E["Address offsets map to x86 interrupt tables & BIOS variables"]
    D -->|C64| F["Address offsets map to Commodore 64 VIC-II registers & color RAM"]
    D -->|APPLE2| G["Address offsets map to Apple II page zero and screen soft switches"]
    
    E --> H["Perform read/write offset bounds validation"]
    F --> H
    G --> H
    H --> I["Update target cell in virtual memory pool"]
```

---

## 10. Virtual Terminal, Consoles & Framebuffer

The graphics engine renders color pixel grids to standard text terminals. It maps pixel matrices to Unicode characters.

```mermaid
graph TD
    A["Graphics command: PSET (x, y), color"] --> B["Validate coordinates against active view window"]
    B --> C["Write color value to Framebuffer (graphics/gfxbuf.c)"]
    C --> D["SCREEN rendering trigger (or PRINT CLS)"]
    
    D --> E["Walk 320x200 pixel matrix in 2-pixel vertical slices"]
    E --> F["Map upper/lower pixels to Unicode Half-Block characters (U+2580/U+2584)"]
    F --> G["Inject ANSI foreground/background escape sequences for color indices"]
    G --> H["Write formatted Unicode stream to dev_con stdout stream"]
    H --> I["Terminal displays crisp 16-color graphics in standard text window"]
```

---

## 11. Maintenance Reference

### What We Can Change
- **Keywords Registry Table**: New keywords or system operations can be registered alphabetically in `functions/builtins.c` and documented in `help/help.c`.
- **Dialect Profiles Definitions**: Dialect-specific constraints (READY prompts, zone widths, separator styles) can be modified inside their respective profiles in the `dialect/` folder.
- **Hardware Drivers Callbacks**: Custom VDev driver mappings (e.g. LCD display devices, sensor drivers) can be registered inside `virtual/vdev.c`.

### What We Cannot Change
- **Core Pool Allocators**: The bump-allocation architecture in `core/memory.c` handles all program statements and variables statically. Modifying its allocator will corrupt variable indexing.
- **Token Index Sequences**: The ordering of token enumerations in `lexer.h` must match the lexical parsing dispatch table logic exactly.
- **RuntimeState Layout**: The `RuntimeState` struct in `runtime.h` contains the active call stacks, file channel indexes, and variable frames. Its structure must remain intact for correct VM operations.

### Troubleshooting
- **Stack Overflow**: If deep recursion in `SUB` or `FUNCTION` blocks causes a crash, increase `MAX_STACK_DEPTH` in `config.h`.
- **String Memory Depletion**: If a long-running string utility crashes with out-of-memory errors, check allocations and call `FRE(0)` to force string pool recycling.
- **Garbled Terminal Display**: If the Unicode framebuffer renders poorly, check that the host shell console is set to UTF-8 mode and supports standard 16-color ANSI escape sequences.
