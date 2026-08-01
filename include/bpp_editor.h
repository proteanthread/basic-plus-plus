/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
#ifndef BPP_EDITOR_H
#define BPP_EDITOR_H

#include "bpp_vm.h"
#include <stdbool.h>

/**
 * Common editor plugin interface.
 */
typedef struct {
    const char *name;
    int (*run_editor)(VMContext *vm, const char *filename);
} BppEditorPlugin;

void tui_multiplexer_init(void);
void tui_multiplexer_shutdown(void);

void editor_manager_init(VMContext *vm);
int editor_manager_run(VMContext *vm, const char *editor_name, const char *filename);

/* Plugin entry points */
int mod_ws_main(VMContext *vm, const char *filename);
int mod_vi_main(VMContext *vm, const char *filename);
int mod_edit_main(VMContext *vm, const char *filename);
int mod_edlin_main(VMContext *vm, const char *filename);

#endif /* BPP_EDITOR_H */
