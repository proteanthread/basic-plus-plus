// FILENAME: stack.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (alloc.h, alloc.c, hal.h, memops.h, memops.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (config.h)
// Implements bytecode virtual machine execution and state for stack.
//
// ---- Includes ----

#include "vm/vm.h"
#include "types/config.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "hal/hal.h"

typedef struct {
    BppLineNumber line;
    const char   *pos;
} GosubFrame;

// Private GOSUB stack representation
typedef struct GosubStack {
    GosubFrame *frames;
    size_t      count;
    size_t      capacity;
} GosubStack;

// Stored inside the VM Context structure. We'll expose helpers:
GosubStack *gosub_stack_init(void) {
    HalContext *hal = hal_get();
    GosubStack *stack = NULL;
    if (hal && hal->mem.alloc) {
        stack = (GosubStack *)hal->mem.alloc(sizeof(GosubStack));
    }
    if (!stack) return NULL;
    runtime_memset(stack, 0, sizeof(GosubStack));
    stack->capacity = BASIC_MAX_STACK_DEPTH;
    stack->count = 0;
    if (hal && hal->mem.alloc) {
        stack->frames = (GosubFrame *)hal->mem.alloc(stack->capacity * sizeof(GosubFrame));
    }
    if (!stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack);
        return NULL;
    }
    runtime_memset(stack->frames, 0, stack->capacity * sizeof(GosubFrame));
    return stack;
}

void gosub_stack_shutdown(GosubStack *stack) {
    if (!stack) return;
    HalContext *hal = hal_get();
    if (stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack->frames);
        stack->frames = NULL;
    }
    if (hal && hal->mem.free) {
        hal->mem.free(stack);
    }
}

void gosub_stack_clear(GosubStack *stack) {
    if (stack) {
        stack->count = 0;
    }
}

bool gosub_stack_push(GosubStack *stack, BppLineNumber line, const char *pos) {
    if (!stack) return false;
    if (stack->count >= stack->capacity) {
        return false; // Stack overflow
    }
    stack->frames[stack->count].line = line;
    stack->frames[stack->count].pos = pos;
    stack->count++;
    return true;
}

bool gosub_stack_pop(GosubStack *stack, BppLineNumber *out_line, const char **out_pos) {
    if (!stack || stack->count == 0) {
        return false; // Stack underflow
    }
    stack->count--;
    if (out_line) *out_line = stack->frames[stack->count].line;
    if (out_pos) *out_pos = stack->frames[stack->count].pos;
    return true;
}

// Private FOR stack representation
typedef struct ForStack {
    BppForFrame *frames;
    size_t       count;
    size_t       capacity;
} ForStack;

ForStack *for_stack_init(void) {
    HalContext *hal = hal_get();
    ForStack *stack = NULL;
    if (hal && hal->mem.alloc) {
        stack = (ForStack *)hal->mem.alloc(sizeof(ForStack));
    }
    if (!stack) return NULL;
    runtime_memset(stack, 0, sizeof(ForStack));
    stack->capacity = BASIC_MAX_STACK_DEPTH;
    stack->count = 0;
    if (hal && hal->mem.alloc) {
        stack->frames = (BppForFrame *)hal->mem.alloc(stack->capacity * sizeof(BppForFrame));
    }
    if (!stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack);
        return NULL;
    }
    runtime_memset(stack->frames, 0, stack->capacity * sizeof(BppForFrame));
    return stack;
}

void for_stack_shutdown(ForStack *stack) {
    if (!stack) return;
    HalContext *hal = hal_get();
    if (stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack->frames);
        stack->frames = NULL;
    }
    if (hal && hal->mem.free) {
        hal->mem.free(stack);
    }
}

void for_stack_clear(ForStack *stack) {
    if (stack) {
        stack->count = 0;
    }
}

static bool for_stack_matches_var(const BppForFrame *f, const char *var_name) {
    if (!f) return false;
    if (!var_name || var_name[0] == '\0') return true;
    if (runtime_strcasecmp(f->var_name, var_name) == 0) return true;
    for (int i = 0; i < f->var_count - 1 && i < 7; i++) {
        if (runtime_strcasecmp(f->extra_vars[i], var_name) == 0) return true;
    }
    return false;
}

bool for_stack_pop(ForStack *stack, const char *var_name, BppForFrame *out_frame);

