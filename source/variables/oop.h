/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: oop.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Object-Oriented Programming (OOP) class parsing and structures.
 * ===================================================================== */

#ifndef BASICPP_OOP_H
#define BASICPP_OOP_H

#include "lexer.h"
#include "runtime.h"

void pi_parse_class(Lexer *lex, RuntimeState *rt, int line_num);
void pi_parse_endclass(Lexer *lex, RuntimeState *rt, int line_num);

#endif // BASICPP_OOP_H
