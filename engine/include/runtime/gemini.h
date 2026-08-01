/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_gemini.h
 * @brief Gemini Protocol TLS Client API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares functions to fetch content from gemini:// capsules
 *   using TLS 1.2/1.3 client connections.
 * - Why it exists: Supports the GEMINI protocol network client interface (Phase 20).
 * - Why it works this way: It connects to port 1965, initiates a TLS handshake, sends the request line,
 *   and receives the status and body response payload.
 */

#ifndef BPP_GEMINI_H
#define BPP_GEMINI_H

#include "vm/vm.h"

/**
 * @brief Fetch the payload of a gemini:// URL.
 * @param vm The VM Context.
 * @param url The gemini:// URL.
 * @param out_err Pointer to retrieve any runtime errors.
 * @return The allocated string response payload (must be freed by caller), or NULL on failure.
 */
char *net_gemini_fetch(VMContext *vm, const char *url, BppError *out_err);

#endif /* BPP_GEMINI_H */
