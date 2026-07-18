/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: module.h
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
 // BASIC++ Interpreter - module.h
 // ---
 //
 // Module System interface.
 //
 // PURPOSE:
 // Provides a unified framework for packaging and managing
 // interpreter extensions. Modules group related functions,
 // devices, and dialect extensions into named, versioned units
 // with declared capabilities.
 //
 // MODULE CLASSES:
 // MOD_LIBRARY - Function library (registers via funcreg)
 // MOD_DIALECT - Dialect extension (configures via dialect)
 // MOD_DEVICE - Virtual device driver (registers via vdev)
 // MOD_EXTENSION - General extension (any combination)
 //
 // LIFECYCLE:
 // 1. module_system_init() - clears the module table
 // 2. module_register() - registers a module descriptor
 // 3. module_activate() - calls the module's init callback
 // 4. module_deactivate() - calls cleanup, marks inactive
 //
 // CAPABILITY FLAGS:
 // Each module declares what system resources it needs via
 // a bitfield. (Security) will gate activation
 // based on these capabilities.
 //
 // ---

#ifndef BASICPP_MODULE_H
#define BASICPP_MODULE_H

#include "config.h"

// --- Module Classes ---
typedef enum ModuleClass {
 MOD_LIBRARY = 0, // function library
 MOD_DIALECT = 1, // dialect extension
 MOD_DEVICE = 2, // virtual device driver
 MOD_EXTENSION = 3 // general extension
} ModuleClass;

// --- Capability Flags (bitfield) ---
 // Declares what system resources a module uses. These are
 // informational in and will be enforced in 
#define CAP_NONE 0x0000u
#define CAP_MATH 0x0001u // mathematical functions
#define CAP_STRING 0x0002u // string manipulation
#define CAP_IO 0x0004u // I/O operations
#define CAP_FILE 0x0008u // file system access
#define CAP_SYSTEM 0x0010u // system-level operations
#define CAP_GRAPHICS 0x0020u // graphics device
#define CAP_SOUND 0x0040u // sound device
#define CAP_NETWORK 0x0080u // network (TCP, HTTP, etc.)
// Modern device capabilities
#define CAP_GPIO 0x0100u // GPIO / digital I/O pins
#define CAP_I2C 0x0200u // I2C bus access
#define CAP_SPI 0x0400u // SPI bus access
#define CAP_SENSOR 0x0800u // sensor hardware (accel, GPS)
#define CAP_CAMERA 0x1000u // camera / video capture
#define CAP_BLUETOOTH 0x2000u // Bluetooth / BLE
#define CAP_USB 0x4000u // USB device access

// --- Module Descriptor ---
 // Static metadata for a registered module. All strings are
 // expected to be string literals (not heap-allocated).
typedef struct ModuleInfo {
 const char *name; // display name (e.g., "STDLIB")
 const char *version; // version string (e.g., "1.0")
 const char *description; // one-line description
 ModuleClass mod_class; // module class
 unsigned int capabilities; // CAP_ bitfield
 int (*init)(void *); // init callback (rt=RuntimeState*)
 void (*cleanup)(void); // cleanup callback (may be NULL)
} ModuleInfo;

// --- Module System API ---

 // module_system_init - Initialize the module system.
 //
 // Clears the module table. Must be called once at boot before
 // any modules are registered.
void module_system_init(void);

 // module_count - Return the number of registered modules.
 //
 // Used by boot diagnostics and INFO command.
int module_count(void);

 // module_register - Register a module descriptor.
 //
 // Stores the module info in the table. The module starts
 // in the INACTIVE state. Call module_activate() to init it.
 //
 // Returns 0 on success, -1 if the table is full.
int module_register(const ModuleInfo *info);

 // module_activate - Activate a registered module by name.
 //
 // Calls the module's init callback. If already active, this
 // is a no-op (idempotent).
 //
 // Parameters:
 // name - module name (case-insensitive)
 // rt - opaque pointer to RuntimeState (passed to init)
 //
 // Returns 0 on success, -1 if not found or init failed.
int module_activate(const char *name, void *rt);

 // module_deactivate - Deactivate a module by name.
 //
 // Calls the module's cleanup callback (if any) and marks
 // the module as inactive.
 //
 // Returns 0 on success, -1 if not found or not active.
int module_deactivate(const char *name);

 // module_is_active - Check if a module is active.
 //
 // Returns 1 if active, 0 if inactive or not found.
int module_is_active(const char *name);

 // module_find - Find a module by name.
 //
 // Returns a pointer to the ModuleInfo, or NULL if not found.
 // Name comparison is case-insensitive.
const ModuleInfo *module_find(const char *name);

 // module_count - Return the number of registered modules.
int module_count(void);

 // module_get - Get a module by index.
 //
 // Returns a pointer to the ModuleInfo at the given index,
 // or NULL if the index is out of range.
const ModuleInfo *module_get(int index);

 // module_is_loaded - Check if a module at the given index is active.
 //
 // Returns 1 if active, 0 if inactive.
int module_is_loaded(int index);

 // module_class_name - Get human-readable class name.
const char *module_class_name(ModuleClass cls);

 // module_caps_string - Format capability flags as a short string.
 //
 // Writes abbreviated capability letters into buf (max buf_len).
 // Example: CAP_MATH|CAP_STRING -> "MS"
void module_caps_string(unsigned int caps, char *buf, int buf_len);

 // module_load_dynamic - Load an external shared library.
 //
 // Loads a .dll (Windows) or .so (Linux) and executes its
 // bpp_module_init() function to register itself.
int module_load_dynamic(const char *path);

#endif // BASICPP_MODULE_H
