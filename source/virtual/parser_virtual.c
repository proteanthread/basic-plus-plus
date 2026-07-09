/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: parser_virtual.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Virtual Device Layer (VDev), Virtual Machine opcode dispatcher, and platform memory mappings.
 *
 * 2. WHAT TO EXPECT:
 *    VDev slots abstract screen, keyboard, files, and GPIO. VM runs bytecode dispatch.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Virtual memory preset maps, new peripheral classes, custom hardware simulation registers.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    VM opcodes, VM stack math, VDev operations interface.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Trace I/O issues in VDev registration calls. Verify memory offsets are within configured segments.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - parser_virtual.c
 // ---
 //
 // Virtual infrastructure introspection.
 //
 // VDEV, VMEM, VNET, VCON, VTERM, VMACH, DEVMAP.
 //
//
// HOW TO EXTEND:
//   To add a new statement or sub-command:
//   1. Add the keyword to lexer.h (KeywordId enum).
//   2. Add it to the keyword table in lexer.c.
//   3. Add a handler function in this file.
//   4. Wire it into parser.c's dispatch switch.
//
// TROUBLESHOOTING:
//   - 'WHAT?' on valid syntax: check dialect feature flags.
//   - Crash in expression: ensure error_occurred() is checked
//     after every parse_expression call.
 // ---

#include "parser_internal.h"
#include "task.h"
#ifdef _WIN32
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

 // pi_parse_vdev - Handle VDEV command.
void pi_parse_vdev(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex;
    (void)rt;
    (void)line_num;
  // VDEV - List all registered virtual devices.
  //
  // Shows slot ID, name, class, capability flags,
  // version, and description for every device in the
  // VDev table (CON:, ERR:, FILE:, N:, FUJI:, etc.).
 vdev_list_all();
 return;
}

 // pi_parse_vmem - Handle VMEM command.