bool for_stack_push_multi(ForStack *stack, const char **var_names, int var_count, double target, double step, BppLineNumber line, const char *pos) {
    if (!stack) return false;
    if (var_names && var_names[0] && var_names[0][0]) {
        for_stack_pop(stack, var_names[0], NULL);
    }
    if (stack->count >= stack->capacity) return false;
    BppForFrame *f = &stack->frames[stack->count];
    runtime_memset(f, 0, sizeof(*f));
    f->var_count = (var_count > 8) ? 8 : (var_count < 1 ? 1 : var_count);
    if (var_names && var_names[0]) {
        runtime_strncpy(f->var_name, var_names[0], sizeof(f->var_name) - 1);
        f->var_name[sizeof(f->var_name) - 1] = '\0';
    }
    for (int i = 1; i < f->var_count && i < 8; i++) {
        if (var_names && var_names[i]) {
            runtime_strncpy(f->extra_vars[i - 1], var_names[i], sizeof(f->extra_vars[i - 1]) - 1);
            f->extra_vars[i - 1][sizeof(f->extra_vars[i - 1]) - 1] = '\0';
        }
    }
    f->target = target;
    f->step   = step;
    f->line   = line;
    f->pos    = pos;
    stack->count++;
    return true;
}

bool for_stack_push(ForStack *stack, const char *var_name, double target, double step, BppLineNumber line, const char *pos) {
    const char *names[1];
    names[0] = var_name;
    return for_stack_push_multi(stack, names, 1, target, step, line, pos);
}

bool for_stack_update(ForStack *stack, const char *var_name, double target, double step, const char *next_range_pos) {
    if (!stack || stack->count == 0) return false;
    if (var_name && runtime_strlen(var_name) > 0) {
        for (int i = (int)stack->count - 1; i >= 0; --i) {
            if (for_stack_matches_var(&stack->frames[i], var_name)) {
                stack->frames[i].target = target;
                stack->frames[i].step = step;
                stack->frames[i].next_range_pos = next_range_pos;
                return true;
            }
        }
    }
    return false;
}

void for_stack_set_cached_ptr(ForStack *stack, BValue *ptr) {
    if (stack && stack->count > 0) {
        stack->frames[stack->count - 1].cached_var_ptr = ptr;
    }
}

