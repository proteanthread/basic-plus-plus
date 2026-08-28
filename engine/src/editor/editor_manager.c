// FILENAME: editor_manager.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libstandard (editor.h, editor.c)
// Implements visual text editor subsystem components for editor_manager.
//
// ---- Includes ----

#include "editor/editor.h"
#include "runtime/string/strops.h"

void editor_manager_init(VMContext *vm) {
    (void)vm;
}

int editor_manager_run(VMContext *vm, const char *editor_name, const char *filename) {
    if (!editor_name) return 0;
    
    if (runtime_strcmp(editor_name, "ws") == 0) {
        return mod_ws_main(vm, filename);
    } else if (runtime_strcmp(editor_name, "vi") == 0) {
        return mod_vi_main(vm, filename);
    } else if (runtime_strcmp(editor_name, "edit") == 0) {
        return mod_edit_main(vm, filename);
    } else if (runtime_strcmp(editor_name, "edlin") == 0) {
        return mod_edlin_main(vm, filename);
    }
    
    return -1;
}
