# C17 API Reference: FujiNet Hardware Emulation (`device/fujinet.h`)

## 1. Subsystem Overview & Responsibilities

The FujiNet Subsystem (`device/fujinet.h`, implemented in `engine/src/device/fujinet.c`) provides peripheral bus emulation for the multi-platform FujiNet retro-computing network adapter across Atari 8-bit, Apple II, Commodore 64, and modern BASIC++ virtual devices.

Key architectural responsibilities include:
- **`N:` Network Adapter Virtual Device**: Connects open channels to remote TCP sockets, UDP datagrams, HTTP/HTTPS endpoints, JSON queries, and Telnet BBS sessions using FujiNet SIO protocol framing.
- **`FUJI:` Configuration Virtual Device**: Emulates the FujiNet management interface, allowing configuration of Wi-Fi credentials, host disk slot mappings (`D1:` .. `D8:`), and device firmware settings.
- **`CLOCK:` Network Time Virtual Device**: Retrieves network time protocol (NTP) timestamps, providing ISO datetime strings to retro-computing clock registers.
- **VDev Bus Integration**: Registers virtual devices directly with `VDevContext`, enabling standard BASIC I/O statements (`OPEN "N:HTTP://..."`, `INPUT#`, `PRINT#`) to access modern network protocols transparently.

## 2. Header Inclusion & Prerequisites

```c
#include "device/fujinet.h"
#include "device/vdev.h"
#include "vm/vm.h"
```

## 3. Function Prototypes & Operational Contracts

### System Lifecycle
```c
/**
 * @brief Initializes the FujiNet driver subsystem and binds network devices to VDevContext.
 * @param vm Pointer to active VMContext.
 */
void fujinet_init_system(VMContext *vm);

/**
 * @brief Shuts down the FujiNet subsystem, closing active network adapters and sockets.
 */
void fujinet_shutdown_system(void);
```

### Device Instantiation
```c
/**
 * @brief Creates the 'N:' network adapter virtual device struct.
 */
VDev fujinet_create_n_dev(VMContext *vm);

/**
 * @brief Creates the 'FUJI:' configuration virtual device struct.
 */
VDev fujinet_create_fuji_dev(VMContext *vm);

/**
 * @brief Creates the 'CLOCK:' network time virtual device struct.
 */
VDev fujinet_create_clock_dev(VMContext *vm);
```

## 4. Architectural Invariants

- **Sandboxed Network Capability**: Opening or communicating through FujiNet `N:` devices requires `CAP_NET` privilege in `SecurityContext`.
- **SIO Protocol Framing**: Translates higher-level HTTP/JSON calls to FujiNet SIO command frames (`0x4E` 'N' command set).

## 5. Code Example: Opening FujiNet N: Device from BASIC++

```basic
10 OPEN "N:HTTPS://api.github.com/zen" FOR INPUT AS #1
20 LINE INPUT #1, ZEN_QUOTE$
30 PRINT "GitHub Zen: "; ZEN_QUOTE$
40 CLOSE #1
```
