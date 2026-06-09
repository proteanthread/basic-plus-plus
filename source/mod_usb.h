/*
 * =====================================================================
 * BASIC++ Interpreter - mod_usb.h
 * =====================================================================
 *
 * USB Device Module header (Phase 16).
 *
 * Registers USB HID, serial, and storage device VDevs.
 * Activated via MODULE "USB" from BASIC.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#ifndef BASICPP_MOD_USB_H
#define BASICPP_MOD_USB_H

/*
 * mod_usb_register - Register the USB module.
 *
 * Called from main.c during boot to register the USB device
 * module in the module system. Does not activate — the user
 * must call MODULE "USB" from BASIC, or the caller must
 * call module_activate("USB", rt).
 */
void mod_usb_register(void);

#endif /* BASICPP_MOD_USB_H */
