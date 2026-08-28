# BASIC++ v6.5.2 Module System Architecture

## 1. OVERVIEW

The module system allows BASIC++ programs to load, validate, and activate extension modules at runtime. Modules can add new keywords, functions, statement handlers, and virtual device drivers without modifying the core interpreter. The system enforces a strict security pipeline: modules are validated before loading, their capabilities are verified against the active security level, and they execute within a sandboxed environment.

## 2. MODULE LIFECYCLE

A module passes through five stages before it becomes active:

1. **Validation** — The module file is read and its header is checked for format correctness, version compatibility, and integrity. Invalid modules are rejected with an error message.

2. **Capability Verification** — The module declares which operations it requires (file access, network, memory, device registration). These capabilities are checked against the active security level via security_module_allowed(capabilities). If any required capability is denied, the module is rejected.

3. **Sandbox Allocation** — A sandboxed execution environment is created for the module. The sandbox limits the module's access to the VM internals and prevents direct modification of VM instructions, execution of host code bypasses, or corruption of internal stacks.

4. **Registration** — The module's keywords, functions, and handlers are registered with the StmtRegistry and function registry. New BppKeywordId values are allocated via keyword_register_custom(name).

5. **Activation** — The module is marked as active and its keywords become available to the parser. The module appears in MODULE LIST output and CATALOG.

## 3. LOADING AND UNLOADING

MODULE LOAD "name" loads a module by name. The interpreter searches for the module file in the module search path. MODULE LOAD supports optional arguments: MODULE LOAD "name", OPTIONS.

MODULE UNLOAD "name" deactivates a module and removes its registered keywords and handlers. Open resources owned by the module are cleaned up during unload.

MODULE LIST displays all currently loaded modules with their names, versions, and capability sets.

MODULE INFO "name" displays detailed information about a specific module.

## 4. BUILT-IN MODULES

The following modules ship with BASIC++ and can be loaded on demand:

**mathext** — Extended mathematics functions including hyperbolic functions, inverse trigonometric functions, statistical aggregates (MIN, MAX, AVG, MED), and additional numeric utilities. Implemented in engine/src/module/mathext.c.

**arrayext** — Extended array operations including ARRAY SORT, ARRAY REVERSE, ARRAY SHUFFLE, ARRAY SEARCH, FILTER, REDUCE, and UNPACK. Implemented in engine/src/module/arrayext.c.

**regex** — Regular expression pattern matching using the platform regex engine. Implemented in engine/src/module/regex.c.

## 5. MODULE SECURITY CONSTRAINTS

Modules operate under strict security constraints:

- Modules cannot directly modify VM instructions or the bytecode execution loop.
- Modules cannot execute arbitrary host code (no dlsym of system functions).
- Modules cannot corrupt internal stacks (GosubStack, ForStack, etc.).
- Modules cannot bypass the security system's operation checks.
- Module file access is subject to the same security level restrictions as the host program.

The security_module_allowed() function checks the module's capability bitmask against the active security level. A module that requires network access will be rejected at EDUCATIONAL or higher security levels where SECOP_NETWORK is denied.

## 6. CUSTOM MODULE DEVELOPMENT

Custom modules are implemented as C17 source files that link against the BASIC++ engine headers. A module must provide:

1. A module descriptor structure with the module name, version, and capability requirements.
2. An initialization function that registers keywords and handlers.
3. A shutdown function that cleans up resources.

The module registration API uses the same StmtRegistry and function registry APIs that built-in statements use. This ensures that module-provided keywords behave identically to built-in keywords from the user's perspective.

## 7. THE MODULE SEARCH PATH

When MODULE LOAD is called with a module name, the interpreter searches the following locations in order:

1. The current working directory.
2. The directory containing the currently running program.
3. The BASIC++ installation directory's modules/ subdirectory.
4. Directories listed in the BASICPATH environment variable.

The first match found is loaded. If no match is found, Error 53 (File not found) is raised.
