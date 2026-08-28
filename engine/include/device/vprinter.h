// FILENAME: vprinter.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (print_using.c)
// NEEDED BY: libengine (context.c, exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, exec_interrupt.c, llist.c, lprint.c)
// NEEDED BY: libkernel (vprinter.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Implements virtual device and graphics rendering logic for vprinter.
//
// ---- Includes ----

#ifndef DEVICE_VPRINTER_H
#define DEVICE_VPRINTER_H

#include "device/vdev.h"
#include <stdbool.h>

// Initializes global virtual printer subsystem.
void vprinter_init(void);

// Shuts down virtual printer subsystem, releasing allocated memory.
void vprinter_shutdown(void);

// Sets the active program source file path to derive the output PDF filename (<name>.pdf).
// If NULL or empty, defaults to "OUTPUT.PDF".
void vprinter_set_program_path(const char *prog_path);

// Returns the current output PDF filename for the active print session.
const char *vprinter_get_output_filename(void);

// Writes a string directly to the virtual printer buffer.
void vprinter_write_str(const char *str);

// Writes a single character to the virtual printer buffer.
void vprinter_write_char(char c);

// Checks if any printer output has been accumulated in the active session.
bool vprinter_has_output(void);

// Flushes accumulated printer buffer and serializes the PDF file to disk.
// If override_filename is provided, saves to that path; otherwise uses default.
// Resets the session buffer after successful write.
// @return true if PDF was written, false if no output or write failure.
bool vprinter_flush_pdf(const char *override_filename);

// Resets the current printer buffer without writing to disk.
void vprinter_reset(void);

// Creates a VDev virtual device instance (e.g. "LPT1:" or "PRN:") for registration in VDevContext.
VDev vprinter_create_vdev(const char *dev_name);

#endif // DEVICE_VPRINTER_H
