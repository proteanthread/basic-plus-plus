// FILENAME: stmt_nfc.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_nfc.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Declares the NFC statement handler for PN532 / MFRC522 RFID card and tag operations.
//
// ---- Includes ----

#ifndef STMT_NFC_H
#define STMT_NFC_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Parses and executes the NFC statement and sub-commands.
BppError stmt_nfc_handler(VMContext *vm, LexerContext *lex);

// @brief Registers NFC metadata in runtime registry.
void stmt_nfc_register(void);

#ifdef __cplusplus
}
#endif

#endif // STMT_NFC_H
