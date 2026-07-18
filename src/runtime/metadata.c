/**
 * @file metadata.c
 * @brief Implementation of the metadata registry for namespaces, global labels, and docstrings.
 *
 * WHAT IT DOES:
 * This file implements registration, lookup, and lifecycle functions for namespaces,
 * cross-file global labels, and queryable docstring documentation.
 *
 * WHY IT EXISTS:
 * To provide memory-efficient, O(1)/O(N) data structures that store and resolve modular
 * properties, linking them to interpreter runtime states.
 *
 * WHY IT WORKS THIS WAY:
 * Linear array searches are chosen over complex hash maps because the registry limits (256/512 items)
 * are small, keeping execution times minimal and ensuring low footprint on target hardware (e.g. FreeDOS).
 * String matching uses case-insensitive routines where appropriate.
 *
 * WHAT CAN BE CHANGED:
 * Tuning scan performance or implementing binary search for label mappings.
 *
 * WHAT CANNOT BE CHANGED:
 * Registry array capacities must not exceed predefined limits to avoid stack/heap overflows.
 *
 * WHAT TO EXPECT:
 * High-performance, deterministic search times. Clean cleanup of buffers.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Check for string overflow during copy operations. Ensure bounds-checking returns false.
 *
 * ASSUMPTIONS:
 * Caller validates names and strings before calling registration functions.
 *
 * PORTABILITY CONCERNS:
 * Completely standard ANSI C17. No dynamic memory allocation used during runtime registry lookups.
 *
 * FUTURE EXPANSIONS:
 * Allow dynamically-sized metadata buckets if building for modern, 64-bit systems.
 *
 * EXTERNAL EXTENSION HOOKS:
 * Exposes clean C registration entry points used by both internal parser and external module loaders.
 */

#include "bpp_metadata.h"
#include "bpp_vm.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#if defined(_WIN32) && !defined(strcasecmp)
#define strcasecmp _stricmp
#endif