void pi_parse_vmem(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex;
    (void)line_num;
 {
  // VMEM - Virtual memory status.
  //
  // Displays memory map preset, pool sizes, variable
  // storage, string pool, stack usage, and program
  // storage utilization.
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

void pi_parse_bank(Lexer *lex, RuntimeState *rt, int line_num)
{
    // Get current token word and length dynamically
    const char *word = NULL;
    int word_len = 0;
    if (lex->current.type == TOK_NAMED_VAR) {
        word = lex->current.str_start;
        word_len = lex->current.str_length;
    } else if (lex->current.type == TOK_KEYWORD) {
        word = lexer_keyword_name(lex->current.value.keyword);
        if (word != NULL) word_len = (int)strlen(word);
    }

    // BANK LIST
    if (word != NULL && word_len == 4 && strncasecmp(word, "LIST", 4) == 0) {
        lexer_next(lex); // consume LIST
        printf("=== RAMBANK LIST ===\n\n");
        printf(" Bank  Status    Access   State\n");
        printf(" ----  ------    ------   -----\n");
        MemorySystem *main_mem = task_get_main_mem();
        MemorySystem *target_mem = rt->memory;
        if (main_mem == NULL) main_mem = target_mem;
        
        int count = 0;
        for (int i = 1; i < main_mem->num_rambanks; i++) {
            RamBank *b = &main_mem->banks[i];
            char path[260];
            sprintf(path, "bank_swap_%d.tmp", i);
            FILE *f = fopen(path, "rb");
            int has_swap = (f != NULL);
            if (f != NULL) fclose(f);
            
            if (b->base != NULL || b->resident || b->shared || has_swap) {
                const char *status = "FREE";
                if (b->resident) status = "\x1b[32mRESIDENT\x1b[0m";
                else if (has_swap) status = "\x1b[33mSWAPPED\x1b[0m";
                
                const char *access = b->shared ? "\x1b[36mSHARED\x1b[0m" : "PRIVATE";
                const char *state = b->dirty ? "\x1b[31mDIRTY\x1b[0m" : "CLEAN";
                
                printf("  %3d  %-16s %-16s %-16s\n", i, status, access, state);
                count++;
            }
        }
        printf("\nTotal Active RAMBANKs: %d\n", count);
        return;
    }

    // BANK COPY
    if (word != NULL && word_len == 4 && strncasecmp(word, "COPY", 4) == 0) {
        lexer_next(lex); // consume COPY
        int src_bank = (int)parse_expression(lex, rt, line_num);
        if (error_occurred()) return;
        if (!lexer_expect(lex, TOK_COMMA)) return;
        long src_offset = parse_expression(lex, rt, line_num);
        if (error_occurred()) return;
        if (!lexer_expect(lex, TOK_COMMA)) return;
        
        // Optionally allow "TO" keyword
        if (lex->current.type == TOK_NAMED_VAR && 
            lex->current.str_length == 2 && 
            strncasecmp(lex->current.str_start, "TO", 2) == 0) {
            lexer_next(lex); // consume TO
        } else if (lex->current.type == TOK_KEYWORD && lex->current.value.keyword == KW_TO) {
            lexer_next(lex); // consume TO keyword
        }
        int dst_bank = (int)parse_expression(lex, rt, line_num);
        if (error_occurred()) return;
        if (!lexer_expect(lex, TOK_COMMA)) return;
        long dst_offset = parse_expression(lex, rt, line_num);
        if (error_occurred()) return;
        if (!lexer_expect(lex, TOK_COMMA)) return;
        long length = parse_expression(lex, rt, line_num);
        if (error_occurred()) return;
        
        rambank_copy(rt->memory, src_bank, src_offset, dst_bank, dst_offset, length, line_num);
        return;
    }

    // BANK FILL
    if (word != NULL && word_len == 4 && strncasecmp(word, "FILL", 4) == 0) {
        lexer_next(lex); // consume FILL
        int bank_id = (int)parse_expression(lex, rt, line_num);
        if (error_occurred()) return;
        if (!lexer_expect(lex, TOK_COMMA)) return;
        long offset = parse_expression(lex, rt, line_num);
        if (error_occurred()) return;
        if (!lexer_expect(lex, TOK_COMMA)) return;
        long length = parse_expression(lex, rt, line_num);
        if (error_occurred()) return;
        if (!lexer_expect(lex, TOK_COMMA)) return;
        int value = (int)parse_expression(lex, rt, line_num);
        if (error_occurred()) return;
        
        rambank_fill(rt->memory, bank_id, offset, length, (unsigned char)value, line_num);
        return;
    }

    // BANK bank_id
    int bank_id = (int)parse_expression(lex, rt, line_num);
    if (error_occurred()) return;

    if (bank_id <= 0 || bank_id >= rt->memory->num_rambanks) {
        error_raise(ERR_HOW, line_num);
        return;
    }

    const char *mod = NULL;
    int mod_len = 0;
    if (lex->current.type == TOK_NAMED_VAR) {
        mod = lex->current.str_start;
        mod_len = lex->current.str_length;
    } else if (lex->current.type == TOK_KEYWORD) {
        mod = lexer_keyword_name(lex->current.value.keyword);
        if (mod != NULL) mod_len = (int)strlen(mod);
    }

    if (mod != NULL) {
        if (mod_len == 6 && strncasecmp(mod, "SHARED", 6) == 0) {
            lexer_next(lex); // consume SHARED
            MemorySystem *main_mem = task_get_main_mem();
            if (main_mem != NULL) {
                main_mem->banks[bank_id].shared = 1;
            }
            rt->memory->banks[bank_id].shared = 1;
            return;
        }
        if (mod_len == 7 && strncasecmp(mod, "PRIVATE", 7) == 0) {
            lexer_next(lex); // consume PRIVATE
            MemorySystem *main_mem = task_get_main_mem();
            if (main_mem != NULL) {
                main_mem->banks[bank_id].shared = 0;
            }
            rt->memory->banks[bank_id].shared = 0;
            return;
        }
        if (mod_len == 5 && strncasecmp(mod, "CLEAR", 5) == 0) {
            lexer_next(lex); // consume CLEAR
            MemorySystem *main_mem = task_get_main_mem();
            int is_shared = (main_mem != NULL && main_mem->banks[bank_id].shared);
            MemorySystem *target_mem = rt->memory;
            if (is_shared && main_mem != NULL) {
                target_mem = main_mem;
            }
            if (is_shared) {
                task_mutex_lock();
            }
            rambank_ensure_resident(target_mem, bank_id);
            RamBank *b = &target_mem->banks[bank_id];
            if (b->base != NULL) {
                memset(b->base, 0, RAMBANK_SIZE);
                b->dirty = 1;
            }
            if (is_shared) {
                task_mutex_unlock();
            }
            return;
        }
        if (mod_len == 6 && strncasecmp(mod, "STATUS", 6) == 0) {
            lexer_next(lex); // consume STATUS
            MemorySystem *main_mem = task_get_main_mem();
            if (main_mem == NULL) main_mem = rt->memory;
            RamBank *b = &main_mem->banks[bank_id];
            
            char path[260];
            sprintf(path, "bank_swap_%d.tmp", bank_id);
            FILE *f = fopen(path, "rb");
            int has_swap = (f != NULL);
            if (f != NULL) fclose(f);
            
            printf("Bank ID:      %d\n", bank_id);
            printf("Status:       %s\n", b->resident ? "RESIDENT" : (has_swap ? "SWAPPED" : "FREE"));
            printf("Sharing:      %s\n", b->shared ? "SHARED" : "PRIVATE");
            printf("State:        %s\n", b->dirty ? "DIRTY" : "CLEAN");
            printf("Last Access:  %ld\n", b->last_access);
            return;
        }
    }

    // Default BANK n: Switch current task's active_bank_id
    BasicTask *curr = task_get_current();
    if (curr != NULL) {
        curr->active_bank_id = bank_id;
    }
}

 // pi_parse_vnet - Handle VNET command.
void pi_parse_vnet(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex;
    (void)rt;
    (void)line_num;
 {
  // VNET - Virtual network status.
  //
  // Shows FujiNet N: channel table: which channels
  // are open, protocol, host, port, connection state,
  // bytes waiting, and error status.
  // If the FUJINET module is not active, says so.
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
     // Access via vdev_info on the N: device
     VDev *nd = vdev_get(
         vdev_find_by_name("N:"));
     if (nd == NULL) break;

     // Check channel via a probe - channel state
      // is internal to mod_fujinet. We report what
      // vdev_info exposes. For now show the device
      // info as summary. 
     (void)i;
     break;
 }

 // Channel summary from N: device info
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

 // Adapter summary
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

 // pi_parse_vcon - Handle VCON command.
void pi_parse_vcon(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex;
    (void)rt;
    (void)line_num;
 {
  // VCON - Virtual console information.
  //
  // Shows the console device (CON:) status: device
  // class, capabilities, and the error output (ERR:).
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

 // pi_parse_vterm - Handle VTERM command.
void pi_parse_vterm(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex;
    (void)line_num;
 {
  // VTERM - Virtual terminal information.
  //
  // Shows terminal type, screen dimensions (when
  // available), character encoding, and dialect-
  // specific terminal behavior.
 printf("=== VIRTUAL TERMINAL ===\n\n");
 printf(" Dialect: %s\n",
     "BASIC++");
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

 // pi_parse_vmach - Handle VMACH command.
void pi_parse_vmach(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)lex;
    (void)line_num;
 {
  // VMACH - Virtual machine state.
  //
  // Shows VM execution state, dispatch table stats,
  // stack frame info, opcode count, and module/
  // security status.
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

 // Module summary
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
     "BASIC++");
 printf(" Trace: %s\n",
     rt->trace_on ? "ON (TRON)" : "OFF (TROFF)");
 return;
 }
}

 // pi_parse_devmap - Handle DEVMAP command.
void pi_parse_devmap(Lexer *lex, RuntimeState *rt, int line_num)
{
    (void)rt;
 {
   // DEVMAP - Device slot mapping.
   //
   // Shows which VDev slot each BASIC file channel
   // (#1-#8) is mapped to, including open/closed
   // status and the device name in each slot. Also
   // lists all occupied VDev slots and active device
   // aliases.
   //
   // Subcommands:
   //   DEVMAP         - show full device map + aliases
   //   DEVMAP ALIAS   - show only device aliases
   //   DEVMAP ALIAS "E:" "CON:" - create manual alias
   //   DEVMAP ALIAS RESET - clear all aliases
   //   DEVMAP ALIAS DIALECT - reload dialect defaults

  // Check for ALIAS subcommand
  if (lex->current.type == TOK_KEYWORD &&
      lex->current.value.keyword == KW_ALIAS) {
   lexer_next(lex);

   // DEVMAP ALIAS RESET
   if (lex->current.type == TOK_KEYWORD &&
       lex->current.value.keyword == KW_RESET) {
    lexer_next(lex);
    device_alias_clear_all();
    printf("Device aliases cleared.\n");
    return;
   }

   

   // DEVMAP ALIAS "src" "tgt" - manual alias creation
   if (lex->current.type == TOK_STRING) {
    char alias_name[16];
    char target_name[16];
    int al, tl;

    al = lex->current.str_length;
    if (al > 15) al = 15;
    memcpy(alias_name, lex->current.str_start,
           (size_t)al);
    alias_name[al] = '\0';
    lexer_next(lex);

    if (lex->current.type != TOK_STRING) {
     error_raise(ERR_WHAT, line_num);
     return;
    }
    tl = lex->current.str_length;
    if (tl > 15) tl = 15;
    memcpy(target_name,
           lex->current.str_start, (size_t)tl);
    target_name[tl] = '\0';
    lexer_next(lex);

    if (device_alias_register(alias_name,
        target_name, DEVALIAS_BOTH, -1) == 0) {
     printf("Alias %s -> %s created.\n",
            alias_name, target_name);
    } else {
     printf("Alias table full.\n");
     error_raise(ERR_HOW, line_num);
    }
    return;
   }

   // DEVMAP ALIAS (no args) - list aliases only
   printf("=== DEVICE ALIASES ===\n\n");
   device_alias_list();
   return;
  }

  // Full DEVMAP display
  {
  int i;
  int total = 0;

  printf("=== DEVICE MAP ===\n\n");

  printf(" BASIC File Channels:\n");
  printf(" Channel Device "
      "Status\n");
  printf(" ------- ------ "
      "------\n");
  for (i = 1; i <= 8; i++) {
      // File channels are tracked by the FILE: VDev
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

  // Show active aliases if any
  {
  int ac = device_alias_count();
  if (ac > 0) {
   printf("\n Device Aliases (%d active):\n", ac);
   device_alias_list();
  } else {
   printf("\n No device aliases active.\n");
  }
  }
  return;
  }

  // ===== Final polish =====
 }
}
