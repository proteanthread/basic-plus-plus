/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file vm_stack.c
 * @brief VM Call Stack and GOSUB/RETURN frame manager.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Manages pushing and popping GOSUB return frames (containing line numbers
 *   and character offsets) on an interpreter-managed heap stack.
 * - Why it exists: Fulfills the "Strict Non-Recursive VM" mandate. It avoids using the host C runtime stack
 *   for BASIC subroutine calls, eliminating any risk of stack overflow on deep nesting.
 * - Why it works this way: It allocates a fixed-size or dynamic array of GosubFrame structs on the heap
 *   during VM initialization and checks bounds on pushes and pops.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Call frame fields, maximum depth limit parameters.
 * - What cannot be changed: Memory alignment and standard push/pop error codes.
 * - What to expect: Exceeding BPP_MAX_STACK_DEPTH will return false, triggering a stack overflow error.
 * - What to do if something breaks: Trace GOSUB calls, check stack boundary pointers, and print active frames.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Stack frames are allocated contiguous on VM init.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add local scoping stack frames or exception catch points.
 * - How to write external extensions: Plugins do not manipulate GOSUB frames directly.
 */

#include "vm/vm.h"
#include "types/config.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    BppLineNumber line;
    const char   *pos;
} GosubFrame;

/* Private GOSUB stack representation */
typedef struct GosubStack {
    GosubFrame *frames;
    size_t      count;
    size_t      capacity;
} GosubStack;

/* Stored inside the VM Context structure. We'll expose helpers: */
GosubStack *gosub_stack_init(void) {
    GosubStack *stack = (GosubStack *)calloc(1, sizeof(GosubStack));
    if (!stack) return NULL;
    stack->capacity = BPP_MAX_STACK_DEPTH;
    stack->count = 0;
    stack->frames = (GosubFrame *)calloc(stack->capacity, sizeof(GosubFrame));
    if (!stack->frames) {
        free(stack);
        return NULL;
    }
    return stack;
}

void gosub_stack_shutdown(GosubStack *stack) {
    if (!stack) return;
    free(stack->frames);
    free(stack);
}

void gosub_stack_clear(GosubStack *stack) {
    if (stack) {
        stack->count = 0;
    }
}

bool gosub_stack_push(GosubStack *stack, BppLineNumber line, const char *pos) {
    if (!stack) return false;
    if (stack->count >= stack->capacity) {
        return false; /* Stack overflow */
    }
    stack->frames[stack->count].line = line;
    stack->frames[stack->count].pos = pos;
    stack->count++;
    return true;
}

bool gosub_stack_pop(GosubStack *stack, BppLineNumber *out_line, const char **out_pos) {
    if (!stack || stack->count == 0) {
        return false; /* Stack underflow */
    }
    stack->count--;
    if (out_line) *out_line = stack->frames[stack->count].line;
    if (out_pos) *out_pos = stack->frames[stack->count].pos;
    return true;
}

/* Private FOR stack representation */
typedef struct ForStack {
    BppForFrame *frames;
    size_t       count;
    size_t       capacity;
} ForStack;

ForStack *for_stack_init(void) {
    ForStack *stack = (ForStack *)calloc(1, sizeof(ForStack));
    if (!stack) return NULL;
    stack->capacity = BPP_MAX_STACK_DEPTH;
    stack->count = 0;
    stack->frames = (BppForFrame *)calloc(stack->capacity, sizeof(BppForFrame));
    if (!stack->frames) {
        free(stack);
        return NULL;
    }
    return stack;
}

void for_stack_shutdown(ForStack *stack) {
    if (!stack) return;
    free(stack->frames);
    free(stack);
}

void for_stack_clear(ForStack *stack) {
    if (stack) {
        stack->count = 0;
    }
}

bool for_stack_push(ForStack *stack, const char *var_name, double target, double step, BppLineNumber line, const char *pos) {
    if (!stack || stack->count >= stack->capacity) return false;
    BppForFrame *f = &stack->frames[stack->count];
    strncpy(f->var_name, var_name ? var_name : "", sizeof(f->var_name) - 1);
    f->var_name[sizeof(f->var_name) - 1] = '\0';
    f->target = target;
    f->step = step;
    f->line = line;
    f->pos = pos;
    f->next_range_pos = NULL;
    stack->count++;
    return true;
}

