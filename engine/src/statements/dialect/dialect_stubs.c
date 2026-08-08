/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file dialect_stubs.c
 * @brief Headless fallback stubs placeholder for interactive dialect statements.
 *
 * All dialect statement handlers (HELP, CATALOG, SELFTEST, REMOVE, HOSTNAME, USERNAME)
 * are now fully implemented in dedicated statement micro-libraries (help.c, selftest.c,
 * remove.c, introspection.c). This file is retained as a placeholder for ABI compatibility.
 */

typedef int bpp_dialect_stubs_dummy_t;