bool for_stack_pop(ForStack *stack, const char *var_name, BppForFrame *out_frame) {
    if (!stack || stack->count == 0) return false;
    
    if (var_name && runtime_strlen(var_name) > 0) {
        for (int i = (int)stack->count - 1; i >= 0; --i) {
            if (for_stack_matches_var(&stack->frames[i], var_name)) {
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
    
    if (var_name && runtime_strlen(var_name) > 0) {
        for (int i = (int)stack->count - 1; i >= 0; --i) {
            if (for_stack_matches_var(&stack->frames[i], var_name)) {
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

// Private WHILE loop stack representation
typedef struct WhileStack {
    GosubFrame *frames;
    size_t      count;
    size_t      capacity;
} WhileStack;

WhileStack *while_stack_init(void) {
    HalContext *hal = hal_get();
    WhileStack *stack = NULL;
    if (hal && hal->mem.alloc) {
        stack = (WhileStack *)hal->mem.alloc(sizeof(WhileStack));
    }
    if (!stack) return NULL;
    runtime_memset(stack, 0, sizeof(WhileStack));
    stack->capacity = BASIC_MAX_STACK_DEPTH;
    stack->count = 0;
    if (hal && hal->mem.alloc) {
        stack->frames = (GosubFrame *)hal->mem.alloc(stack->capacity * sizeof(GosubFrame));
    }
    if (!stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack);
        return NULL;
    }
    runtime_memset(stack->frames, 0, stack->capacity * sizeof(GosubFrame));
    return stack;
}

void while_stack_shutdown(WhileStack *stack) {
    if (!stack) return;
    HalContext *hal = hal_get();
    if (stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack->frames);
        stack->frames = NULL;
    }
    if (hal && hal->mem.free) {
        hal->mem.free(stack);
    }
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

// Private DO loop stack representation
typedef struct DoStack {
    GosubFrame *frames;
    size_t      count;
    size_t      capacity;
} DoStack;

DoStack *do_stack_init(void) {
    HalContext *hal = hal_get();
    DoStack *stack = NULL;
    if (hal && hal->mem.alloc) {
        stack = (DoStack *)hal->mem.alloc(sizeof(DoStack));
    }
    if (!stack) return NULL;
    runtime_memset(stack, 0, sizeof(DoStack));
    stack->capacity = BASIC_MAX_STACK_DEPTH;
    stack->count = 0;
    if (hal && hal->mem.alloc) {
        stack->frames = (GosubFrame *)hal->mem.alloc(stack->capacity * sizeof(GosubFrame));
    }
    if (!stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack);
        return NULL;
    }
    runtime_memset(stack->frames, 0, stack->capacity * sizeof(GosubFrame));
    return stack;
}

void do_stack_shutdown(DoStack *stack) {
    if (!stack) return;
    HalContext *hal = hal_get();
    if (stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack->frames);
        stack->frames = NULL;
    }
    if (hal && hal->mem.free) {
        hal->mem.free(stack);
    }
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

// Private SELECT CASE stack representation
struct SelectStack {
    BppSelectFrame *frames;
    size_t          count;
    size_t          capacity;
};

SelectStack *select_stack_init(void) {
    HalContext *hal = hal_get();
    SelectStack *stack = NULL;
    if (hal && hal->mem.alloc) {
        stack = (SelectStack *)hal->mem.alloc(sizeof(SelectStack));
    }
    if (!stack) return NULL;
    runtime_memset(stack, 0, sizeof(SelectStack));
    stack->capacity = BASIC_MAX_STACK_DEPTH;
    stack->count = 0;
    if (hal && hal->mem.alloc) {
        stack->frames = (BppSelectFrame *)hal->mem.alloc(stack->capacity * sizeof(BppSelectFrame));
    }
    if (!stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack);
        return NULL;
    }
    runtime_memset(stack->frames, 0, stack->capacity * sizeof(BppSelectFrame));
    return stack;
}

void select_stack_shutdown(SelectStack *stack) {
    if (!stack) return;
    HalContext *hal = hal_get();
    if (stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack->frames);
        stack->frames = NULL;
    }
    if (hal && hal->mem.free) {
        hal->mem.free(stack);
    }
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

// Private SUB/FUNCTION stack representation
struct SubStack {
    BppSubFrame *frames;
    size_t       count;
    size_t       capacity;
};

SubStack *sub_stack_init(void) {
    HalContext *hal = hal_get();
    SubStack *stack = NULL;
    if (hal && hal->mem.alloc) {
        stack = (SubStack *)hal->mem.alloc(sizeof(SubStack));
    }
    if (!stack) return NULL;
    runtime_memset(stack, 0, sizeof(SubStack));
    stack->capacity = BASIC_MAX_STACK_DEPTH;
    stack->count = 0;
    if (hal && hal->mem.alloc) {
        stack->frames = (BppSubFrame *)hal->mem.alloc(stack->capacity * sizeof(BppSubFrame));
    }
    if (!stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack);
        return NULL;
    }
    runtime_memset(stack->frames, 0, stack->capacity * sizeof(BppSubFrame));
    return stack;
}

void sub_stack_shutdown(SubStack *stack) {
    if (!stack) return;
    HalContext *hal = hal_get();
    if (stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack->frames);
        stack->frames = NULL;
    }
    if (hal && hal->mem.free) {
        hal->mem.free(stack);
    }
}

void sub_stack_clear(SubStack *stack) {
    if (stack) {
        stack->count = 0;
    }
}

bool sub_stack_push(SubStack *stack, const char *name, BppLineNumber line, const char *pos, bool is_func) {
    if (!stack || stack->count >= stack->capacity) return false;
    runtime_strncpy(stack->frames[stack->count].name, name, sizeof(stack->frames[stack->count].name) - 1);
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

// TryStack Implementation
struct TryStack {
    BppTryFrame *frames;
    size_t       count;
    size_t       capacity;
};

TryStack *try_stack_init(void) {
    HalContext *hal = hal_get();
    TryStack *stack = NULL;
    if (hal && hal->mem.alloc) {
        stack = (TryStack *)hal->mem.alloc(sizeof(TryStack));
    }
    if (!stack) return NULL;
    runtime_memset(stack, 0, sizeof(TryStack));
    stack->capacity = BASIC_MAX_STACK_DEPTH;
    stack->count = 0;
    if (hal && hal->mem.alloc) {
        stack->frames = (BppTryFrame *)hal->mem.alloc(stack->capacity * sizeof(BppTryFrame));
    }
    if (!stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack);
        return NULL;
    }
    runtime_memset(stack->frames, 0, stack->capacity * sizeof(BppTryFrame));
    return stack;
}

void try_stack_shutdown(TryStack *stack) {
    if (!stack) return;
    HalContext *hal = hal_get();
    if (stack->frames) {
        if (hal && hal->mem.free) hal->mem.free(stack->frames);
        stack->frames = NULL;
    }
    if (hal && hal->mem.free) {
        hal->mem.free(stack);
    }
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

// Stack structures are defined inside this file, so we can access their private count fields here


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

