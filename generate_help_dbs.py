import os

progmgmt_content = """/* BASIC++ Help Database - Program Management & Editing */
#include "help.h"

const HelpEntry help_db_progmgmt[] = {
    { "RUN", "Execute the stored program", "RUN [line]", HCAT_PROGMGMT },
    { "LIST", "List stored program lines", "LIST [start]-[end]", HCAT_PROGMGMT },
    { "LLIST", "Print stored program lines to LPT", "LLIST [start]-[end]", HCAT_PROGMGMT },
    { "SAVE", "Save program to disk", "SAVE \\"file.bas\\"", HCAT_PROGMGMT },
    { "LOAD", "Load program from disk", "LOAD \\"file.bas\\"", HCAT_PROGMGMT },
    { "NEW", "Clear program and variables", "NEW", HCAT_PROGMGMT },
    { "MERGE", "Merge program lines from disk", "MERGE \\"file.bas\\"", HCAT_PROGMGMT },
    { "CHAIN", "Load and execute program, passing vars", "CHAIN \\"file.bas\\"", HCAT_PROGMGMT },
    { "BSAVE", "Save binary memory image", "BSAVE \\"file.bin\\", start, length", HCAT_PROGMGMT },
    { "BLOAD", "Load binary memory image", "BLOAD \\"file.bin\\" [, offset]", HCAT_PROGMGMT },
    { "BRUN", "Load and execute binary program", "BRUN \\"file.bin\\"", HCAT_PROGMGMT },
    { "CSAVE", "Save program to cassette", "CSAVE \\"PROG\\"", HCAT_PROGMGMT },
    { "CLOAD", "Load program from cassette", "CLOAD \\"PROG\\"", HCAT_PROGMGMT },
    { "CRUN", "Load and run from cassette", "CRUN", HCAT_PROGMGMT },
    { "UNLOAD", "Unload module or program", "UNLOAD", HCAT_PROGMGMT },
    { "UNSAVE", "Delete saved program (alias for KILL)", "UNSAVE \\"file.bas\\"", HCAT_PROGMGMT },
    { "AUTO", "Automatic line numbering", "AUTO [start] [, step]", HCAT_EDITING },
    { "DELETE", "Delete program lines", "DELETE [start]-[end]", HCAT_EDITING },
    { "RENUM", "Renumber program lines", "RENUM [newstart] [, [oldstart] [, step]]", HCAT_EDITING },
    { "EDIT", "Edit a program line", "EDIT line", HCAT_EDITING },
    { NULL, NULL, NULL, 0 }
};
"""

testing_content = """/* BASIC++ Help Database - Debugging & Testing */
#include "help.h"

const HelpEntry help_db_testing[] = {
    { "ASSERT", "Assert a condition is true", "ASSERT A = 5, \\"A must be 5\\"", HCAT_DEBUG_TEST },
    { "BREAK", "Set a breakpoint or pause", "BREAK", HCAT_DEBUG_TEST },
    { "CONT", "Continue after STOP or BREAK", "CONT", HCAT_DEBUG_TEST },
    { "BACKTRACE", "Print the call stack", "BACKTRACE", HCAT_DEBUG_TEST },
    { "CHECK", "Check internal state", "CHECK", HCAT_DEBUG_TEST },
    { "DEBUG", "Enter debug mode", "DEBUG ON", HCAT_DEBUG_TEST },
    { "DUMP", "Dump variables or memory", "DUMP", HCAT_DEBUG_TEST },
    { "ENDTEST", "End a test block", "ENDTEST", HCAT_DEBUG_TEST },
    { "SELFTEST", "Run built-in interpreter tests", "SELFTEST", HCAT_DEBUG_TEST },
    { "TEST", "Begin a test block", "TEST \\"Math functions\\"", HCAT_DEBUG_TEST },
    { "TRACE", "Enable detailed tracing", "TRACE ON", HCAT_DEBUG_TEST },
    { "TROFF", "Disable execution tracing", "TROFF", HCAT_DEBUG_TEST },
    { "TRON", "Enable execution tracing", "TRON", HCAT_DEBUG_TEST },
    { "VARS", "List all defined variables", "VARS", HCAT_DEBUG_TEST },
    { "VERIFY", "Verify program integrity", "VERIFY", HCAT_DEBUG_TEST },
    { NULL, NULL, NULL, 0 }
};
"""

devices_content = """/* BASIC++ Help Database - Devices & Network */
#include "help.h"

const HelpEntry help_db_devices[] = {
    { "VDEV", "Virtual device introspection", "VDEV", HCAT_DEVICES_NET },
    { "VMEM", "Virtual memory introspection", "VMEM", HCAT_DEVICES_NET },
    { "VNET", "Virtual network introspection", "VNET", HCAT_DEVICES_NET },
    { "VCON", "Virtual console info", "VCON", HCAT_DEVICES_NET },
    { "VTERM", "Virtual terminal info", "VTERM", HCAT_DEVICES_NET },
    { "VMACH", "Virtual machine info", "VMACH", HCAT_DEVICES_NET },
    { "VPATH", "Virtual filesystem path setup", "VPATH \\"vfs\\"", HCAT_DEVICES_NET },
    { "DEVMAP", "Map a device to an alias", "DEVMAP \\"SCRN:\\", \\"CON:\\"", HCAT_DEVICES_NET },
    { "INP", "Read from a hardware port", "V = INP(port)", HCAT_DEVICES_NET },
    { "OUT", "Write to a hardware port", "OUT port, value", HCAT_DEVICES_NET },
    { "IOCTL", "Send control string to device", "IOCTL #1, \\"BAUD=9600\\"", HCAT_DEVICES_NET },
    { "COM", "Enable/disable COM port events", "COM(1) ON", HCAT_DEVICES_NET },
    { "WAIT", "Wait for hardware port state", "WAIT port, mask [, xor]", HCAT_DEVICES_NET },
    { "BIOREAD", "Block I/O read", "BIOREAD #1, buf, len", HCAT_DEVICES_NET },
    { "SIOREAD", "Stream I/O read", "SIOREAD #1, buf, len", HCAT_DEVICES_NET },
    { "SIOREADLN", "Stream I/O read line", "SIOREADLN #1, buf", HCAT_DEVICES_NET },
    { "BIOS", "Access mock BIOS interrupt", "BIOS int_num, regs", HCAT_DEVICES_NET },
    { "FILEMOD", "Modify file attributes", "FILEMOD \\"file.txt\\", \\"R\\"", HCAT_DEVICES_NET },
    { "IMAGE", "Manage disk images", "IMAGE MOUNT \\"disk.img\\"", HCAT_DEVICES_NET },
    { "MOUNT", "Mount a VFS path", "MOUNT \\"A:\\", \\"/mnt/floppy\\"", HCAT_DEVICES_NET },
    { "UMOUNT", "Unmount a VFS path", "UMOUNT \\"A:\\"", HCAT_DEVICES_NET },
    { "NINFO", "Network info query", "NINFO \\"interfaces\\"", HCAT_DEVICES_NET },
    { NULL, NULL, NULL, 0 }
};
"""

with open('source/help/help_progmgmt.c', 'w') as f: f.write(progmgmt_content)
with open('source/help/help_testing.c', 'w') as f: f.write(testing_content)
with open('source/help/help_devices.c', 'w') as f: f.write(devices_content)
