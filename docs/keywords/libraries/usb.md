# `usb` Virtual USB Device & Gamepad Controller Subsystem (`libhardware`)

## 1. Architectural Purpose & Overview

The `usb` subsystem (`engine/src/device/usb.c`) abstracts USB game controllers, joysticks, keyboards, and serial communications adapters for the virtual hardware bus.

### Key Architectural Invariants:
- **Gamepad Virtualization**: Maps modern USB analog sticks and gamepads to retro `JOY()`, `STICK()`, and `STRIG()` BASIC functions.
- **Hotplug Event Handling**: Automatically detects peripheral connection/disconnection without interrupting the VM loop.

---

## 2. Technical API Signatures (C17)

```c
void usb_init(void);
int usb_poll_gamepad(int player_id, GamepadState *state);
```