static void safe_strncpy(char *dest, const char *src, size_t dest_size) {
    if (!dest || dest_size == 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    size_t len = strlen(src);
    if (len >= dest_size) {
        len = dest_size - 1;
    }
    memcpy(dest, src, len);
    dest[len] = '\0';
}

#undef strncpy
#define strncpy(dest, src, size) safe_strncpy(dest, src, (size) + 1)

void metadata_init(BppMetadataRegistry *reg) {
    if (!reg) return;
    reg->global_label_count = 0;
    reg->docstring_count = 0;
    reg->current_namespace[0] = '\0';
    reg->last_docstring_buffer[0] = '\0';
    reg->option_strict = false;
    reg->metadata_block_count = 0;
}

bool metadata_register_label(BppMetadataRegistry *reg, const char *label, const char *filename, BppLineNumber line) {
    if (!reg || !label || !filename) return false;
    if (reg->global_label_count >= MAX_GLOBAL_LABELS) return false;

    // Check if the label is already registered to avoid duplicates
    for (int i = 0; i < reg->global_label_count; i++) {
        if (strcasecmp(reg->global_labels[i].label_name, label) == 0) {
            // Update location
            strncpy(reg->global_labels[i].filename, filename, sizeof(reg->global_labels[i].filename) - 1);
            reg->global_labels[i].filename[sizeof(reg->global_labels[i].filename) - 1] = '\0';
            reg->global_labels[i].line_number = line;
            return true;
        }
    }

    int idx = reg->global_label_count;
    strncpy(reg->global_labels[idx].label_name, label, sizeof(reg->global_labels[idx].label_name) - 1);
    reg->global_labels[idx].label_name[sizeof(reg->global_labels[idx].label_name) - 1] = '\0';

    strncpy(reg->global_labels[idx].filename, filename, sizeof(reg->global_labels[idx].filename) - 1);
    reg->global_labels[idx].filename[sizeof(reg->global_labels[idx].filename) - 1] = '\0';

    reg->global_labels[idx].line_number = line;
    reg->global_label_count++;
    return true;
}

bool metadata_resolve_label(const BppMetadataRegistry *reg, const char *label, char *out_filename, size_t max_len, BppLineNumber *out_line) {
    if (!reg || !label || !out_filename || !out_line) return false;

    for (int i = 0; i < reg->global_label_count; i++) {
        if (strcasecmp(reg->global_labels[i].label_name, label) == 0) {
            strncpy(out_filename, reg->global_labels[i].filename, max_len - 1);
            out_filename[max_len - 1] = '\0';
            *out_line = reg->global_labels[i].line_number;
            return true;
        }
    }
    return false;
}

bool metadata_register_docstring(BppMetadataRegistry *reg, const char *target, const char *docstring) {
    if (!reg || !target || !docstring) return false;
    if (reg->docstring_count >= MAX_DOCSTRINGS) return false;

    // Check if target already has a docstring to update it
    for (int i = 0; i < reg->docstring_count; i++) {
        if (strcasecmp(reg->docstrings[i].target_name, target) == 0) {
            strncpy(reg->docstrings[i].docstring, docstring, sizeof(reg->docstrings[i].docstring) - 1);
            reg->docstrings[i].docstring[sizeof(reg->docstrings[i].docstring) - 1] = '\0';
            return true;
        }
    }

    int idx = reg->docstring_count;
    strncpy(reg->docstrings[idx].target_name, target, sizeof(reg->docstrings[idx].target_name) - 1);
    reg->docstrings[idx].target_name[sizeof(reg->docstrings[idx].target_name) - 1] = '\0';

    strncpy(reg->docstrings[idx].docstring, docstring, sizeof(reg->docstrings[idx].docstring) - 1);
    reg->docstrings[idx].docstring[sizeof(reg->docstrings[idx].docstring) - 1] = '\0';

    reg->docstring_count++;
    return true;
}

const char *metadata_get_docstring(const BppMetadataRegistry *reg, const char *target) {
    if (!reg || !target) return NULL;

    for (int i = 0; i < reg->docstring_count; i++) {
        if (strcasecmp(reg->docstrings[i].target_name, target) == 0) {
            return reg->docstrings[i].docstring;
        }
    }
    return NULL;
}

#include <stdio.h>

void metadata_pre_scan_line(VMContext *vm, const char *filename, BppLineNumber line_num, const char *text) {
    BppMetadataRegistry *reg = vm_get_metadata(vm);
    if (!reg || !text) return;

    LexerContext *lex = lex_init(vm_get_mem(vm), text);
    if (!lex) return;

    BppToken tok = lex_next(lex);

    // 1. Docstring: // docstring
    if (tok.type == TOK_DOCSTRING) {
        int len = (int)(tok.length < sizeof(reg->last_docstring_buffer) - 1 ? tok.length : sizeof(reg->last_docstring_buffer) - 1);
        memcpy(reg->last_docstring_buffer, tok.as.string, len);
        reg->last_docstring_buffer[len] = '\0';
    }
    // 2. Namespace: ::[namespace]
    else if (tok.type == TOK_NAMESPACE_DECL) {
        int len = (int)(tok.length < sizeof(reg->current_namespace) - 1 ? tok.length : sizeof(reg->current_namespace) - 1);
        memcpy(reg->current_namespace, tok.as.string, len);
        reg->current_namespace[len] = '\0';

        // Associate buffered docstring with the namespace if present
        if (reg->last_docstring_buffer[0] != '\0') {
            metadata_register_docstring(reg, reg->current_namespace, reg->last_docstring_buffer);
            reg->last_docstring_buffer[0] = '\0';
        }
    }
    // 3. Global Label: ::label:
    else if (tok.type == TOK_GLOBAL_LABEL) {
        char label_name[64];
        int len = (int)(tok.length < sizeof(label_name) - 1 ? tok.length : sizeof(label_name) - 1);
        memcpy(label_name, tok.as.string, len);
        label_name[len] = '\0';

        metadata_register_label(reg, label_name, filename, line_num);

        // Associate buffered docstring with the global label key (e.g. "::label")
        if (reg->last_docstring_buffer[0] != '\0') {
            char target_key[128];
            snprintf(target_key, sizeof(target_key), "::%s", label_name);
            metadata_register_docstring(reg, target_key, reg->last_docstring_buffer);
            reg->last_docstring_buffer[0] = '\0';
        }
    }
    // 4. Procedures: SUB, FUNCTION
    else if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_SUB || tok.as.keyword == KW_FUNCTION) {
            BppToken name_tok = lex_next(lex);
            if (name_tok.type == TOK_IDENT) {
                char proc_name[64];
                int len = (int)(name_tok.length < sizeof(proc_name) - 1 ? name_tok.length : sizeof(proc_name) - 1);
                memcpy(proc_name, name_tok.as.string, len);
                proc_name[len] = '\0';

                char fully_qualified[128];
                if (reg->current_namespace[0] != '\0') {
                    snprintf(fully_qualified, sizeof(fully_qualified), "%s.%s", reg->current_namespace, proc_name);
                } else {
                    snprintf(fully_qualified, sizeof(fully_qualified), "%s", proc_name);
                }

                if (reg->last_docstring_buffer[0] != '\0') {
                    metadata_register_docstring(reg, fully_qualified, reg->last_docstring_buffer);
                    reg->last_docstring_buffer[0] = '\0';
                }
            }
        }
    }

    // Clear docstring buffer if we hit a statement that isn't a docstring, EOL, or comment
    if (tok.type != TOK_DOCSTRING && tok.type != TOK_EOL && tok.type != TOK_EOF) {
        if (tok.type != TOK_KEYWORD || (tok.as.keyword != KW_REM && tok.as.keyword != KW_SUB && tok.as.keyword != KW_FUNCTION)) {
            // Keep docstring buffer for LET or other assignments only if they might declare a documented variable,
            // but for simplicity let's only clear on other non-declarative statements.
            reg->last_docstring_buffer[0] = '\0';
        }
    }

    lex_shutdown(lex);
}

