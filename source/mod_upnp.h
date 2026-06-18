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
