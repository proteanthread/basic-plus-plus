/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_upnp.h
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
 // BASIC++ Interpreter - mod_upnp.h
 // ---
 //
 // UPnP/SSDP Device Discovery Module header.
 //
 // Registers UPnP discovery and control VDevs for
 // network device enumeration via SSDP multicast.
 // Activated via MODULE "UPNP" from BASIC.
 //
 // This is a pure API framework. Network I/O uses
 // platform sockets (Winsock on Windows, BSD sockets
 // on Linux, stub on FreeDOS).
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

#ifndef BASICPP_MOD_UPNP_H
#define BASICPP_MOD_UPNP_H

 // mod_upnp_register - Register the UPnP module.
 //
 // Called from main.c during boot to register the UPnP
 // device module in the module system. Does not activate -
 // the user must call MODULE "UPNP" from BASIC, or the
 // caller must call module_activate("UPNP", rt).
void mod_upnp_register(void);

#endif // BASICPP_MOD_UPNP_H
