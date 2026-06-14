/*
 * ---
 * BASIC++ Interpreter - parser_virtual.c
 * ---
 *
 * Virtual infrastructure introspection.
 *
 * VDEV, VMEM, VNET, VCON, VTERM, VMACH, DEVMAP.
 *
 * ---
 */

#include "parser_internal.h"

/*
 * pi_parse_vdev - Handle VDEV command.
 */
void pi_parse_vdev(Lexer *lex, RuntimeState *rt, int line_num)
{
 /*
  * VDEV - List all registered virtual devices.
  *
  * Shows slot ID, name, class, capability flags,
  * version, and description for every device in the
  * VDev table (CON:, ERR:, FILE:, N:, FUJI:, etc.).
  */
 vdev_list_all();
 return;
}

/*
 * pi_parse_vmem - Handle VMEM command.
 */
void pi_parse_vmem(Lexer *lex, RuntimeState *rt, int line_num)
{
 {
 /*
  * VMEM - Virtual memory status.
  *
  * Displays memory map preset, pool sizes, variable
  * storage, string pool, stack usage, and program
  * storage utilization.
  */
 int lines;
 int stack_used;

 printf("=== VIRTUAL MEMORY ===\n\n");
 printf(" Memory Map:\n");
 memmap_list();
 printf("\n");
 printf(" Variable Slots: %d (A-Z",
     MAX_VARIABLES);
 if (MAX_VARIABLES == 286) printf(" + arrays");
 printf(")\n");
 printf(" Stack Depth: %d max\n",
     MAX_STACK_DEPTH);
 stack_used = rt->stack_top;
 printf(" Stack Used: %d (%d%%)\n",
     stack_used,
     MAX_STACK_DEPTH > 0 ?
     (stack_used * 100) / MAX_STACK_DEPTH : 0);
 lines = (rt->program != NULL) ?
     rt->program->count : 0;
 printf(" Program Lines: %d / %d (%d%%)\n",
     lines, MAX_PROGRAM_LINES,
     MAX_PROGRAM_LINES > 0 ?
     (lines * 100) / MAX_PROGRAM_LINES : 0);
 printf(" Max Line Length: %d chars\n",
     MAX_LINE_LENGTH);
 printf(" Word Size: %d-bit\n",
     platform_word_size());
 printf(" sizeof(int): %d bytes\n",
     (int)sizeof(int));
 printf(" sizeof(long): %d bytes\n",
     (int)sizeof(long));
 printf(" sizeof(double): %d bytes\n",
     (int)sizeof(double));
 printf(" sizeof(void*): %d bytes\n",
     (int)sizeof(void *));
 return;
 }
}

/*
 * pi_parse_vnet - Handle VNET command.
 */
