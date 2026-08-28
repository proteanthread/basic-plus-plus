# BASIC++ v6.5.2 Device Discovery

## 1. OVERVIEW

Device discovery allows BASIC++ programs to query the available virtual devices at runtime. This is useful for programs that adapt their behavior based on which subsystems are present — for example, falling back to text mode when graphics are unavailable, or skipping network operations when VNet is disabled.

## 2. THE DEVICES COMMAND

DEVICES lists all registered virtual devices with their slot number, type, name, and status:

```basic
> DEVICES
Slot  Type      Name          Status
----  --------  -----------   ------
  0   VCon      Console       Active
  1   File      FileSystem    Active
  2   VNet      Network       Idle
  3   VFS       VirtFS        Active
  4   BGI       Graphics      Disabled
  5   BIOS      PCBios        Active
  6   FujiNet   FujiNet       Idle
```

Status values: Active (initialized and ready), Idle (registered but not currently in use), Disabled (compiled out or security-blocked).

## 3. PROGRAMMATIC DEVICE QUERIES

DEVICES$ returns the device list as a string that can be parsed programmatically.

DEVSTATUS$(slot) returns the status of the device in the specified slot as a string: "ACTIVE", "IDLE", "DISABLED", or "EMPTY".

DEVNAME$(slot) returns the name of the device in the specified slot.

DEVTYPE$(slot) returns the type of the device in the specified slot.

These functions allow programs to check for device availability before attempting to use them:

```basic
10 GraphicsAvailable = 0
20 FOR I = 0 TO 15
30   IF DEVTYPE$(I) = "BGI" AND DEVSTATUS$(I) <> "DISABLED" THEN
40     GraphicsAvailable = 1
50   END IF
60 NEXT I
70 IF GraphicsAvailable THEN
80   SCREEN 12
90 ELSE
100  PRINT "Running in text mode"
110 END IF
```

## 4. FEATURE DETECTION

Beyond device discovery, BASIC++ provides feature detection through the INFO command and related introspection functions:

VER$ returns the version string. DIALECT$ returns the active dialect name. MEMMAP$ returns the memory profile name.

The SUPPORT_* feature gates determine which subsystems are compiled into the current build. Programs can detect available features by attempting operations inside TRY/CATCH blocks:

```basic
10 TRY
20   SCREEN 0        ' Test if graphics are available
30   HasGraphics = 1
40 CATCH
50   HasGraphics = 0
60 END TRY
```

Error 73 (Advanced feature disabled) indicates that a feature gate has excluded the requested subsystem from the current build.

## 5. MOUNT AND UMOUNT

MOUNT path TO target creates a virtual device mount point. UMOUNT path removes it. Mount points are tracked by the VFS and displayed by DEVMAP:

```basic
> DEVMAP
Mount Points:
  A: -> /home/user/basic/drive_a
  B: -> /home/user/basic/drive_b
Device Names:
  CON: -> VCon
  NUL: -> Null
  LPT1: -> Printer
  N: -> FujiNet
```

## 6. DEVICE EVENTS

When a device status changes (connected, disconnected, error), a device event can be trapped:

```basic
10 ON DEVICE GOSUB 5000
20 DEVICE ON
```

Inside the handler, DEVSLOT returns the slot number of the device that triggered the event, and DEVEVENT$ returns the event type ("CONNECT", "DISCONNECT", "ERROR", "READY").

## 7. HOT-PLUGGING

Module-registered devices support hot-plugging. When a module is loaded (MODULE LOAD), its devices are registered and appear in DEVICES output. When a module is unloaded (MODULE UNLOAD), its devices are removed. Programs that monitor device events receive notifications for these changes.