bool metadata_register_block(BppMetadataRegistry *reg, const char *type, const char *target, const char *docstring, const char *body) {
    if (!reg || !type || !target) return false;
    if (reg->metadata_block_count >= MAX_METADATA_BLOCKS) return false;

    // Check if target block already exists to update it (merging docstrings and bodies)
    for (int i = 0; i < reg->metadata_block_count; i++) {
        if (strcasecmp(reg->metadata_blocks[i].block_type, type) == 0 &&
            strcasecmp(reg->metadata_blocks[i].target_name, target) == 0) {
            if (docstring && docstring[0] != '\0') {
                if (reg->metadata_blocks[i].docstring[0] == '\0') {
                    strncpy(reg->metadata_blocks[i].docstring, docstring, sizeof(reg->metadata_blocks[i].docstring) - 1);
                    reg->metadata_blocks[i].docstring[sizeof(reg->metadata_blocks[i].docstring) - 1] = '\0';
                } else if (strstr(reg->metadata_blocks[i].docstring, docstring) == NULL) {
                    size_t cur_dlen = strlen(reg->metadata_blocks[i].docstring);
                    if (cur_dlen + strlen(docstring) + 3 < sizeof(reg->metadata_blocks[i].docstring)) {
                        snprintf(reg->metadata_blocks[i].docstring + cur_dlen,
                                 sizeof(reg->metadata_blocks[i].docstring) - cur_dlen,
                                 " | %s", docstring);
                    }
                }
            }
            if (body && body[0] != '\0') {
                size_t cur_len = strlen(reg->metadata_blocks[i].body);
                if (cur_len > 0) {
                    if (cur_len + strlen(body) + 2 < sizeof(reg->metadata_blocks[i].body)) {
                        snprintf(reg->metadata_blocks[i].body + cur_len,
                                 sizeof(reg->metadata_blocks[i].body) - cur_len,
                                 "\n%s", body);
                    }
                } else {
                    strncpy(reg->metadata_blocks[i].body, body, sizeof(reg->metadata_blocks[i].body) - 1);
                    reg->metadata_blocks[i].body[sizeof(reg->metadata_blocks[i].body) - 1] = '\0';
                }
            }
            return true;
        }
    }

    int idx = reg->metadata_block_count;
    strncpy(reg->metadata_blocks[idx].block_type, type, sizeof(reg->metadata_blocks[idx].block_type) - 1);
    reg->metadata_blocks[idx].block_type[sizeof(reg->metadata_blocks[idx].block_type) - 1] = '\0';

    strncpy(reg->metadata_blocks[idx].target_name, target, sizeof(reg->metadata_blocks[idx].target_name) - 1);
    reg->metadata_blocks[idx].target_name[sizeof(reg->metadata_blocks[idx].target_name) - 1] = '\0';

    if (docstring) {
        strncpy(reg->metadata_blocks[idx].docstring, docstring, sizeof(reg->metadata_blocks[idx].docstring) - 1);
        reg->metadata_blocks[idx].docstring[sizeof(reg->metadata_blocks[idx].docstring) - 1] = '\0';
    } else {
        reg->metadata_blocks[idx].docstring[0] = '\0';
    }

    if (body) {
        strncpy(reg->metadata_blocks[idx].body, body, sizeof(reg->metadata_blocks[idx].body) - 1);
        reg->metadata_blocks[idx].body[sizeof(reg->metadata_blocks[idx].body) - 1] = '\0';
    } else {
        reg->metadata_blocks[idx].body[0] = '\0';
    }

    reg->metadata_block_count++;
    return true;
}