void pi_parse_vnet(Lexer *lex, RuntimeState *rt, int line_num)
{
 {
 /*
  * VNET - Virtual network status.
  *
  * Shows FujiNet N: channel table: which channels
  * are open, protocol, host, port, connection state,
  * bytes waiting, and error status.
  * If the FUJINET module is not active, says so.
  */
 int i;
 int any_open = 0;

 printf("=== VIRTUAL NETWORK ===\n\n");

 if (!module_is_active("FUJINET")) {
     printf(" FUJINET module is not active.\n");
     printf(" Use MODULE \"FUJINET\" to enable.\n");
     return;
 }

 printf(" Ch Proto   Host"
        "              Port  Status\n");
 printf(" -- -----   ----"
        "              ----  ------\n");

 for (i = 0; i < FN_MAX_CHANNELS; i++) {
     /* Access via vdev_info on the N: device */
     VDev *nd = vdev_get(
         vdev_find_by_name("N:"));
     if (nd == NULL) break;

     /* Check channel via a probe - channel state
      * is internal to mod_fujinet. We report what
      * vdev_info exposes. For now show the device
      * info as summary. */
     (void)i;
     break;
 }

 /* Channel summary from N: device info */
 {
     VDev *nd = vdev_get(
         vdev_find_by_name("N:"));
     if (nd) {
         const char *proto;
         const char *host;
         const char *port;
         const char *conn;
         proto = vdev_info(nd, "proto");
         host = vdev_info(nd, "host");
         port = vdev_info(nd, "port");
         conn = vdev_info(nd, "connected");
         if (proto && proto[0]) {
             printf("  0  %-7s %-18s %-5s %s\n",
                 proto,
                 host ? host : "(none)",
                 port ? port : "-",
                 (conn && conn[0] == '1') ?
                     "CONNECTED" : "IDLE");
             any_open = 1;
         }
     }
 }

 if (!any_open)
     printf(" No active connections.\n");

 /* Adapter summary */
 {
     VDev *fd = vdev_get(
         vdev_find_by_name("FUJI:"));
     if (fd) {
         const char *ssid;
         const char *ip;
         const char *wifi;
         const char *ver;
         printf("\n Adapter:\n");
         ssid = vdev_info(fd, "ssid");
         ip = vdev_info(fd, "ip");
         wifi = vdev_info(fd, "wifi");
         ver = vdev_info(fd, "version");
         if (ssid)
             printf("  SSID: %s\n", ssid);
         if (ip)
             printf("  IP: %s\n", ip);
         if (wifi)
             printf("  WiFi: %s\n", wifi);
         if (ver)
             printf("  FW: %s\n", ver);
     }
 }
 return;
 }
}

/*
 * pi_parse_vcon - Handle VCON command.
 */
void pi_parse_vcon(Lexer *lex, RuntimeState *rt, int line_num)
{
 {
 /*
  * VCON - Virtual console information.
  *
  * Shows the console device (CON:) status: device
  * class, capabilities, and the error output (ERR:).
  */
 VDev *con;
 VDev *err_dev;

 printf("=== VIRTUAL CONSOLE ===\n\n");

 con = vdev_get(VDEV_CON);
 if (con) {
     printf(" Console Device: %s\n",
         con->name);
     printf(" Class: %s\n",
         vdev_class_name(con->dev_class));
     printf(" Capabilities: %04X\n",
         con->dev_caps);
     printf(" Description: %s\n",
         con->dev_description ?
         con->dev_description : "(none)");
     printf(" Input: %s\n",
         con->dev_getc ? "YES" : "NO");
     printf(" Output: %s\n",
         con->dev_putc ? "YES" : "NO");
     printf(" Clear Screen: %s\n",
         con->dev_cls ? "YES" : "NO");
 }

 err_dev = vdev_get(VDEV_ERR);
 if (err_dev) {
     printf("\n Error Device: %s\n",
         err_dev->name);
     printf(" Class: %s\n",
         vdev_class_name(err_dev->dev_class));
     printf(" Description: %s\n",
         err_dev->dev_description ?
         err_dev->dev_description : "(none)");
 }
 return;
 }
}

/*
 * pi_parse_vterm - Handle VTERM command.
 */
void pi_parse_vterm(Lexer *lex, RuntimeState *rt, int line_num)
{
 {
 /*
  * VTERM - Virtual terminal information.
  *
  * Shows terminal type, screen dimensions (when
  * available), character encoding, and dialect-
  * specific terminal behavior.
  */
 printf("=== VIRTUAL TERMINAL ===\n\n");
 printf(" Dialect: %s\n",
     dialect_get_name());
 printf(" Encoding: ASCII (7-bit)\n");
 printf(" Columns: %d\n",
     rt->screen_width > 0 ?
         rt->screen_width : 80);
 printf(" Rows: %d\n",
     rt->screen_lines > 0 ?
         rt->screen_lines : 25);
 printf(" Cursor Col: %d\n",
     rt->cursor_col);
 printf(" Cursor Row: %d\n",
     rt->cursor_row);
 printf(" Screen Mode: %d\n",
     rt->screen_mode);
 printf(" Line Input: %d chars max\n",
     MAX_LINE_LENGTH);
 printf(" Tab Width: 8\n");
 printf(" Bell: %s\n",
     platform_short_name());
 return;
 }
}