bool for_stack_update(ForStack *stack, const char *var_name, double target, double step, const char *next_range_pos) {
    if (!stack || stack->count == 0) return false;
    if (var_name && strlen(var_name) > 0) {
        for (int i = (int)stack->count - 1; i >= 0; --i) {
            if (strcmp(stack->frames[i].var_name, var_name) == 0) {
                stack->frames[i].target = target;
                stack->frames[i].step = step;
                stack->frames[i].next_range_pos = next_range_pos;
                return true;
            }
        }
    }
    return false;
}

bool for_stack_pop(ForStack *stack, const char *var_name, BppForFrame *out_frame) {
    if (!stack || stack->count == 0) return false;
    
    if (var_name && strlen(var_name) > 0) {
        for (int i = (int)stack->count - 1; i >= 0; --i) {
            if (strcmp(stack->frames[i].var_name, var_name) == 0) {
                if (out_frame) *out_frame = stack->frames[i];
                for (size_t j = (size_t)i; j < stack->count - 1; ++j) {
                    stack->frames[j] = stack->frames[j + 1];
                }
                stack->count--;
                return true;
            }
        }
        return false;
    } else {
        stack->count--;
        if (out_frame) *out_frame = stack->frames[stack->count];
        return true;
    }
}

bool for_stack_peek(ForStack *stack, const char *var_name, BppForFrame *out_frame) {
    if (!stack || stack->count == 0) return false;
    
    if (var_name && strlen(var_name) > 0) {
        for (int i = (int)stack->count - 1; i >= 0; --i) {
            if (strcmp(stack->frames[i].var_name, var_name) == 0) {
                if (out_frame) *out_frame = stack->frames[i];
                return true;
            }
        }
        return false;
    } else {
        if (out_frame) *out_frame = stack->frames[stack->count - 1];
        return true;
    }
}

/* Private WHILE loop stack representation */
typedef struct WhileStack {
    GosubFrame *frames;
    size_t      count;
    size_t      capacity;
} WhileStack;

WhileStack *while_stack_init(void) {
    WhileStack *stack = (WhileStack *)calloc(1, sizeof(WhileStack));
    if (!stack) return NULL;
    stack->capacity = BPP_MAX_STACK_DEPTH;
    stack->count = 0;
    stack->frames = (GosubFrame *)calloc(stack->capacity, sizeof(GosubFrame));
    if (!stack->frames) {
        free(stack);
        return NULL;
    }
    return stack;
}

void while_stack_shutdown(WhileStack *stack) {
    if (!stack) return;
    free(stack->frames);
    free(stack);
}

void while_stack_clear(WhileStack *stack) {
    if (stack) {
        stack->count = 0;
    }
}

bool while_stack_push(WhileStack *stack, BppLineNumber line, const char *pos) {
    if (!stack || stack->count >= stack->capacity) return false;
    stack->frames[stack->count].line = line;
    stack->frames[stack->count].pos = pos;
    stack->count++;
    return true;
}

bool while_stack_pop(WhileStack *stack, BppLineNumber *out_line, const char **out_pos) {
    if (!stack || stack->count == 0) return false;
    stack->count--;
    if (out_line) *out_line = stack->frames[stack->count].line;
    if (out_pos) *out_pos = stack->frames[stack->count].pos;
    return true;
}

bool while_stack_peek(WhileStack *stack, BppLineNumber *out_line, const char **out_pos) {
    if (!stack || stack->count == 0) return false;
    if (out_line) *out_line = stack->frames[stack->count - 1].line;
    if (out_pos) *out_pos = stack->frames[stack->count - 1].pos;
    return true;
}

/* Private DO loop stack representation */
typedef struct DoStack {
    GosubFrame *frames;
    size_t      count;
    size_t      capacity;
} DoStack;

DoStack *do_stack_init(void) {
    DoStack *stack = (DoStack *)calloc(1, sizeof(DoStack));
    if (!stack) return NULL;
    stack->capacity = BPP_MAX_STACK_DEPTH;
    stack->count = 0;
    stack->frames = (GosubFrame *)calloc(stack->capacity, sizeof(GosubFrame));
    if (!stack->frames) {
        free(stack);
        return NULL;
    }
    return stack;
}

