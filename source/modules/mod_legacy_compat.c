/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Virtual memory segment offsets (e.g. 0x7000 for scalars, 0x8000 for strings).
 *    - Variable mapping hooks converting between BASIC++ internal value representation
 *      (BValue) and legacy binary representations (Microsoft Binary Format / MBF).
 *    - Module capabilities, description metadata, and module initialization/cleanup flow.
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - The module name ("GWBASIC") and registration callback signature.
 *    - The core memory read/write hook dispatch contracts mapping virtual addresses.
 *    - The lifecycle bindings linking GWBASIC activation to dialect activation in boot.c.
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Emulation of virtual memory regions backing direct PEEK, POKE, and VARPTR.
 *    - Transparent redirection of graphics buffer character peeks to SDL2 text cache.
 *    - Bidirectional variable synchronizations keeping variables and simulated RAM aligned.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Verify memory address offsets match tests/test_gw_compat.bas.
 *    - Check single/double type suffix conversions and MBF floating point conversion logic.
 *    - Inspect runtime deftype layouts and verify standard test outputs using the test runner.
 * -----------------------------------------------------------------------------
 */
#include "mod_legacy_compat.h"
#include "module.h"
#include "segmented_mem.h"
#include "compat_plugin.h"
#include "sdl2_emu.h"
#include "value.h"
#include "runtime.h"
#include "mbf_math.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "legacy_compat.h"

struct GW_Memory *g_gw_mem = NULL;
GW_State *g_state = NULL;
struct GW_PluginManager *g_gw_plugin_mgr = NULL;
#ifdef INPUT_CONSOLE
int g_gw_machine_type = 8; // Default to MACHINE_MDA (Monochrome Text Mode) for Console/text-only builds
#else
int g_gw_machine_type = 9; // Default to MACHINE_CGA (Color Graphics Adapter) for SDL GUI builds
#endif
int g_cga_snow = 0;
char g_mda_color[16] = "amber";

static uint8_t gw_var_read_hook(uint32_t addr, void *ctx) {
    RuntimeState *rt = (RuntimeState *)ctx;
    if (!rt) return 0;
    
    int var_offset = (addr - 0x7000);
    int var_idx = var_offset / 8;
    int byte_offset = var_offset % 8;
    
    if (var_idx < 26) {
        BValue val = rt->variables[var_idx];
        unsigned char dtype = rt->deftype_map[var_idx];
        if (dtype == DEFTYPE_NONE) {
            if (bval_is_int(&val)) dtype = DEFTYPE_INT;
            else if (bval_is_float(&val)) dtype = DEFTYPE_SNG;
            else if (bval_is_string(&val)) dtype = DEFTYPE_STR;
            else dtype = DEFTYPE_SNG;
        }
        
        if (dtype == DEFTYPE_INT) {
            if (byte_offset < 2) {
                int16_t ival = (int16_t)bval_to_int(&val);
                return (uint8_t)((ival >> (byte_offset * 8)) & 0xFF);
            }
            return 0;
        } else if (dtype == DEFTYPE_SNG) {
            if (byte_offset < 4) {
                uint8_t mbf[4];
                gw_double_to_mbf32(bval_to_float(&val), mbf);
                return mbf[byte_offset];
            }
            return 0;
        } else if (dtype == DEFTYPE_DBL) {
            uint8_t mbf[8];
            gw_double_to_mbf64(bval_to_float(&val), mbf);
            return mbf[byte_offset];
        } else if (dtype == DEFTYPE_STR) {
            if (byte_offset == 0) {
                return (uint8_t)(bval_is_string(&val) ? val.v.sval.length & 0xFF : 0);
            } else if (byte_offset == 1) {
                uint16_t str_offset = 0x8000 + var_idx * 256;
                return (uint8_t)(str_offset & 0xFF);
            } else if (byte_offset == 2) {
                uint16_t str_offset = 0x8000 + var_idx * 256;
                return (uint8_t)((str_offset >> 8) & 0xFF);
            }
            return 0;
        }
    } else if (var_idx >= 26 && var_idx < 52) {
        int str_idx = var_idx - 26;
        BValue val = rt->string_vars[str_idx];
        if (byte_offset == 0) {
            return (uint8_t)(bval_is_string(&val) ? val.v.sval.length & 0xFF : 0);
        } else if (byte_offset == 1) {
            uint16_t str_offset = 0x8000 + var_idx * 256;
            return (uint8_t)(str_offset & 0xFF);
        } else if (byte_offset == 2) {
            uint16_t str_offset = 0x8000 + var_idx * 256;
            return (uint8_t)((str_offset >> 8) & 0xFF);
        }
        return 0;
    } else if (var_idx >= 52) {
        int named_idx = var_idx - 52;
        if (named_idx < rt->named_count) {
            BValue val = rt->named_vars[named_idx].value;
            const char *name = rt->named_vars[named_idx].name;
            int name_len = (int)strlen(name);
            char suffix = (name_len > 0) ? name[name_len - 1] : '\0';
            
            if (suffix == '%') {
                if (byte_offset < 2) {
                    int16_t ival = (int16_t)bval_to_int(&val);
                    return (uint8_t)((ival >> (byte_offset * 8)) & 0xFF);
                }
                return 0;
            } else if (suffix == '$') {
                if (byte_offset == 0) {
                    return (uint8_t)(bval_is_string(&val) ? val.v.sval.length & 0xFF : 0);
                } else if (byte_offset == 1) {
                    uint16_t str_offset = 0x8000 + var_idx * 256;
                    return (uint8_t)(str_offset & 0xFF);
                } else if (byte_offset == 2) {
                    uint16_t str_offset = 0x8000 + var_idx * 256;
                    return (uint8_t)((str_offset >> 8) & 0xFF);
                }
                return 0;
            } else if (suffix == '#') {
                uint8_t mbf[8];
                gw_double_to_mbf64(bval_to_float(&val), mbf);
                return mbf[byte_offset];
            } else {
                if (byte_offset < 4) {
                    uint8_t mbf[4];
                    gw_double_to_mbf32(bval_to_float(&val), mbf);
                    return mbf[byte_offset];
                }
                return 0;
            }
        }
    }
    return 0;
}