const BppMetadataBlock *metadata_get_block(const BppMetadataRegistry *reg, const char *type, const char *target) {
    if (!reg || !type || !target) return NULL;

    for (int i = 0; i < reg->metadata_block_count; i++) {
        if (strcasecmp(reg->metadata_blocks[i].block_type, type) == 0 &&
            strcasecmp(reg->metadata_blocks[i].target_name, target) == 0) {
            return &reg->metadata_blocks[i];
        }
    }
    return NULL;
}

static bool is_block_end_marker_simple(const char *text, const char *block_type, const char *block_target, MemoryContext *mem) {
    LexerContext *check_lex = lex_init(mem, text);
    if (!check_lex) return false;

    BppToken tok = lex_next(check_lex);
    
    /* 1. Check for 'END {block_type}' */
    if ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length == 3 && strncasecmp(tok.start, "END", 3) == 0) {
        BppToken next_tok = lex_next(check_lex);
        if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
            char next_name[64];
            size_t nlen = (next_tok.length < 63) ? next_tok.length : 63;
            memcpy(next_name, next_tok.start, nlen);
            next_name[nlen] = '\0';
            if (strcasecmp(next_name, block_type) == 0) {
                lex_shutdown(check_lex);
                return true;
            }
        }
    }

    /* 2. Check for target-qualified endings: '{target} {block_type}::' */
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_STRING) {
        char target_buf[64];
        size_t tlen = (tok.length < 63) ? tok.length : 63;
        memcpy(target_buf, tok.start, tlen);
        target_buf[tlen] = '\0';

        if (tok.start + tok.length < text + strlen(text) && *(tok.start + tok.length) == ':') {
            if (tlen + 1 < 63) {
                target_buf[tlen] = ':';
                target_buf[tlen + 1] = '\0';
            }
        }

        if (strcasecmp(target_buf, block_target) == 0) {
            BppToken next_tok = lex_next(check_lex);
            if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
                char next_name[64];
                size_t nlen = (next_tok.length < 63) ? next_tok.length : 63;
                memcpy(next_name, next_tok.start, nlen);
                next_name[nlen] = '\0';

                if (strcasecmp(next_name, block_type) == 0 && lex_peek(check_lex).type == TOK_DOUBLE_COLON) {
                    lex_shutdown(check_lex);
                    return true;
                }
            }
        }
    }

    /* 3. Check for standard block ending: '{block_type}::' */
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char name_buf[64];
        size_t len = (tok.length < 63) ? tok.length : 63;
        memcpy(name_buf, tok.start, len);
        name_buf[len] = '\0';

        if (strcasecmp(name_buf, block_type) == 0 && lex_peek(check_lex).type == TOK_DOUBLE_COLON) {
            lex_shutdown(check_lex);
            return true;
        }
    }

    lex_shutdown(check_lex);
    return false;
}

