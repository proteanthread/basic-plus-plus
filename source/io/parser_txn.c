 // ---
 // BASIC++ Interpreter - parser_txn.c
 // ---
 //
 // Parser handlers for ATOMIC / TXN transaction control.
 //
 // STATEMENTS:
 //   ATOMIC               - begin an atomic block (auto-rollback)
 //   END ATOMIC           - commit and end atomic block
 //   TXN BEGIN             - begin explicit transaction
 //   TXN COMMIT / COMMIT  - commit and end transaction
 //   TXN ROLLBACK / ROLLBACK - rollback and end transaction
 //   TXN STATUS            - print transaction status
 //
 // ATOMIC blocks automatically rollback on any error via
 // txn_on_error() called from the error handler. They commit
 // automatically on END ATOMIC.
 //
//
// HOW TO EXTEND:
//   To add a new statement or sub-command:
//   1. Add the keyword to lexer.h (KeywordId enum).
//   2. Add it to the keyword table in lexer.c.
//   3. Add a handler function in this file.
//   4. Wire it into parser.c's dispatch switch.
//
// TROUBLESHOOTING:
//   - 'WHAT?' on valid syntax: check dialect feature flags.
//   - Crash in expression: ensure error_occurred() is checked
//     after every parse_expression call.
 // ---

#include "parser_internal.h"
#include "txn.h"

 // pi_parse_atomic - Handle ATOMIC statement.
 //
 // The ATOMIC keyword has already been consumed.
 // Begins an atomic block with auto-rollback on error.
 //
 // In stored programs, ATOMIC marks the start of the block.
 // END ATOMIC marks the end and commits.
 //
 // In direct mode, ATOMIC begins the transaction; the user
 // must use END ATOMIC or COMMIT to end it.
void pi_parse_atomic(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex; (void)rt;

    if (txn_is_active() != TXN_NONE) {
        printf("Transaction already active.\n");
        error_raise(ERR_HOW, line_num);
        return;
    }

    if (txn_begin(TXN_ATOMIC) != 0) {
        error_raise(ERR_HOW, line_num);
        return;
    }
}

 // pi_parse_end_atomic - Handle END ATOMIC.
 //
 // Called when parser sees END and then checks for ATOMIC.
 // Commits the current ATOMIC block.
void pi_parse_end_atomic(int line_num)
{
    int state = txn_is_active();

    if (state != TXN_ATOMIC) {
        printf("No ATOMIC block to end.\n");
        error_raise(ERR_HOW, line_num);
        return;
    }

    txn_commit();
}

 // pi_parse_txn - Handle TXN statement.
 //
 // TXN keyword has already been consumed. Parse subcommand:
 //   TXN BEGIN     - start explicit transaction
 //   TXN COMMIT   - commit transaction
 //   TXN ROLLBACK - rollback transaction
 //   TXN STATUS   - print status
void pi_parse_txn(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)rt;

    // Check for subcommand as a named variable or keyword
    if (lex->current.type == TOK_NAMED_VAR) {
        const char *s = lex->current.str_start;
        int n = lex->current.str_length;

        // BEGIN
        if (n == 5 &&
            (s[0]=='B'||s[0]=='b') &&
            (s[1]=='E'||s[1]=='e') &&
            (s[2]=='G'||s[2]=='g') &&
            (s[3]=='I'||s[3]=='i') &&
            (s[4]=='N'||s[4]=='n')) {
            lexer_next(lex); // consume BEGIN
            if (txn_is_active() != TXN_NONE) {
                printf("Transaction already active.\n");
                error_raise(ERR_HOW, line_num);
                return;
            }
            if (txn_begin(TXN_ACTIVE) != 0) {
                error_raise(ERR_HOW, line_num);
            }
            return;
        }

        // STATUS
        if (n == 6 &&
            (s[0]=='S'||s[0]=='s') &&
            (s[1]=='T'||s[1]=='t') &&
            (s[2]=='A'||s[2]=='a') &&
            (s[3]=='T'||s[3]=='t') &&
            (s[4]=='U'||s[4]=='u') &&
            (s[5]=='S'||s[5]=='s')) {
            int state;
            lexer_next(lex);
            state = txn_is_active();
            printf("TXN: %s (%d entries)\n",
                   state == TXN_NONE   ? "NONE" :
                   state == TXN_ACTIVE ? "ACTIVE" :
                   state == TXN_ATOMIC ? "ATOMIC" : "?",
                   txn_entry_count());
            return;
        }
    }

    // TXN COMMIT
    if (lex->current.type == TOK_KEYWORD &&
        lex->current.value.keyword == KW_COMMIT) {
        lexer_next(lex);
        if (txn_is_active() == TXN_NONE) {
            printf("No active transaction.\n");
            error_raise(ERR_HOW, line_num);
            return;
        }
        txn_commit();
        return;
    }

    // TXN ROLLBACK
    if (lex->current.type == TOK_KEYWORD &&
        lex->current.value.keyword == KW_ROLLBACK) {
        lexer_next(lex);
        if (txn_is_active() == TXN_NONE) {
            printf("No active transaction.\n");
            error_raise(ERR_HOW, line_num);
            return;
        }
        txn_rollback(line_num);
        return;
    }

    error_raise(ERR_WHAT, line_num);
}

 // pi_parse_commit - Handle standalone COMMIT statement.
 //
 // COMMIT keyword has already been consumed.
 // Commits the active transaction (TXN or ATOMIC).
void pi_parse_commit(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex; (void)rt;

    if (txn_is_active() == TXN_NONE) {
        printf("No active transaction.\n");
        error_raise(ERR_HOW, line_num);
        return;
    }

    txn_commit();
}

 // pi_parse_rollback - Handle standalone ROLLBACK statement.
 //
 // ROLLBACK keyword has already been consumed.
 // Rolls back the active transaction.
void pi_parse_rollback(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex; (void)rt;

    if (txn_is_active() == TXN_NONE) {
        printf("No active transaction.\n");
        error_raise(ERR_HOW, line_num);
        return;
    }

    txn_rollback(line_num);
}
