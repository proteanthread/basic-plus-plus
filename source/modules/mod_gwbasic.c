#include "mod_gwbasic.h"
#include "module.h"
#include "gw_memory.h"
#include "gw_plugin.h"
#include "gw_sdl2.h"
#include <stddef.h>

struct GW_Memory *g_gw_mem = NULL;
struct GW_PluginManager *g_gw_plugin_mgr = NULL;
int g_gw_machine_type = 0; // Default to VGA

static int gwbasic_init(void *rt)
{
    (void)rt;
    if (!g_gw_mem) {
        g_gw_mem = gw_mem_create(1048576); // 1MB virtual segment memory
    }
    if (!g_gw_plugin_mgr) {
        g_gw_plugin_mgr = gw_plugin_init();
    }
#ifndef NO_SDL2
    gw_sdl2_set_machine(g_gw_machine_type);
#endif
    return 0;
}

static void gwbasic_cleanup(void)
{
    if (g_gw_mem) {
        gw_mem_destroy(g_gw_mem);
        g_gw_mem = NULL;
    }
    if (g_gw_plugin_mgr) {
        gw_plugin_cleanup(g_gw_plugin_mgr);
        g_gw_plugin_mgr = NULL;
    }
#ifndef NO_SDL2
    gw_sdl2_cleanup();
#endif
}

static const ModuleInfo gwbasic_module_info = {
    "GWBASIC",
    "1.0",
    "GW-BASIC Port Integration Module",
    MOD_EXTENSION,
    CAP_MATH | CAP_STRING | CAP_IO | CAP_GRAPHICS | CAP_SOUND,
    gwbasic_init,
    gwbasic_cleanup
};

void mod_gwbasic_register(void)
{
    module_register(&gwbasic_module_info);
}
