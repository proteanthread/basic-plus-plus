# `mobile` Mobile & Handheld Runtime Profile

## 1. Architectural Purpose & Overview

The `mobile` runtime profile adapts the BASIC++ engine for mobile platforms (Android NDK, iOS, handheld gaming devices), providing virtual touch keyboard input, scaled display presentation, and battery-conscious execution loops.

### Key Architectural Specifications:
- **Display Virtualization**: High-DPI screen auto-scaling with aspect ratio correction.
- **Touch Event Bus**: Maps multi-touch gestures to mouse/joystick events (`PEN`, `STICK`, `STRIG`).
- **Power Management**: Throttles VM loop when idle to preserve battery life.
- **Virtual Keyboards**: On-screen soft keyboard interaction hooks for REPL and `INPUT`.

---

## 2. Platform Flags

```c
#define BPP_TARGET_MOBILE 1
#define TOUCH_VIRTUALIZATION 1
```
