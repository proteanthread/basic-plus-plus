 // ---
 // BASIC++ Interpreter - mod_usb.h
 // ---
 //
 // USB Device Module header.
 //
 // Registers USB HID, serial, and storage device VDevs.
 // Activated via MODULE "USB" from BASIC.
 //
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
 // ---

#ifndef BASICPP_MOD_USB_H
#define BASICPP_MOD_USB_H

 // mod_usb_register - Register the USB module.
 //
 // Called from main.c during boot to register the USB device
 // module in the module system. Does not activate - the user
 // must call MODULE "USB" from BASIC, or the caller must
 // call module_activate("USB", rt).
void mod_usb_register(void);

#endif // BASICPP_MOD_USB_H