void metadata_pre_scan_program(VMContext *vm, const char *filename) {
    BppMetadataRegistry *reg = vm_get_metadata(vm);
    if (!reg) return;

    metadata_init(reg);

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);

    bool inside_block = false;
    char current_block_type[32] = "";
    char current_block_target[64] = "";
    char current_block_body[MAX_BLOCK_BODY_LEN] = "";
    char current_block_docstring[256] = "";

    struct {
        char type[32];
        char target[64];
        char body[MAX_BLOCK_BODY_LEN];
        char docstring[256];
    } nest_stack[8];
    int nest_stack_ptr = 0;

#define POP_NEST_STACK() \
    do { \
        nest_stack_ptr--; \
        strncpy(current_block_type, nest_stack[nest_stack_ptr].type, sizeof(current_block_type) - 1); \
        current_block_type[sizeof(current_block_type) - 1] = '\0'; \
        strncpy(current_block_target, nest_stack[nest_stack_ptr].target, sizeof(current_block_target) - 1); \
        current_block_target[sizeof(current_block_target) - 1] = '\0'; \
        strncpy(current_block_body, nest_stack[nest_stack_ptr].body, sizeof(current_block_body) - 1); \
        current_block_body[sizeof(current_block_body) - 1] = '\0'; \
        strncpy(current_block_docstring, nest_stack[nest_stack_ptr].docstring, sizeof(current_block_docstring) - 1); \
        current_block_docstring[sizeof(current_block_docstring) - 1] = '\0'; \
    } while (0)

    for (size_t i = 0; i < count; i++) {
        const char *text = lines[i].text;
        LexerContext *lex = lex_init(vm_get_mem(vm), text);
        if (!lex) continue;

        BppToken tok = lex_next(lex);

        if (tok.type == TOK_DIRECTIVE) {
            char dir_name[64];
            size_t len = (tok.length < 63) ? tok.length : 63;
            memcpy(dir_name, tok.as.string, len);
            dir_name[len] = '\0';

            if (strcasecmp(dir_name, "KEYWORD") == 0 || strcasecmp(dir_name, "SCOPE") == 0 || strcasecmp(dir_name, "ALIAS") == 0 || strcasecmp(dir_name, "OPTION") == 0) {
                /* If we are already inside a block, push the current block state to the stack */
                if (inside_block) {
                    if (nest_stack_ptr < 8) {
                        strncpy(nest_stack[nest_stack_ptr].type, current_block_type, sizeof(nest_stack[nest_stack_ptr].type) - 1);
                        nest_stack[nest_stack_ptr].type[sizeof(nest_stack[nest_stack_ptr].type) - 1] = '\0';
                        strncpy(nest_stack[nest_stack_ptr].target, current_block_target, sizeof(nest_stack[nest_stack_ptr].target) - 1);
                        nest_stack[nest_stack_ptr].target[sizeof(nest_stack[nest_stack_ptr].target) - 1] = '\0';
                        strncpy(nest_stack[nest_stack_ptr].body, current_block_body, sizeof(nest_stack[nest_stack_ptr].body) - 1);
                        nest_stack[nest_stack_ptr].body[sizeof(nest_stack[nest_stack_ptr].body) - 1] = '\0';
                        strncpy(nest_stack[nest_stack_ptr].docstring, current_block_docstring, sizeof(nest_stack[nest_stack_ptr].docstring) - 1);
                        nest_stack[nest_stack_ptr].docstring[sizeof(nest_stack[nest_stack_ptr].docstring) - 1] = '\0';
                        nest_stack_ptr++;
                    }
                }

                inside_block = true;
                strncpy(current_block_type, dir_name, sizeof(current_block_type) - 1);
                current_block_type[sizeof(current_block_type) - 1] = '\0';

                BppToken target_tok = lex_next(lex);
                if (target_tok.type == TOK_IDENT || target_tok.type == TOK_KEYWORD || target_tok.type == TOK_STRING) {
                    size_t tlen = (target_tok.length < 63) ? target_tok.length : 63;
                    memcpy(current_block_target, target_tok.start, tlen);
                    current_block_target[tlen] = '\0';

                    if (target_tok.start + target_tok.length < text + strlen(text) && *(target_tok.start + target_tok.length) == ':') {
                        if (tlen + 1 < 63) {
                            current_block_target[tlen] = ':';
                            current_block_target[tlen + 1] = '\0';
                        }
                    }
                } else {
                    current_block_target[0] = '\0';
                }

                /* Inherit docstring from parent if stack has a parent block and local docstring starts empty */
                if (nest_stack_ptr > 0 && current_block_docstring[0] == '\0') {
                    strncpy(current_block_docstring, nest_stack[nest_stack_ptr - 1].docstring, sizeof(current_block_docstring) - 1);
                    current_block_docstring[sizeof(current_block_docstring) - 1] = '\0';
                } else {
                    current_block_docstring[0] = '\0';
                }

                current_block_body[0] = '\0';

                /* Check if there is a matching closing block marker later in the program */
                bool has_closing_marker = false;
                for (size_t j = i + 1; j < count; j++) {
                    if (is_block_end_marker_simple(lines[j].text, dir_name, current_block_target, vm_get_mem(vm))) {
                        has_closing_marker = true;
                        break;
                    }
                }

                if (has_closing_marker) {
                    inside_block = true;
                } else {
                    /* Single-line block! Scan rest of line for inline docstring/comment using strstr */
                    const char *doc_ptr = strstr(text, "//");
                    if (doc_ptr) {
                        doc_ptr += 2;
                        while (*doc_ptr && isspace((unsigned char)*doc_ptr)) {
                            doc_ptr++;
                        }
                        size_t dlen = strlen(doc_ptr);
                        if (dlen < sizeof(current_block_docstring)) {
                            memcpy(current_block_docstring, doc_ptr, dlen);
                            current_block_docstring[dlen] = '\0';
                        } else {
                            memcpy(current_block_docstring, doc_ptr, sizeof(current_block_docstring) - 1);
                            current_block_docstring[sizeof(current_block_docstring) - 1] = '\0';
                        }
                    }
                    metadata_register_block(reg, current_block_type, current_block_target, current_block_docstring, "");

                    /* Restore parent block state since the single-line block didn't keep inside_block state */
                    if (nest_stack_ptr > 0) {
                        POP_NEST_STACK();
                        inside_block = true;
                    } else {
                        inside_block = false;
                    }
                }

                lex_shutdown(lex);
                continue;
            }
        }

        if (inside_block) {
            /* 1. Check for 'END {block_type}' */
            if ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length == 3 && strncasecmp(tok.start, "END", 3) == 0) {
                BppToken next_tok = lex_next(lex);
                if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
                    char next_name[64];
                    size_t nlen = (next_tok.length < 63) ? next_tok.length : 63;
                    memcpy(next_name, next_tok.start, nlen);
                    next_name[nlen] = '\0';
                    if (strcasecmp(next_name, current_block_type) == 0) {
                        /* Check for optional double colon END KEYWORD:: */
                        if (lex_peek(lex).type == TOK_DOUBLE_COLON) {
                            lex_next(lex); /* consume :: */
                        }
                        /* Close block! */
                        metadata_register_block(reg, current_block_type, current_block_target, current_block_docstring, current_block_body);
                        /* Pop parent block */
                        if (nest_stack_ptr > 0) {
                            POP_NEST_STACK();
                            inside_block = true;
                        } else {
                            inside_block = false;
                        }
                        lex_shutdown(lex);
                        continue;
                    }
                }
            }

            /* 2. Check for target-qualified endings: '{target} {block_type}::' */
            if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_STRING) {
                char target_buf[64];
                size_t tlen = (tok.length < 63) ? tok.length : 63;
                memcpy(target_buf, tok.start, tlen);
                target_buf[tlen] = '\0';
                
                /* Support target ending with colon, e.g. E: ALIAS:: */
                if (tok.start + tok.length < text + strlen(text) && *(tok.start + tok.length) == ':') {
                    if (tlen + 1 < 63) {
                        target_buf[tlen] = ':';
                        target_buf[tlen + 1] = '\0';
                    }
                }

                if (strcasecmp(target_buf, current_block_target) == 0) {
                    BppToken next_tok = lex_next(lex);
                    if (next_tok.type == TOK_IDENT || next_tok.type == TOK_KEYWORD) {
                        char next_name[64];
                        size_t nlen = (next_tok.length < 63) ? next_tok.length : 63;
                        memcpy(next_name, next_tok.start, nlen);
                        next_name[nlen] = '\0';

                        if (strcasecmp(next_name, current_block_type) == 0 && lex_peek(lex).type == TOK_DOUBLE_COLON) {
                            /* Close block! */
                            metadata_register_block(reg, current_block_type, current_block_target, current_block_docstring, current_block_body);
                            /* Pop parent block */
                            if (nest_stack_ptr > 0) {
                                POP_NEST_STACK();
                                inside_block = true;
                            } else {
                                inside_block = false;
                            }
                            lex_shutdown(lex);
                            continue;
                        }
                    }
                }
            }

            /* 3. Check for standard block ending: '{block_type}::' */
            if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
                char name_buf[64];
                size_t len = (tok.length < 63) ? tok.length : 63;
                memcpy(name_buf, tok.start, len);
                name_buf[len] = '\0';

                if (strcasecmp(name_buf, current_block_type) == 0 && lex_peek(lex).type == TOK_DOUBLE_COLON) {
                    /* End of block! Register it. */
                    metadata_register_block(reg, current_block_type, current_block_target, current_block_docstring, current_block_body);
                    
                    /* Pop parent block from stack if stack is not empty */
                    if (nest_stack_ptr > 0) {
                        POP_NEST_STACK();
                        inside_block = true;
                    } else {
                        inside_block = false;
                    }
                    lex_shutdown(lex);
                    continue;
                }
            }

            /* Otherwise, collect lines into block body */
            if (tok.type == TOK_DOCSTRING) {
                /* If it's a docstring comment, we can extract and append it to the docstring if empty */
                if (current_block_docstring[0] == '\0') {
                    size_t dlen = (tok.length < sizeof(current_block_docstring) - 1) ? tok.length : sizeof(current_block_docstring) - 1;
                    memcpy(current_block_docstring, tok.as.string, dlen);
                    current_block_docstring[dlen] = '\0';
                }
            }

            /* Append text line to body */
            size_t current_len = strlen(current_block_body);
            size_t text_len = text ? strlen(text) : 0;
            if (text_len > 0 && current_len + text_len + 2 < MAX_BLOCK_BODY_LEN) {
                if (current_len > 0) {
                    snprintf(current_block_body + current_len, MAX_BLOCK_BODY_LEN - current_len, "\n%s", text);
                } else {
                    snprintf(current_block_body, MAX_BLOCK_BODY_LEN, "%s", text);
                }
            }
            lex_shutdown(lex);
            continue;
        }

        lex_shutdown(lex);

        /* Fallback to standard line pre-scanning */
        metadata_pre_scan_line(vm, filename ? filename : "", lines[i].line_number, lines[i].text);
    }
#undef POP_NEST_STACK
}