static void gw_var_write_hook(uint32_t addr, uint8_t val, void *ctx) {
    RuntimeState *rt = (RuntimeState *)ctx;
    if (!rt) return;
    
    int var_offset = (addr - 0x7000);
    int var_idx = var_offset / 8;
    int byte_offset = var_offset % 8;
    
    if (var_idx < 26) {
        BValue *val_ptr = &rt->variables[var_idx];
        unsigned char dtype = rt->deftype_map[var_idx];
        if (dtype == DEFTYPE_NONE) {
            if (bval_is_int(val_ptr)) dtype = DEFTYPE_INT;
            else if (bval_is_float(val_ptr)) dtype = DEFTYPE_SNG;
            else if (bval_is_string(val_ptr)) dtype = DEFTYPE_STR;
            else dtype = DEFTYPE_SNG;
        }
        
        if (dtype == DEFTYPE_INT) {
            if (byte_offset < 2) {
                int16_t ival = (int16_t)bval_to_int(val_ptr);
                if (byte_offset == 0) {
                    ival = (ival & 0xFF00) | val;
                } else {
                    ival = (ival & 0x00FF) | ((int16_t)val << 8);
                }
                *val_ptr = bval_int(ival);
            }
        } else if (dtype == DEFTYPE_SNG) {
            if (byte_offset < 4) {
                uint8_t mbf[4];
                gw_double_to_mbf32(bval_to_float(val_ptr), mbf);
                mbf[byte_offset] = val;
                double dval = gw_mbf32_to_double(mbf);
                *val_ptr = bval_float(dval);
            }
        } else if (dtype == DEFTYPE_DBL) {
            uint8_t mbf[8];
            gw_double_to_mbf64(bval_to_float(val_ptr), mbf);
            mbf[byte_offset] = val;
            double dval = gw_mbf64_to_double(mbf);
            *val_ptr = bval_float(dval);
        } else if (dtype == DEFTYPE_STR) {
            if (byte_offset == 0) {
                if (bval_is_string(val_ptr)) {
                    int new_len = val;
                    if (new_len > val_ptr->v.sval.length) new_len = val_ptr->v.sval.length;
                    val_ptr->v.sval.length = new_len;
                }
            }
        }
    } else if (var_idx >= 26 && var_idx < 52) {
        int str_idx = var_idx - 26;
        BValue *val_ptr = &rt->string_vars[str_idx];
        if (byte_offset == 0) {
            if (bval_is_string(val_ptr)) {
                int new_len = val;
                if (new_len > val_ptr->v.sval.length) new_len = val_ptr->v.sval.length;
                val_ptr->v.sval.length = new_len;
            }
        }
    } else if (var_idx >= 52) {
        int named_idx = var_idx - 52;
        if (named_idx < rt->named_count) {
            BValue *val_ptr = &rt->named_vars[named_idx].value;
            const char *name = rt->named_vars[named_idx].name;
            int name_len = (int)strlen(name);
            char suffix = (name_len > 0) ? name[name_len - 1] : '\0';
            
            if (suffix == '%') {
                if (byte_offset < 2) {
                    int16_t ival = (int16_t)bval_to_int(val_ptr);
                    if (byte_offset == 0) {
                        ival = (ival & 0xFF00) | val;
                    } else {
                        ival = (ival & 0x00FF) | ((int16_t)val << 8);
                    }
                    *val_ptr = bval_int(ival);
                }
            } else if (suffix == '$') {
                if (byte_offset == 0) {
                    if (bval_is_string(val_ptr)) {
                        int new_len = val;
                        if (new_len > val_ptr->v.sval.length) new_len = val_ptr->v.sval.length;
                        val_ptr->v.sval.length = new_len;
                    }
                }
            } else if (suffix == '#') {
                uint8_t mbf[8];
                gw_double_to_mbf64(bval_to_float(val_ptr), mbf);
                mbf[byte_offset] = val;
                double dval = gw_mbf64_to_double(mbf);
                *val_ptr = bval_float(dval);
            } else {
                if (byte_offset < 4) {
                    uint8_t mbf[4];
                    gw_double_to_mbf32(bval_to_float(val_ptr), mbf);
                    mbf[byte_offset] = val;
                    double dval = gw_mbf32_to_double(mbf);
                    *val_ptr = bval_float(dval);
                }
            }
        }
    }
}

