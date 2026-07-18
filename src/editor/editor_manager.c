#include "bpp_editor.h"
#include <string.h>

void editor_manager_init(VMContext *vm) {
    (void)vm;
}

int editor_manager_run(VMContext *vm, const char *editor_name, const char *filename) {
    if (!editor_name) return 0;
    
    if (strcmp(editor_name, "ws") == 0) {
        return mod_ws_main(vm, filename);
    } else if (strcmp(editor_name, "vi") == 0) {
        return mod_vi_main(vm, filename);
    } else if (strcmp(editor_name, "edit") == 0) {
        return mod_edit_main(vm, filename);
    } else if (strcmp(editor_name, "edlin") == 0) {
        return mod_edlin_main(vm, filename);
    }
    
    return -1;
}
