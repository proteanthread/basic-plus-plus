// FILENAME: stmt_bluetooth.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_bluetooth.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Declares Bluetooth Classic (SPP) and BLE statement handlers.
//
// ---- Includes ----

#ifndef STMT_BLUETOOTH_H
#define STMT_BLUETOOTH_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Parses and executes the BT (Bluetooth Classic SPP) statement.
BppError stmt_bt_handler(VMContext *vm, LexerContext *lex);

// @brief Parses and executes the BLE (Bluetooth Low Energy) statement.
BppError stmt_ble_handler(VMContext *vm, LexerContext *lex);

// @brief Registers BT and BLE metadata in runtime registry.
void stmt_bt_register(void);
void stmt_ble_register(void);

#ifdef __cplusplus
}
#endif

#endif // STMT_BLUETOOTH_H