void do_stack_shutdown(DoStack *stack) {
    if (!stack) return;
    free(stack->frames);
    free(stack);
}

void do_stack_clear(DoStack *stack) {
    if (stack) {
        stack->count = 0;
    }
}

bool do_stack_push(DoStack *stack, BppLineNumber line, const char *pos) {
    if (!stack || stack->count >= stack->capacity) return false;
    stack->frames[stack->count].line = line;
    stack->frames[stack->count].pos = pos;
    stack->count++;
    return true;
}

bool do_stack_pop(DoStack *stack, BppLineNumber *out_line, const char **out_pos) {
    if (!stack || stack->count == 0) return false;
    stack->count--;
    if (out_line) *out_line = stack->frames[stack->count].line;
    if (out_pos) *out_pos = stack->frames[stack->count].pos;
    return true;
}

bool do_stack_peek(DoStack *stack, BppLineNumber *out_line, const char **out_pos) {
    if (!stack || stack->count == 0) return false;
    if (out_line) *out_line = stack->frames[stack->count - 1].line;
    if (out_pos) *out_pos = stack->frames[stack->count - 1].pos;
    return true;
}

/* Private SELECT CASE stack representation */
struct SelectStack {
    BppSelectFrame *frames;
    size_t          count;
    size_t          capacity;
};

SelectStack *select_stack_init(void) {
    SelectStack *stack = (SelectStack *)calloc(1, sizeof(SelectStack));
    if (!stack) return NULL;
    stack->capacity = BPP_MAX_STACK_DEPTH;
    stack->count = 0;
    stack->frames = (BppSelectFrame *)calloc(stack->capacity, sizeof(BppSelectFrame));
    if (!stack->frames) {
        free(stack);
        return NULL;
    }
    return stack;
}

void select_stack_shutdown(SelectStack *stack) {
    if (!stack) return;
    free(stack->frames);
    free(stack);
}

void select_stack_clear(SelectStack *stack) {
    if (stack) {
        stack->count = 0;
    }
}

bool select_stack_push(SelectStack *stack, BValue val, bool matched, BppLineNumber line, const char *pos) {
    if (!stack || stack->count >= stack->capacity) return false;
    stack->frames[stack->count].val = val;
    stack->frames[stack->count].matched = matched;
    stack->frames[stack->count].line = line;
    stack->frames[stack->count].pos = pos;
    stack->count++;
    return true;
}

bool select_stack_pop(SelectStack *stack, BppSelectFrame *out_frame) {
    if (!stack || stack->count == 0) return false;
    stack->count--;
    if (out_frame) *out_frame = stack->frames[stack->count];
    return true;
}

bool select_stack_peek(SelectStack *stack, BppSelectFrame *out_frame) {
    if (!stack || stack->count == 0) return false;
    if (out_frame) *out_frame = stack->frames[stack->count - 1];
    return true;
}

/* Private SUB/FUNCTION stack representation */
struct SubStack {
    BppSubFrame *frames;
    size_t       count;
    size_t       capacity;
};

SubStack *sub_stack_init(void) {
    SubStack *stack = (SubStack *)calloc(1, sizeof(SubStack));
    if (!stack) return NULL;
    stack->capacity = BPP_MAX_STACK_DEPTH;
    stack->count = 0;
    stack->frames = (BppSubFrame *)calloc(stack->capacity, sizeof(BppSubFrame));
    if (!stack->frames) {
        free(stack);
        return NULL;
    }
    return stack;
}

void sub_stack_shutdown(SubStack *stack) {
    if (!stack) return;
    free(stack->frames);
    free(stack);
}

void sub_stack_clear(SubStack *stack) {
    if (stack) {
        stack->count = 0;
    }
}

bool sub_stack_push(SubStack *stack, const char *name, BppLineNumber line, const char *pos, bool is_func) {
    if (!stack || stack->count >= stack->capacity) return false;
    strncpy(stack->frames[stack->count].name, name, sizeof(stack->frames[stack->count].name) - 1);
    stack->frames[stack->count].name[sizeof(stack->frames[stack->count].name) - 1] = '\0';
    stack->frames[stack->count].line = line;
    stack->frames[stack->count].pos = pos;
    stack->frames[stack->count].is_func = is_func;
    stack->count++;
    return true;
}