/*
 * pi_parse_vmach - Handle VMACH command.
 */
void pi_parse_vmach(Lexer *lex, RuntimeState *rt, int line_num)
{
 {
 /*
  * VMACH - Virtual machine state.
  *
  * Shows VM execution state, dispatch table stats,
  * stack frame info, opcode count, and module/
  * security status.
  */
 VMState vstate;
 const char *state_name;
 int mcount;
 int i;
 int active_mods = 0;

 printf("=== VIRTUAL MACHINE ===\n\n");

 vstate = vm_get_state(rt);
 switch (vstate) {
 case VM_STOPPED: state_name = "STOPPED"; break;
 case VM_RUNNING: state_name = "RUNNING"; break;
 case VM_PAUSED: state_name = "PAUSED"; break;
 case VM_ERROR: state_name = "ERROR"; break;
 case VM_HALTED: state_name = "HALTED"; break;
 default: state_name = "UNKNOWN"; break;
 }
 printf(" State: %s\n", state_name);
 printf(" Opcodes: %d defined\n", OP_COUNT);
 printf(" Keywords: %d registered\n",
     KW_COUNT);
 printf(" Stack Depth: %d / %d\n",
     rt->stack_top, MAX_STACK_DEPTH);

 /* Module summary */
 mcount = module_count();
 for (i = 0; i < mcount; i++) {
     if (module_is_loaded(i)) active_mods++;
 }
 printf("\n Modules: %d registered, "
     "%d active\n", mcount, active_mods);
 for (i = 0; i < mcount; i++) {
     const ModuleInfo *mi = module_get(i);
     if (mi) {
         printf("  %-12s %s  [%s]\n",
             mi->name,
             module_is_loaded(i) ?
                 "ACTIVE " : "INACTIVE",
             module_class_name(mi->mod_class));
     }
 }

 printf("\n Security: %s\n",
     security_get_level() == 0 ?
         "UNRESTRICTED" : "RESTRICTED");
 printf(" Dialect: %s\n",
     dialect_get_name());
 printf(" Trace: %s\n",
     rt->trace_on ? "ON (TRON)" : "OFF (TROFF)");
 return;
 }
}

/*
 * pi_parse_devmap - Handle DEVMAP command.
 */
void pi_parse_devmap(Lexer *lex, RuntimeState *rt, int line_num)
{
 {
 /*
  * DEVMAP - Device slot mapping.
  *
  * Shows which VDev slot each BASIC file channel
  * (#1-#8) is mapped to, including open/closed
  * status and the device name in each slot. Also
  * lists all occupied VDev slots.
  */
 int i;
 int total = 0;

 printf("=== DEVICE MAP ===\n\n");

 printf(" BASIC File Channels:\n");
 printf(" Channel Device "
     "Status\n");
 printf(" ------- ------ "
     "------\n");
 for (i = 1; i <= 8; i++) {
     /* File channels are tracked by the FILE: VDev */
     printf("  #%-6d FILE:  ---\n", i);
 }

 printf("\n VDev Slot Table:\n");
 printf(" Slot Name       "
     "Class      Caps  Description\n");
 printf(" ---- ----       "
     "-----      ----  -----------\n");
 for (i = 0; i < VDEV_MAX; i++) {
     VDev *d = vdev_get(i);
     if (d == NULL) continue;
     printf("  %2d  %-10s %-10s %04X  %s\n",
         i,
         d->name ? d->name : "(null)",
         vdev_class_name(d->dev_class),
         d->dev_caps,
         d->dev_description ?
             d->dev_description : "");
     total++;
 }
 printf("\n %d device(s) registered, "
     "%d slots available.\n",
     total, VDEV_MAX - total);
 return;
 }

 /* ===== Final polish ===== */
}

