// FILENAME: exec_postfix.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (exec_control_internal.h)
// Implements bytecode virtual machine execution and state for exec_postfix.
//
// ---- Includes ----

#include "vm/exec_control_internal.h"

//
// ---- Keyword Exemption Checker ----

bool is_postfix_exempt_keyword(BppKeywordId kw) {
    switch (kw) {
        case KW_IF:
        case KW_THEN:
        case KW_ELSE:
        case KW_FOR:
        case KW_NEXT:
        case KW_WHILE:
        case KW_WEND:
        case KW_UNTIL:
        case KW_DO:
        case KW_LOOP:
        case KW_REPEAT:
        case KW_ENDLOOP:
        case KW_SELECT:
        case KW_CASE:
        case KW_END:
        case KW_EXIT:
        case KW_WHEN:
        case KW_SUB:
        case KW_FUNCTION:
        case KW_DEF:
        case KW_TYPE:
        case KW_CLASS:
        case KW_MODULE:
        case KW_SCOPE:
        case KW_TRY:
        case KW_CATCH:
        case KW_RECORD:
        case KW_HANDLER:
        case KW_WITH:
            return true;
        default:
            return false;
    }
}