static uint8_t gw_string_read_hook(uint32_t addr, void *ctx) {
    RuntimeState *rt = (RuntimeState *)ctx;
    if (!rt) return 0;
    
    int str_offset = (addr - 0x8000);
    int var_idx = str_offset / 256;
    int char_offset = str_offset % 256;
    
    BValue val;
    if (var_idx < 26) {
        val = rt->variables[var_idx];
    } else if (var_idx >= 26 && var_idx < 52) {
        val = rt->string_vars[var_idx - 26];
    } else {
        int named_idx = var_idx - 52;
        if (named_idx < rt->named_count) {
            val = rt->named_vars[named_idx].value;
        } else {
            return 0;
        }
    }
    
    if (bval_is_string(&val)) {
        if (char_offset < val.v.sval.length && val.v.sval.data) {
            return (uint8_t)val.v.sval.data[char_offset];
        }
    }
    return 0;
}

static void gw_string_write_hook(uint32_t addr, uint8_t val, void *ctx) {
    RuntimeState *rt = (RuntimeState *)ctx;
    if (!rt) return;
    
    int str_offset = (addr - 0x8000);
    int var_idx = str_offset / 256;
    int char_offset = str_offset % 256;
    
    BValue *val_ptr = NULL;
    if (var_idx < 26) {
        val_ptr = &rt->variables[var_idx];
    } else if (var_idx >= 26 && var_idx < 52) {
        val_ptr = &rt->string_vars[var_idx - 26];
    } else {
        int named_idx = var_idx - 52;
        if (named_idx < rt->named_count) {
            val_ptr = &rt->named_vars[named_idx].value;
        }
    }
    
    if (val_ptr && bval_is_string(val_ptr)) {
        if (char_offset < val_ptr->v.sval.length && val_ptr->v.sval.data) {
            val_ptr->v.sval.data[char_offset] = (char)val;
        }
    }
}

static uint8_t gw_array_read_hook(uint32_t addr, void *ctx) {
    RuntimeState *rt = (RuntimeState *)ctx;
    if (!rt) return 0;
    
    int element_offset = (addr - 0x10000) / 8;
    int byte_offset = (addr - 0x10000) % 8;
    if (element_offset < 0 || element_offset >= rt->dim_elements_used) {
        return rt->mem_segment[addr % 1048576];
    }
    
    BValue val = rt->dim_elements[element_offset];
    int is_int = 0;
    int is_double = 1;
    int is_str = 0;
    
    for (int i = 0; i < rt->dim_count; i++) {
        DimArray *arr = &rt->dim_arrays[i];
        int start_idx = (int)(arr->elements - rt->dim_elements);
        if (element_offset >= start_idx && element_offset < start_idx + arr->total) {
            int name_len = (int)strlen(arr->name);
            if (name_len > 0) {
                char suffix = arr->name[name_len - 1];
                if (suffix == '%') { is_int = 1; is_double = 0; }
                else if (suffix == '!') { is_double = 0; }
                else if (suffix == '$') { is_str = 1; is_double = 0; }
            }
            break;
        }
    }
    
    if (is_int) {
        if (byte_offset < 2) {
            int16_t ival = (int16_t)bval_to_int(&val);
            return (uint8_t)((ival >> (byte_offset * 8)) & 0xFF);
        }
        return 0;
    } else if (is_str) {
        if (byte_offset == 0) {
            return (uint8_t)(bval_is_string(&val) ? val.v.sval.length & 0xFF : 0);
        } else if (byte_offset == 1) {
            uint16_t str_offset = 0x8000 + (element_offset % 32) * 256;
            return (uint8_t)(str_offset & 0xFF);
        } else if (byte_offset == 2) {
            uint16_t str_offset = 0x8000 + (element_offset % 32) * 256;
            return (uint8_t)((str_offset >> 8) & 0xFF);
        }
        return 0;
    } else {
        if (is_double) {
            uint8_t mbf[8];
            gw_double_to_mbf64(bval_to_float(&val), mbf);
            return mbf[byte_offset];
        } else {
            if (byte_offset < 4) {
                uint8_t mbf[4];
                gw_double_to_mbf32(bval_to_float(&val), mbf);
                return mbf[byte_offset];
            }
            return 0;
        }
    }
}

