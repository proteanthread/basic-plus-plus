 // ---
 // BASIC++ Interpreter - mod_stdlib.h
 // ---
 //
 // Standard Library Module header.
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

#ifndef BASICPP_MOD_STDLIB_H
#define BASICPP_MOD_STDLIB_H

 // mod_stdlib_register - Register the STDLIB module.
 //
 // Called from main.c during boot to register the standard
 // library module in the module system.
void mod_stdlib_register(void);

#endif // BASICPP_MOD_STDLIB_H
