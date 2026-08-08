/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file module_stmt.c
 * @brief Note on MODULE statement handler implementation location.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Serves as a landmark notice for statement handler layout.
 * - Why it exists: The authoritative MODULE statement handler is defined in engine/src/statements/oop/module.c
 *   which compiles into the stmt_module micro-library target.
 * - Why it works this way: Single responsibility invariant for statement target objects.
 */

typedef int bpp_module_stmt_dummy;
