// FILENAME: ast_parser.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (ast_internal.h)
// Implements AST parsing and evaluation structures for ast_parser.
//
// ---- Includes ----

#include "eval/ast_internal.h"

// All AST parsing implementation is decomposed into:
// - eval/ast/parser/ast_parse_expr.c: Arithmetic, relational, logical expression parsing
// - eval/ast/parser/ast_parse_stmt.c: Single-line statement parsing (LET, IF, FOR, etc.)
// - eval/ast/parser/ast_parse_block.c: Multi-line block compilation (IF, FOR, WHILE)
