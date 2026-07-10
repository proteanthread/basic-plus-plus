/* BASIC++ Help Database - Devices & Network */
#include "help.h"

const HelpEntry help_db_devices[] = {
    { "VDEV", "Virtual device introspection", "VDEV", HCAT_DEVICE },
    { "VMEM", "Virtual memory introspection", "VMEM", HCAT_DEVICE },
    { "VNET", "Virtual network introspection", "VNET", HCAT_DEVICE },
    { "VCON", "Virtual console info", "VCON", HCAT_DEVICE },
    { "VTERM", "Virtual terminal info", "VTERM", HCAT_DEVICE },
    { "VMACH", "Virtual machine info", "VMACH", HCAT_DEVICE },
    { "VPATH", "Virtual filesystem path setup", "VPATH \"vfs\"", HCAT_DEVICE },
    { "DEVMAP", "Map a device to an alias", "DEVMAP \"SCRN:\", \"CON:\"", HCAT_DEVICE },
    { "INP", "Read from a hardware port", "V = INP(port)", HCAT_DEVICE },
    { "OUT", "Write to a hardware port", "OUT port, value", HCAT_DEVICE },
    { "IOCTL", "Send control string to device", "IOCTL #1, \"BAUD=9600\"", HCAT_DEVICE },
    { "COM", "Enable/disable COM port events", "COM(1) ON", HCAT_DEVICE },
    { "WAIT", "Wait for hardware port state", "WAIT port, mask [, xor]", HCAT_DEVICE },
    { "BIOREAD", "Block I/O read", "BIOREAD #1, buf, len", HCAT_DEVICE },
    { "SIOREAD", "Stream I/O read", "SIOREAD #1, buf, len", HCAT_DEVICE },
    { "SIOREADLN", "Stream I/O read line", "SIOREADLN #1, buf", HCAT_DEVICE },
    { "BIOS", "Access mock BIOS interrupt", "BIOS int_num, regs", HCAT_DEVICE },
    { "FILEMOD", "Modify file attributes", "FILEMOD \"file.txt\", \"R\"", HCAT_DEVICE },
    { "IMAGE", "Manage disk images", "IMAGE MOUNT \"disk.img\"", HCAT_DEVICE },
    { "MOUNT", "Mount a VFS path", "MOUNT \"A:\", \"/mnt/floppy\"", HCAT_DEVICE },
    { "UMOUNT", "Unmount a VFS path", "UMOUNT \"A:\"", HCAT_DEVICE },
    { "NINFO", "Network info query", "NINFO \"interfaces\"", HCAT_DEVICE },
    { NULL, NULL, NULL, 0 }
};
