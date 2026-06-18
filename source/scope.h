 // ---
 // BASIC++ Interpreter - scope.h
 // ---
 //
 // SCOPE keyword: keyword access control and behavior hooks.
 //
 // Features:
 // - Disable/enable individual keywords
 // - BEFORE/AFTER/OVERRIDE hooks via GOSUB
 // - Named presets (STRUCTURED, SAFE, etc.)
 //
 // Hook execution:
 // BEFORE: GOSUB hook_line before keyword executes,
 //         RETURN resumes at same line (keyword runs).
 // AFTER:  keyword executes first, then GOSUB hook_line,
 //         RETURN resumes at next line.
 // OVERRIDE: GOSUB hook_line instead of keyword,
 //           RETURN resumes at next line (keyword skipped).
 //
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
 // ---

#ifndef BASICPP_SCOPE_H
#define BASICPP_SCOPE_H

#include "lexer.h"

// --- Scope Entry ---
 //
 // Per-keyword scope control. One entry per KeywordId.
typedef struct ScopeEntry {
 int disabled; // 1 = keyword raises error
 int before_line; // GOSUB target before, -1 = none
 int after_line; // GOSUB target after,  -1 = none
 int override_line; // GOSUB target instead, -1 = none
} ScopeEntry;

 // scope_init - Initialize scope table (all enabled, no hooks).
void scope_init(void);

 // scope_reset - Clear all scope rules (enable all, remove hooks).
void scope_reset(void);

 // scope_is_disabled - Check if a keyword is blocked.
int scope_is_disabled(KeywordId kw);

 // scope_disable / scope_enable - Toggle keyword access.
void scope_disable(KeywordId kw);
void scope_enable(KeywordId kw);

 // scope_set_before / after / override - Set hooks.
 // line = BASIC line number for GOSUB target.
void scope_set_before(KeywordId kw, int line);
void scope_set_after(KeywordId kw, int line);
void scope_set_override(KeywordId kw, int line);

 // scope_get_before / after / override - Get hook targets.
 // Returns -1 if no hook set.
int scope_get_before(KeywordId kw);
int scope_get_after(KeywordId kw);
int scope_get_override(KeywordId kw);

 // scope_clear_hooks - Remove all hooks from a keyword.
void scope_clear_hooks(KeywordId kw);

 // scope_restore - Enable keyword and remove all hooks.
void scope_restore(KeywordId kw);

 // scope_has_any_rules - Check if any scope rules are active.
int scope_has_any_rules(void);

 // scope_list - Print all active scope rules.
void scope_list(void);

 // scope_load_preset - Load a named preset.
 //
 // Valid names: "STRUCTURED", "SAFE", "MINIMAL", "EDUCATIONAL"
 // Returns 0 on success, -1 if unknown preset.
int scope_load_preset(const char *name);

 // scope_list_presets - Show available presets.
void scope_list_presets(void);

 // scope_get_last_kw_name - Get the keyword name that triggered
 // the most recent hook (for SCOPE$ variable).
 // Returns "" if no hook is active.
const char *scope_get_last_kw_name(void);

 // scope_set_last_kw - Record which keyword triggered a hook.
void scope_set_last_kw(KeywordId kw);

#endif // BASICPP_SCOPE_H
