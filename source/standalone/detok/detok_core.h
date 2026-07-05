/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: detok_core.h
 * Subsystem: Portable GWBASIC/QBASIC Token Detokenizer Core
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Maps internal tokens and keyword IDs back to plain text strings.
 *
 * 2. WHAT TO EXPECT:
 *    Safe string formatting, string length returns, and index boundaries checking.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Local buffer size limits.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Format signatures and keyword name arrays indexing map.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If a token formats incorrectly, verify the token type enum mappings.
 * ===================================================================== */

#ifndef STANDALONE_DETOK_CORE_H
#define STANDALONE_DETOK_CORE_H

#include "../../lexer.h"

int detok_core_token_to_string(const Token *tok, char *buf, int bufsize);
const char *detok_core_keyword_name(int keyword_id);

#endif /* STANDALONE_DETOK_CORE_H */
