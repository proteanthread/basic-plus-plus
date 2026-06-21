/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_usb.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    C-level modular expansions providing hardware wrappers and runtime libraries.
 *
 * 2. WHAT TO EXPECT:
 *    Modules register customized functions at boot to dynamically extend vocabulary.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Module naming, registered commands list, setup/shutdown details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Module lifecycle dispatcher, keyword override bindings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure mod_init does not fail. Verify that linkage matches build profiles.
 * ===================================================================== */

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