bool sub_stack_pop(SubStack *stack, BppSubFrame *out_frame) {
    if (!stack || stack->count == 0) return false;
    stack->count--;
    if (out_frame) *out_frame = stack->frames[stack->count];
    return true;
}

bool sub_stack_peek(SubStack *stack, BppSubFrame *out_frame) {
    if (!stack || stack->count == 0) return false;
    if (out_frame) *out_frame = stack->frames[stack->count - 1];
    return true;
}

/* TryStack Implementation */
struct TryStack {
    BppTryFrame *frames;
    size_t       count;
    size_t       capacity;
};

TryStack *try_stack_init(void) {
    TryStack *stack = (TryStack *)calloc(1, sizeof(TryStack));
    if (!stack) return NULL;
    stack->capacity = BPP_MAX_STACK_DEPTH;
    stack->count = 0;
    stack->frames = (BppTryFrame *)calloc(stack->capacity, sizeof(BppTryFrame));
    if (!stack->frames) {
        free(stack);
        return NULL;
    }
    return stack;
}

void try_stack_shutdown(TryStack *stack) {
    if (!stack) return;
    free(stack->frames);
    free(stack);
}

void try_stack_clear(TryStack *stack) {
    if (stack) {
        stack->count = 0;
    }
}

bool try_stack_push(TryStack *stack, BppTryFrame frame) {
    if (!stack || stack->count >= stack->capacity) return false;
    stack->frames[stack->count++] = frame;
    return true;
}

bool try_stack_pop(TryStack *stack, BppTryFrame *out_frame) {
    if (!stack || stack->count == 0) return false;
    stack->count--;
    if (out_frame) *out_frame = stack->frames[stack->count];
    return true;
}

bool try_stack_peek(TryStack *stack, BppTryFrame *out_frame) {
    if (!stack || stack->count == 0) return false;
    if (out_frame) *out_frame = stack->frames[stack->count - 1];
    return true;
}

size_t try_stack_count(TryStack *stack) {
    return stack ? stack->count : 0;
}

/* Stack structures are defined inside this file, so we can access their private count fields here */

size_t gosub_stack_depth(GosubStack *stack) { return stack ? stack->count : 0; }
size_t for_stack_depth(ForStack *stack) { return stack ? stack->count : 0; }
size_t while_stack_depth(WhileStack *stack) { return stack ? stack->count : 0; }
size_t do_stack_depth(DoStack *stack) { return stack ? stack->count : 0; }
size_t select_stack_depth(SelectStack *stack) { return stack ? stack->count : 0; }
size_t sub_stack_depth(SubStack *stack) { return stack ? stack->count : 0; }

extern GosubStack  *vm_get_gosub_stack(VMContext *vm);
extern ForStack    *vm_get_for_stack(VMContext *vm);
extern WhileStack  *vm_get_while_stack(VMContext *vm);
extern DoStack     *vm_get_do_stack(VMContext *vm);
extern SelectStack *vm_get_select_stack(VMContext *vm);
extern SubStack    *vm_get_sub_stack(VMContext *vm);

void vm_restore_stack_depths(VMContext *vm, BppTryFrame frame) {
    if (!vm) return;
    GosubStack *gosub = vm_get_gosub_stack(vm);
    if (gosub && frame.gosub_stack_depth <= gosub->capacity) {
        gosub->count = frame.gosub_stack_depth;
    }
    ForStack *fstack = vm_get_for_stack(vm);
    if (fstack && frame.for_stack_depth <= fstack->capacity) {
        fstack->count = frame.for_stack_depth;
    }
    WhileStack *wstack = vm_get_while_stack(vm);
    if (wstack && frame.while_stack_depth <= wstack->capacity) {
        wstack->count = frame.while_stack_depth;
    }
    DoStack *dstack = vm_get_do_stack(vm);
    if (dstack && frame.do_stack_depth <= dstack->capacity) {
        dstack->count = frame.do_stack_depth;
    }
    SelectStack *sstack = vm_get_select_stack(vm);
    if (sstack && frame.select_stack_depth <= sstack->capacity) {
        sstack->count = frame.select_stack_depth;
    }
    SubStack *substack = vm_get_sub_stack(vm);
    if (substack && frame.sub_stack_depth <= substack->capacity) {
        substack->count = frame.sub_stack_depth;
    }
}