static void gw_array_write_hook(uint32_t addr, uint8_t val, void *ctx) {
    RuntimeState *rt = (RuntimeState *)ctx;
    if (!rt) return;
    
    int element_offset = (addr - 0x10000) / 8;
    int byte_offset = (addr - 0x10000) % 8;
    if (element_offset < 0 || element_offset >= rt->dim_elements_used) {
        rt->mem_segment[addr % 1048576] = val;
        return;
    }
    
    BValue *val_ptr = &rt->dim_elements[element_offset];
    int is_int = 0;
    int is_double = 1;
    int is_str = 0;
    
    for (int i = 0; i < rt->dim_count; i++) {
        DimArray *arr = &rt->dim_arrays[i];
        int start_idx = (int)(arr->elements - rt->dim_elements);
        if (element_offset >= start_idx && element_offset < start_idx + arr->total) {
            int name_len = (int)strlen(arr->name);
            if (name_len > 0) {
                char suffix = arr->name[name_len - 1];
                if (suffix == '%') { is_int = 1; is_double = 0; }
                else if (suffix == '!') { is_double = 0; }
                else if (suffix == '$') { is_str = 1; is_double = 0; }
            }
            break;
        }
    }
    
    if (is_int) {
        if (byte_offset < 2) {
            int16_t ival = (int16_t)bval_to_int(val_ptr);
            if (byte_offset == 0) {
                ival = (ival & 0xFF00) | val;
            } else {
                ival = (ival & 0x00FF) | ((int16_t)val << 8);
            }
            *val_ptr = bval_int(ival);
        }
    } else if (is_str) {
        if (byte_offset == 0) {
            if (bval_is_string(val_ptr)) {
                int new_len = val;
                if (new_len > val_ptr->v.sval.length) new_len = val_ptr->v.sval.length;
                val_ptr->v.sval.length = new_len;
            }
        }
    } else {
        if (is_double) {
            uint8_t mbf[8];
            gw_double_to_mbf64(bval_to_float(val_ptr), mbf);
            mbf[byte_offset] = val;
            double dval = gw_mbf64_to_double(mbf);
            *val_ptr = bval_float(dval);
        } else {
            if (byte_offset < 4) {
                uint8_t mbf[4];
                gw_double_to_mbf32(bval_to_float(val_ptr), mbf);
                mbf[byte_offset] = val;
                double dval = gw_mbf32_to_double(mbf);
                *val_ptr = bval_float(dval);
            }
        }
    }
}

static int gwbasic_init(void *rt)
{
    if (!g_gw_mem) {
        g_gw_mem = gw_mem_create(1048576); // 1MB virtual segment memory
    } else {
        gw_mem_clear_hooks(g_gw_mem);
    }
    
    // Register the bridges between GWBASIC virtual memory and basicpp variables
    gw_mem_register_read_hook(g_gw_mem, 0x7000, 0x7FFF, gw_var_read_hook, rt);
    gw_mem_register_write_hook(g_gw_mem, 0x7000, 0x7FFF, gw_var_write_hook, rt);
    
    gw_mem_register_read_hook(g_gw_mem, 0x8000, 0xFFFF, gw_string_read_hook, rt);
    gw_mem_register_write_hook(g_gw_mem, 0x8000, 0xFFFF, gw_string_write_hook, rt);
    
    gw_mem_register_read_hook(g_gw_mem, 0x10000, 0xFFFFF, gw_array_read_hook, rt);
    gw_mem_register_write_hook(g_gw_mem, 0x10000, 0xFFFFF, gw_array_write_hook, rt);

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
