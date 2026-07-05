#ifndef MOD_GWBASIC_H
#define MOD_GWBASIC_H

// Global machine type configured from command line switches
extern int g_gw_machine_type;
extern int g_cga_snow;
extern char g_mda_color[16];

// Register GWBASIC module in module manager
void mod_gwbasic_register(void);

#endif // MOD_GWBASIC_H
