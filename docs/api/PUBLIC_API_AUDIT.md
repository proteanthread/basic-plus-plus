# BASIC++ Public API Audit

> **Status**: AUTOMATED AUDIT — Completed
> This document catalogs all public functions, structs, and enums exposed by the 38 public header files in `include/` for third-party embedding.

## Subsystem Layer 1 — Embedding & Lifecycle

### `boot.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `boot_shutdown` | `void` | `BootContext *ctx` |

---

### `config.h`

---

### `basic_version.h`

---

### `vm.h`
#### Structs:
- `VMContext VMContext`
- `VDevContext VDevContext`
- `FileContext FileContext`
- `StmtRegistry StmtRegistry`
- `VfsContext VfsContext`
- `VNetContext VNetContext`
- `UsbContext UsbContext`
- `VConContext VConContext`
- `MockBiosContext MockBiosContext`
- `VMemContext VMemContext`
- `TryStack TryStack`
- `GosubStack GosubStack`
- `ForStack ForStack`
- `WhileStack WhileStack`
- `DoStack DoStack`
- `SelectStack SelectStack`
- `SubStack SubStack`
- `SelectStack SelectStack`
- `SubStack SubStack`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vm_shutdown` | `void` | `VMContext *vm` |
| `vm_run_program` | `void` | `VMContext *vm` |
| `vm_is_running` | `bool` | `VMContext *vm` |
| `vm_get_eval_depth` | `int` | `VMContext *vm` |
| `vm_inc_eval_depth` | `void` | `VMContext *vm` |
| `vm_dec_eval_depth` | `void` | `VMContext *vm` |
| `vm_reset_error_state` | `void` | `VMContext *vm` |
| `vm_clear_aliases` | `void` | `VMContext *vm` |
| `vm_reset_for_run` | `void` | `VMContext *vm` |
| `vm_register_alias` | `void` | `VMContext *vm, const char *name, const char *expansion` |
| `vm_load_program_file` | `BppError` | `VMContext *vm, const char *filename` |
| `vm_bload_program_from_stream` | `BppError` | `VMContext *vm, FILE *fp` |
| `vm_stop` | `void` | `VMContext *vm` |
| `vm_execute_line` | `BppError` | `VMContext *vm, const char *source` |
| `execute_single_statement` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `vm_get_bios_registers` | `void` | `VMContext *vm, uint32_t *ax, uint32_t *bx, uint32_t *cx, uint32_t *dx, uint32_t *flags` |
| `vm_set_bios_registers` | `void` | `VMContext *vm, uint32_t ax, uint32_t bx, uint32_t cx, uint32_t dx, uint32_t flags` |
| `vm_get_current_line` | `BppLineNumber` | `VMContext *vm` |
| `vm_build_data_table` | `void` | `VMContext *vm` |
| `vm_get_data_ptr` | `int` | `VMContext *vm` |
| `vm_set_data_ptr` | `void` | `VMContext *vm, int ptr` |
| `vm_get_data_count` | `int` | `VMContext *vm` |
| `vm_get_err_code` | `int` | `VMContext *vm` |
| `vm_get_err_line` | `BppLineNumber` | `VMContext *vm` |
| `vm_set_error_trap` | `void` | `VMContext *vm, BppLineNumber line` |
| `vm_get_error_trap` | `BppLineNumber` | `VMContext *vm` |
| `vm_is_in_error_handler` | `bool` | `VMContext *vm` |
| `vm_set_in_error_handler` | `void` | `VMContext *vm, bool in_handler` |
| `vm_get_error_occurred_line` | `BppLineNumber` | `VMContext *vm` |
| `vm_trigger_error_trap` | `void` | `VMContext *vm, int code, BppLineNumber line, const char *pos, const char *next_pos` |
| `vm_trigger_breakpoint` | `void` | `VMContext *vm, const char *reason` |
| `vm_get_single_step` | `bool` | `VMContext *vm` |
| `vm_set_single_step` | `void` | `VMContext *vm, bool enable` |
| `vm_set_timer_trap` | `void` | `VMContext *vm, double seconds, BppLineNumber line` |
| `vm_set_timer_state` | `void` | `VMContext *vm, int state` |
| `vm_set_key_trap` | `void` | `VMContext *vm, int key_idx, int key_code, BppLineNumber line` |
| `vm_set_key_state` | `void` | `VMContext *vm, int key_idx, int state` |
| `vm_set_play_trap` | `void` | `VMContext *vm, int note_threshold, BppLineNumber line` |
| `vm_set_play_state` | `void` | `VMContext *vm, int state` |
| `vm_trigger_event_polling` | `void` | `VMContext *vm` |
| `vm_clear_event_handlers` | `void` | `VMContext *vm` |
| `try_stack_shutdown` | `void` | `TryStack *stack` |
| `try_stack_clear` | `void` | `TryStack *stack` |
| `try_stack_push` | `bool` | `TryStack *stack, BppTryFrame frame` |
| `try_stack_pop` | `bool` | `TryStack *stack, BppTryFrame *out_frame` |
| `try_stack_peek` | `bool` | `TryStack *stack, BppTryFrame *out_frame` |
| `try_stack_count` | `size_t` | `TryStack *stack` |
| `vm_restore_stack_depths` | `void` | `VMContext *vm, BppTryFrame frame` |
| `gosub_stack_depth` | `size_t` | `GosubStack *stack` |
| `for_stack_depth` | `size_t` | `ForStack *stack` |
| `while_stack_depth` | `size_t` | `WhileStack *stack` |
| `do_stack_depth` | `size_t` | `DoStack *stack` |
| `select_stack_depth` | `size_t` | `SelectStack *stack` |
| `sub_stack_depth` | `size_t` | `SubStack *stack` |
| `vm_set_error` | `void` | `VMContext *vm, int code, const char *msg` |
| `vm_clear_error` | `void` | `VMContext *vm` |
| `vm_has_error` | `bool` | `VMContext *vm` |
| `vm_set_opt_eh` | `void` | `VMContext *vm, bool enable` |
| `vm_get_opt_eh` | `bool` | `VMContext *vm` |
| `vm_set_current_line` | `void` | `VMContext *vm, BppLineNumber line` |
| `vm_set_arithmetic_decimal` | `void` | `VMContext *vm, bool enable` |
| `vm_get_arithmetic_decimal` | `bool` | `VMContext *vm` |
| `vm_get_last_rnd` | `double` | `VMContext *vm` |
| `vm_set_last_rnd` | `void` | `VMContext *vm, double val` |
| `vm_save_state` | `void` | `VMContext *vm, BppVMState *state` |
| `vm_restore_state` | `void` | `VMContext *vm, const BppVMState *state` |
| `vm_get_error` | `BppError` | `VMContext *vm` |
| `vm_jump` | `void` | `VMContext *vm, BppLineNumber line, const char *pos` |
| `vm_gosub_push` | `bool` | `VMContext *vm, BppLineNumber line, const char *pos` |
| `vm_gosub_pop` | `bool` | `VMContext *vm, BppLineNumber *out_line, const char **out_pos` |
| `vm_for_push` | `bool` | `VMContext *vm, const char *var_name, double target, double step, BppLineNumber line, const char *pos` |
| `vm_for_update` | `bool` | `VMContext *vm, const char *var_name, double target, double step, const char *next_range_pos` |
| `vm_for_pop` | `bool` | `VMContext *vm, const char *var_name, BppForFrame *out_frame` |
| `vm_for_peek` | `bool` | `VMContext *vm, const char *var_name, BppForFrame *out_frame` |
| `vm_while_push` | `bool` | `VMContext *vm, BppLineNumber line, const char *pos` |
| `vm_while_pop` | `bool` | `VMContext *vm, BppLineNumber *out_line, const char **out_pos` |
| `vm_while_peek` | `bool` | `VMContext *vm, BppLineNumber *out_line, const char **out_pos` |
| `vm_do_push` | `bool` | `VMContext *vm, BppLineNumber line, const char *pos` |
| `vm_do_pop` | `bool` | `VMContext *vm, BppLineNumber *out_line, const char **out_pos` |
| `vm_do_peek` | `bool` | `VMContext *vm, BppLineNumber *out_line, const char **out_pos` |
| `select_stack_shutdown` | `void` | `SelectStack *stack` |
| `select_stack_clear` | `void` | `SelectStack *stack` |
| `select_stack_push` | `bool` | `SelectStack *stack, BValue val, bool matched, BppLineNumber line, const char *pos` |
| `select_stack_pop` | `bool` | `SelectStack *stack, BppSelectFrame *out_frame` |
| `select_stack_peek` | `bool` | `SelectStack *stack, BppSelectFrame *out_frame` |
| `vm_select_push` | `bool` | `VMContext *vm, BValue val, bool matched, BppLineNumber line, const char *pos` |
| `vm_select_pop` | `bool` | `VMContext *vm, BppSelectFrame *out_frame` |
| `vm_select_peek` | `bool` | `VMContext *vm, BppSelectFrame *out_frame` |
| `sub_stack_shutdown` | `void` | `SubStack *stack` |
| `sub_stack_clear` | `void` | `SubStack *stack` |
| `sub_stack_push` | `bool` | `SubStack *stack, const char *name, BppLineNumber line, const char *pos, bool is_func` |
| `sub_stack_pop` | `bool` | `SubStack *stack, BppSubFrame *out_frame` |
| `sub_stack_peek` | `bool` | `SubStack *stack, BppSubFrame *out_frame` |
| `vm_sub_push` | `bool` | `VMContext *vm, const char *name, BppLineNumber line, const char *pos, bool is_func` |
| `vm_sub_pop` | `bool` | `VMContext *vm, BppSubFrame *out_frame` |
| `vm_sub_peek` | `bool` | `VMContext *vm, BppSubFrame *out_frame` |
| `vm_set_current_filename` | `void` | `VMContext *vm, const char *filename` |
| `vm_set_start_line` | `void` | `VMContext *vm, BppLineNumber line` |
| `vm_get_start_line` | `BppLineNumber` | `VMContext *vm` |
| `vm_is_jump_active` | `bool` | `VMContext *vm` |
| `vm_handle_jump_active` | `bool` | `VMContext *vm, BppLineNumber *out_line` |
| `vm_clear_header_jump` | `void` | `VMContext *vm, BppLineNumber def_line` |
| `vm_with_stack_push` | `void` | `VMContext *vm, const char *path` |
| `vm_with_stack_pop` | `void` | `VMContext *vm` |
| `vm_with_stack_clear` | `void` | `VMContext *vm` |
| `vm_halt` | `void` | `VMContext *vm` |
| `vm_request_exit` | `void` | `VMContext *vm` |
| `vm_exit_requested` | `bool` | `VMContext *vm` |
| `bytecode_set_detokenizer` | `void` | `DetokenizerFn fn` |
| `bytecode_get_detokenizer` | `DetokenizerFn` | `void` |

---

## Subsystem Layer 2 — Language Core

### `arrays.h`
#### Structs:
- `ArrayContext ArrayContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `arr_shutdown` | `void` | `ArrayContext *ctx` |
| `arr_clear_all` | `void` | `ArrayContext *ctx` |
| `arr_dim` | `BppError` | `ArrayContext *ctx, const char *name, int num_dims, const int *bounds` |
| `arr_set_type` | `void` | `ArrayContext *ctx, const char *name, ValueType type` |
| `arr_erase` | `bool` | `ArrayContext *ctx, const char *name` |
| `arr_exists` | `bool` | `ArrayContext *ctx, const char *name` |
| `arr_ubound` | `int` | `ArrayContext *ctx, const char *name, int dimension, bool *out_found` |
| `arr_set_option_base` | `void` | `ArrayContext *ctx, int base` |
| `arr_get_option_base` | `int` | `ArrayContext *ctx` |
| `arr_get_dimensions` | `int` | `ArrayContext *ctx, const char *name, int *out_bounds, int max_dims` |
| `arr_get_last_det` | `double` | `ArrayContext *ctx` |
| `arr_set_last_det` | `void` | `ArrayContext *ctx, double val` |
| `arr_serialize` | `bool` | `ArrayContext *ctx, void *fp` |
| `arr_deserialize` | `bool` | `ArrayContext *ctx, void *fp` |

---

### `eval.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `eval_expression` | `BValue` | `VMContext *vm, LexerContext *lex, BppError *err` |
| `invoke_user_function` | `BValue` | `VMContext *vm, const char *name, BValue *args, int argc, BppError *err` |

---

### `lexer.h`
#### Structs:
- `LexerContext LexerContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `lex_shutdown` | `void` | `LexerContext *ctx` |
| `lex_next` | `BppToken` | `LexerContext *ctx` |
| `lex_peek` | `BppToken` | `LexerContext *ctx` |
| `lex_set_pos` | `void` | `LexerContext *ctx, const char *pos` |
| `lex_find_keyword_by_name` | `BppKeywordId` | `const char *name` |
| `keyword_clear_custom` | `void` | `void` |
| `keyword_register_custom` | `BppKeywordId` | `const char *name` |

---

### `stmt.h`
#### Structs:
- `StmtRegistry StmtRegistry`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `stmt_defseg_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `stmt_registry_shutdown` | `void` | `StmtRegistry *reg` |
| `stmt_register` | `void` | `StmtRegistry *reg, BppKeywordId kw, BppStmtHandler handler, const char *name, uint32_t flags` |
| `stmt_lookup` | `BppStmtHandler` | `StmtRegistry *reg, BppKeywordId kw` |
| `vm_call_sub_procedure` | `BppError` | `VMContext *vm, const char *sub_name, BValue *args, int arg_count, const char *ret_pos` |
| `find_procedure` | `bool` | `VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text` |
| `stmt_assert_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `stmt_tron_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `stmt_troff_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `stmt_break_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `stmt_vars_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |

---

### `types.h`
#### Structs:
- `BppString BppString`
- `BppMap BppMap`
- `BppTypeRegistry BppTypeRegistry`

---

### `variables.h`
#### Structs:
- `VariableContext VariableContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `var_shutdown` | `void` | `VariableContext *ctx` |
| `var_assign` | `bool` | `VariableContext *ctx, const char *name, BValue val` |
| `var_clear_all` | `void` | `VariableContext *ctx` |
| `var_clear_scope` | `void` | `VariableContext *ctx, const char *prefix` |
| `var_set_scope` | `void` | `VariableContext *ctx, const char *scope` |
| `var_set_shared` | `void` | `VariableContext *ctx, const char *name` |
| `var_set_explicit` | `void` | `VariableContext *ctx, bool enable` |
| `var_is_explicit` | `bool` | `VariableContext *ctx` |
| `var_set_namespace` | `void` | `VariableContext *ctx, const char *ns` |
| `var_set_case_sensitive` | `void` | `VariableContext *ctx, bool enable` |
| `var_set_def_type` | `void` | `VariableContext *ctx, const char *scope, char start_letter, char end_letter, ValueType type` |
| `var_get_def_type` | `ValueType` | `VariableContext *ctx, const char *scope, char letter` |
| `var_print_all` | `void` | `VariableContext *ctx, void *vdev_ptr` |
| `var_serialize` | `bool` | `VariableContext *ctx, void *fp` |
| `var_deserialize` | `bool` | `VariableContext *ctx, void *fp` |

---

### `strings.h`
#### Structs:
- `StringContext StringContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `str_init` | `StringContext *` | `MemoryContext *mem` |
| `str_shutdown` | `void` | `StringContext *ctx` |
| `str_create` | `BppStringRef` | `StringContext *ctx, const char *data, size_t length` |
| `str_concat` | `BppStringRef` | `StringContext *ctx, BppStringRef a, BppStringRef b` |
| `str_mid` | `BppStringRef` | `StringContext *ctx, BppStringRef ref, size_t start, size_t len` |
| `str_data` | `const char *` | `BppStringRef ref` |
| `str_len` | `size_t` | `BppStringRef ref` |
| `str_add_ref` | `void` | `BppStringRef ref` |
| `str_release` | `void` | `StringContext *ctx, BppStringRef ref` |
| `str_gc` | `void` | `StringContext *ctx` |

---


## Subsystem Layer 3 — Runtime Extensions

### `file.h`
#### Structs:
- `FileContext FileContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `file_shutdown` | `void` | `FileContext *ctx` |
| `file_open` | `BppError` | `FileContext *ctx, VDevContext *vdev_ctx, int channel, const char *filename, BppFileMode mode, BppFileAccess access, BppFileLockMode lock_mode, int record_len` |
| `file_lock_range` | `BppError` | `FileContext *ctx, int channel, long start, long end` |
| `file_unlock_range` | `BppError` | `FileContext *ctx, int channel, long start, long end` |
| `file_check_overlap` | `BppError` | `FileContext *ctx, int channel, long start, long end` |
| `file_close` | `void` | `FileContext *ctx, int channel` |
| `file_close_all` | `void` | `FileContext *ctx` |
| `file_is_open` | `bool` | `FileContext *ctx, int channel` |
| `file_get_mode` | `BppFileMode` | `FileContext *ctx, int channel` |
| `file_lof` | `long` | `FileContext *ctx, int channel` |
| `file_loc` | `long` | `FileContext *ctx, int channel` |
| `file_eof` | `bool` | `FileContext *ctx, int channel` |
| `file_seek` | `void` | `FileContext *ctx, int channel, long position` |
| `file_get_record_len` | `int` | `FileContext *ctx, int channel` |
| `file_getc` | `int` | `FileContext *ctx, int channel` |
| `file_putc` | `int` | `FileContext *ctx, int channel, int c` |
| `file_ungetc` | `int` | `FileContext *ctx, int channel, int c` |
| `file_puts` | `int` | `FileContext *ctx, int channel, const char *s` |
| `file_printf` | `int` | `FileContext *ctx, int channel, const char *fmt, ...` |
| `file_flush` | `int` | `FileContext *ctx, int channel` |
| `file_read` | `int` | `FileContext *ctx, int channel, void *buf, int len` |
| `file_write` | `int` | `FileContext *ctx, int channel, const void *buf, int len` |
| `file_txn_status` | `int` | `FileContext *ctx` |
| `file_txn_begin` | `void` | `FileContext *ctx, int mode, bool use_file` |
| `file_txn_commit` | `BppError` | `FileContext *ctx` |
| `file_txn_rollback` | `BppError` | `FileContext *ctx` |
| `file_txn_entry_count` | `int` | `FileContext *ctx` |
| `file_txn_log_write` | `void` | `FileContext *ctx, int channel, long position, const void *old_data, int len` |

---

### `funcreg.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `funcreg_init` | `void` | `void` |
| `funcreg_set_registering_module` | `void` | `const char *name` |
| `funcreg_register` | `int` | `const FunctionEntry *entry` |
| `funcreg_override` | `int` | `BppKeywordId kw, FuncHandler handler` |
| `funcreg_count` | `int` | `void` |

---

### `runtime/map.h`
#### Structs:
- `BppMapEntry`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `map_add_ref` | `void` | `BppMap *map` |
| `map_release` | `void` | `void *str_ctx, BppMap *map` |
| `map_set` | `bool` | `void *str_ctx, BppMap *map, const char *key, BValue val` |
| `map_get` | `bool` | `BppMap *map, const char *key, BValue *out_val` |
| `map_remove` | `bool` | `void *str_ctx, BppMap *map, const char *key` |
| `map_count` | `int` | `BppMap *map` |
| `map_has` | `bool` | `BppMap *map, const char *key` |

---

### `metadata.h`
#### Structs:
- `VMContext VMContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `metadata_init` | `void` | `BppMetadataRegistry *reg` |
| `metadata_register_label` | `bool` | `BppMetadataRegistry *reg, const char *label, const char *filename, BppLineNumber line` |
| `metadata_resolve_label` | `bool` | `const BppMetadataRegistry *reg, const char *label, char *out_filename, size_t max_len, BppLineNumber *out_line` |
| `metadata_register_docstring` | `bool` | `BppMetadataRegistry *reg, const char *target, const char *docstring` |
| `metadata_register_block` | `bool` | `BppMetadataRegistry *reg, const char *type, const char *target, const char *docstring, const char *body` |
| `metadata_pre_scan_line` | `void` | `VMContext *vm, const char *filename, BppLineNumber line_num, const char *text` |
| `metadata_pre_scan_program` | `void` | `VMContext *vm, const char *filename` |

---

### `spec.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `spec_registry_init` | `void` | `void` |
| `spec_load_file` | `int` | `VMContext *vm, const char *filename` |
| `spec_load_companion_libraries` | `int` | `VMContext *vm, const char *dir_part` |
| `spec_register_inline` | `int` | `VMContext *vm, const char *name, SpecCategory cat, const char *lib_path, const char *req_level` |
| `spec_get_count` | `int` | `void` |

---

### `struct_ctx.h`
#### Structs:
- `BppTypeRegistry`
- `VMContext VMContext`
- `BppMap BppMap`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `struct_registry_init` | `void` | `BppTypeRegistry *reg` |
| `struct_register_type` | `bool` | `BppTypeRegistry *reg, const BppUserTypeDef *def, char *err_buf, size_t err_len` |
| `struct_copy_instance` | `bool` | `VMContext *vm, BppMap *dst, BppMap *src, char *err_buf, size_t err_len` |

---

### `task.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `task_mutex_lock` | `void` | `void` |
| `task_mutex_unlock` | `void` | `void` |
| `task_mgr_init` | `void` | `void *main_vm` |
| `task_mgr_shutdown` | `void` | `void` |
| `task_spawn` | `int` | `VDevContext *vdev, const char *filename` |
| `task_spawn_at_label` | `int` | `VDevContext *vdev, const char *filename, const char *label` |
| `task_list` | `void` | `VDevContext *vdev` |
| `task_switch` | `void` | `VDevContext *vdev, int target_pid` |
| `task_scheduler_tick` | `void` | `void` |
| `task_kill` | `void` | `VDevContext *vdev, int pid` |
| `task_join` | `void` | `int pid` |
| `task_get_status` | `int` | `int pid` |

---

## Subsystem Layer 4 — Virtual Devices & I/O

### `bus.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vdev_bus_out` | `void` | `int port, int value` |
| `vdev_bus_in` | `int` | `int port` |
| `vdev_bus_peek` | `uint8_t` | `unsigned long addr, bool *intercepted` |
| `vdev_bus_poke` | `void` | `unsigned long addr, uint8_t value, bool *intercepted` |
| `vdev_bus_reset` | `void` | `void` |
| `vdev_bus_set_model` | `void` | `MockBiosModel model` |
| `vdev_bus_get_model` | `MockBiosModel` | `void` |
| `vdev_bus_set_ram` | `void` | `uint8_t *ram, size_t size` |

---

### `fujinet.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `fujinet_init_system` | `void` | `VMContext *vm` |
| `fujinet_shutdown_system` | `void` | `void` |
| `fujinet_create_n_dev` | `VDev` | `VMContext *vm` |
| `fujinet_create_fuji_dev` | `VDev` | `VMContext *vm` |
| `fujinet_create_clock_dev` | `VDev` | `VMContext *vm` |

---

### `gemini.h`

---

### `usb.h`
#### Structs:
- `UsbContext UsbContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `usb_shutdown` | `void` | `UsbContext *ctx` |
| `usb_connect` | `bool` | `UsbContext *ctx, int port, BppUsbDevType type, int vid, int pid` |
| `usb_disconnect` | `void` | `UsbContext *ctx, int port` |
| `usb_get_port_status` | `bool` | `UsbContext *ctx, int port, BppUsbDevice *out_dev` |
| `usb_get_connected_count` | `int` | `UsbContext *ctx` |

---

### `vcon.h`
#### Structs:
- `VConContext VConContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vcon_shutdown` | `void` | `VConContext *ctx` |
| `vcon_select` | `bool` | `VConContext *ctx, int index` |
| `vcon_get_active_index` | `int` | `VConContext *ctx` |
| `vcon_write_char` | `void` | `VConContext *ctx, int index, int c` |
| `vcon_write_string` | `void` | `VConContext *ctx, int index, const char *s` |
| `vcon_clear` | `void` | `VConContext *ctx, int index` |
| `vcon_get_cursor` | `void` | `VConContext *ctx, int index, int *row, int *col` |
| `vcon_get_char_at` | `int` | `VConContext *ctx, int index, int row, int col` |
| `vcon_get_attr_at` | `int` | `VConContext *ctx, int index, int row, int col` |

---

### `vdev.h`
#### Structs:
- `VDev VDev`
- `VDevContext VDevContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vdev_shutdown` | `void` | `VDevContext *ctx` |
| `vdev_register` | `bool` | `VDevContext *ctx, VDev dev` |
| `vdev_printf` | `int` | `VDevContext *ctx, const char *fmt, ...` |
| `vdev_puts` | `int` | `VDevContext *ctx, const char *s` |
| `vdev_putc` | `int` | `VDevContext *ctx, int c` |
| `vdev_play_beep` | `void` | `VDevContext *ctx` |
| `vdev_music_queue_length` | `int` | `void` |
| `vdev_music_clear` | `void` | `void` |
| `vdev_play_sound_freq` | `void` | `double freq, double duration_seconds` |
| `vdev_gfx_poll_events` | `void` | `void` |
| `vdev_gfx_enable` | `void` | `bool allowed, bool gui_boot` |
| `vdev_gfx_boot_check` | `void` | `struct VMContext *vm` |
| `gfx_get_char_at` | `int` | `int row, int col` |
| `gfx_get_attr_at` | `int` | `int row, int col` |
| `vdev_read` | `int` | `VDev *d, void *buf, int len` |
| `vdev_write` | `int` | `VDev *d, const void *buf, int len` |
| `vdev_seek` | `long` | `VDev *d, long offset, int whence` |
| `vdev_ioctl` | `int` | `VDev *d, int cmd, void *arg` |
| `vdev_status` | `int` | `VDev *d` |
| `vdev_poll` | `int` | `VDev *d` |
| `vdev_count` | `int` | `VDevContext *ctx` |
| `vdev_list_all` | `void` | `VDevContext *ctx` |

---

### `vfs.h`
#### Structs:
- `VfsContext VfsContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vfs_shutdown` | `void` | `VfsContext *ctx` |
| `vfs_mount` | `bool` | `VfsContext *ctx, const char *prefix, const char *target, BppMountType type` |
| `vfs_umount` | `bool` | `VfsContext *ctx, const char *prefix` |
| `vfs_resolve` | `bool` | `VfsContext *ctx, const char *virtual_path, char *resolved_path, size_t max_len` |
| `vfs_list_mounts` | `void` | `VfsContext *ctx, VDevContext *vdev` |

---

### `vnet.h`
#### Structs:
- `VNetContext VNetContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vnet_shutdown` | `void` | `VNetContext *ctx` |
| `vnet_open` | `BppError` | `VNetContext *ctx, int channel, const char *protocol, const char *host, int port` |
| `vnet_open_host` | `BppError` | `VNetContext *ctx, int channel, int port` |
| `vnet_accept` | `BppError` | `VNetContext *ctx, int listen_channel, int client_channel, char *client_ip_buf, int ip_buf_len` |
| `vnet_send` | `BppError` | `VNetContext *ctx, int channel, const char *data, size_t len` |
| `vnet_recv` | `BppError` | `VNetContext *ctx, int channel, char *buf, size_t max_len, size_t *out_len` |
| `vnet_close` | `void` | `VNetContext *ctx, int channel` |
| `vnet_status` | `int` | `VNetContext *ctx, int channel` |
| `vnet_connected` | `bool` | `VNetContext *ctx, int channel` |
| `vnet_http_status` | `int` | `VNetContext *ctx, int channel` |
| `vnet_create_vdev` | `VDev` | `VNetContext *ctx, const char *name, const char *protocol, const char *host, int port` |

---

## Subsystem Layer 5 — Graphics

### `bgi.h`
#### Structs:
- `BGI_VideoMode`
- `BGI_Context`
#### Enums:
- `BGI_MemLayout`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `BGI_init` | `int` | `BGI_Context *ctx` |
| `BGI_shutdown` | `void` | `BGI_Context *ctx` |
| `BGI_register_mode` | `int` | `BGI_Context *ctx, const BGI_VideoMode *mode` |
| `BGI_set_mode` | `int` | `BGI_Context *ctx, int mode_handle` |
| `BGI_set_mode_by_id` | `int` | `BGI_Context *ctx, uint32_t mode_id` |
| `BGI_get_dimensions` | `void` | `const BGI_Context *ctx, int *w, int *h` |
| `BGI_putpixel` | `void` | `BGI_Context *ctx, int x, int y, int color` |
| `BGI_getpixel` | `int` | `const BGI_Context *ctx, int x, int y` |
| `BGI_setcolor` | `void` | `BGI_Context *ctx, int color` |
| `BGI_setbkcolor` | `void` | `BGI_Context *ctx, int color` |
| `BGI_setfillstyle` | `void` | `BGI_Context *ctx, int style, int color` |
| `BGI_setlinestyle` | `void` | `BGI_Context *ctx, int style, int pattern, int thickness` |
| `BGI_line` | `void` | `BGI_Context *ctx, int x1, int y1, int x2, int y2` |
| `BGI_circle` | `void` | `BGI_Context *ctx, int cx, int cy, int r` |
| `BGI_fillellipse` | `void` | `BGI_Context *ctx, int cx, int cy, int rx, int ry` |
| `BGI_bar` | `void` | `BGI_Context *ctx, int x1, int y1, int x2, int y2` |
| `BGI_rectangle` | `void` | `BGI_Context *ctx, int x1, int y1, int x2, int y2` |
| `BGI_floodfill` | `void` | `BGI_Context *ctx, int seed_x, int seed_y, int border` |
| `BGI_moveto` | `void` | `BGI_Context *ctx, int x, int y` |
| `BGI_lineto` | `void` | `BGI_Context *ctx, int x, int y` |
| `BGI_clearviewport` | `void` | `BGI_Context *ctx` |
| `BGI_cleardevice` | `void` | `BGI_Context *ctx` |
| `BGI_setpalette` | `void` | `BGI_Context *ctx, int index, uint32_t argb` |
| `BGI_getpalette` | `uint32_t` | `const BGI_Context *ctx, int index` |
| `BGI_setallpalette` | `void` | `BGI_Context *ctx, const uint32_t *pal, int count` |
| `BGI_resolve_color` | `uint32_t` | `const BGI_Context *ctx, int color` |
| `BGI_outtextxy` | `void` | `BGI_Context *ctx, int x, int y, const char *text` |
| `BGI_settextstyle` | `void` | `BGI_Context *ctx, int font, int direction, int size` |
| `BGI_textwidth` | `int` | `const BGI_Context *ctx, const char *text` |
| `BGI_textheight` | `int` | `const BGI_Context *ctx, const char *text` |
| `BGI_imagesize` | `size_t` | `int x1, int y1, int x2, int y2` |
| `BGI_putimage` | `void` | `BGI_Context *ctx, int x, int y, const void *buffer, int op` |
| `BGI_synthesize` | `void` | `BGI_Context *ctx` |
| `BGI_register_heritage_modes` | `void` | `BGI_Context *ctx` |

---

### `bios.h`
#### Structs:
- `BiosRegs`
- `BiosContext`
- `BiosDataArea`
- `BiosMemoryMap`
#### Enums:
- `BiosModel`
- `BiosRevision`
- `BiosClockMode`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `bios_create` | `BiosContext*` | `BiosModel model` |
| `bios_destroy` | `void` | `BiosContext* ctx` |
| `bios_init` | `bool` | `BiosContext* ctx` |
| `bios_set_model` | `void` | `BiosContext* ctx, BiosModel model` |
| `bios_get_model` | `BiosModel` | `const BiosContext* ctx` |
| `bios_set_revision` | `void` | `BiosContext* ctx, BiosRevision revision` |
| `bios_get_revision` | `BiosRevision` | `const BiosContext* ctx` |
| `bios_get_part_number` | `const char*` | `const BiosContext* ctx` |
| `bios_set_clock_mode` | `void` | `BiosContext* ctx, BiosClockMode mode` |
| `bios_get_clock_mode` | `BiosClockMode` | `const BiosContext* ctx` |
| `bios_set_clock_freq` | `void` | `BiosContext* ctx, double mhz` |
| `bios_get_clock_freq` | `double` | `const BiosContext* ctx` |
| `bios_peek` | `uint8_t` | `BiosContext* ctx, uint32_t addr` |
| `bios_poke` | `void` | `BiosContext* ctx, uint32_t addr, uint8_t val` |
| `bios_poke_raw` | `void` | `BiosContext* ctx, uint32_t addr, uint8_t val` |
| `bios_inp` | `uint8_t` | `BiosContext* ctx, uint16_t port` |
| `bios_out` | `void` | `BiosContext* ctx, uint16_t port, uint8_t val` |
| `bios_interrupt` | `bool` | `BiosContext* ctx, uint8_t int_num, BiosRegs* regs` |
| `bios_register_interrupt` | `bool` | `BiosContext* ctx, uint8_t int_num, BiosIntHandlerFn handler, void* user_data` |
| `bios_post_code` | `void` | `BiosContext* ctx, uint8_t code` |

---

## Subsystem Layer 6 — Security & Modules

### `module.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `module_system_init` | `void` | `void` |
| `module_count` | `int` | `void` |
| `module_register` | `int` | `const BppModuleInfo *info` |
| `module_activate` | `int` | `const char *name, void *rt` |
| `module_deactivate` | `int` | `const char *name` |
| `module_is_active` | `int` | `const char *name` |
| `module_is_loaded` | `int` | `int index` |
| `module_caps_string` | `void` | `unsigned int caps, char *buf, int buf_len` |
| `module_load_dynamic` | `int` | `struct VMContext *vm, const char *path` |

---

### `security.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `security_init` | `void` | `BppSecLevel level` |
| `security_get_level` | `BppSecLevel` | `void` |
| `security_set_level` | `void` | `BppSecLevel level` |
| `security_find_level_by_name` | `int` | `const char *name` |
| `security_check` | `int` | `BppSecOperation op, int line_num` |
| `security_module_allowed` | `int` | `unsigned int capabilities` |
| `security_check_pinned_level` | `int` | `BppSecLevel required_level` |
| `security_check_mem` | `int` | `unsigned long address, int size` |
| `security_check_port` | `int` | `int port, int line_num` |
| `security_check_path` | `int` | `const char *path, int line_num` |
| `security_check_file_path` | `int` | `const char *path, int line_num` |
| `security_restrict_op` | `int` | `BppSecOperation op` |
| `security_is_op_restricted` | `int` | `BppSecOperation op` |
| `security_restrict_keyword` | `int` | `int kw_id` |
| `security_is_keyword_restricted` | `int` | `int kw_id` |
| `security_restrict_list` | `void` | `void` |
| `security_restrict_count` | `int` | `void` |

---

### `mod_arrayext.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `arrayext_execute_map` | `BppError` | `VMContext *vm, const char *src_arr, const char *dst_arr, const char *fn_name, const char *label_name, const char *expr_str` |
| `arrayext_func_map` | `BValue` | `VMContext *vm, LexerContext *lex, BppError *err` |
| `arrayext_func_filter` | `BValue` | `VMContext *vm, LexerContext *lex, BppError *err` |
| `arrayext_func_reduce` | `BValue` | `VMContext *vm, LexerContext *lex, BppError *err` |
| `arrayext_func_aggregate` | `BValue` | `VMContext *vm, LexerContext *lex, int agg_type, BppError *err` |

---

## Subsystem Layer 7 — Platform & Memory

### `memory.h`
#### Structs:
- `MemoryContext MemoryContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `mem_shutdown` | `void` | `MemoryContext *ctx` |
| `mem_scratch_reset` | `void` | `MemoryContext *ctx` |
| `mem_program_insert` | `bool` | `MemoryContext *ctx, BppLineNumber line, const char *text` |
| `mem_program_delete` | `bool` | `MemoryContext *ctx, BppLineNumber line` |
| `mem_program_clear` | `void` | `MemoryContext *ctx` |
| `mem_lib_program_insert` | `bool` | `MemoryContext *ctx, BppLineNumber line, const char *text` |
| `mem_lib_program_clear` | `void` | `MemoryContext *ctx` |
| `mem_string_free` | `void` | `MemoryContext *ctx, void *ptr` |
| `mem_get_free_ram` | `size_t` | `MemoryContext *ctx` |
| `mem_get_used_ram` | `size_t` | `MemoryContext *ctx` |
| `mem_format_size` | `void` | `size_t bytes, char *buf, size_t buf_size` |

---

### `platform.h`
#### Structs:
- `BppDirSearch BppDirSearch`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `platform_init` | `void` | `void` |
| `platform_shutdown` | `void` | `void` |
| `platform_get_id` | `BppPlatformId` | `void` |
| `platform_sleep_ms` | `void` | `uint32_t ms` |
| `platform_kbhit` | `bool` | `void` |
| `platform_getch` | `int` | `void` |
| `platform_console_height` | `int` | `void` |
| `platform_console_width` | `int` | `void` |
| `platform_chdir` | `int` | `const char *path` |
| `platform_setup_signals` | `void` | `void *vm_ptr` |
| `platform_execute_shell` | `void` | `void` |
| `platform_execute_command` | `void` | `const char *cmd` |
| `platform_mkdir` | `int` | `const char *path` |
| `platform_rmdir` | `int` | `const char *path` |
| `platform_remove` | `int` | `const char *path` |
| `platform_rename` | `int` | `const char *oldpath, const char *newpath` |
| `platform_filesize` | `long` | `const char *path` |
| `platform_filemod` | `int` | `const char *path, char *out_buf, size_t buf_size` |
| `platform_list_files` | `int` | `void *vdev_ptr, const char *pattern` |
| `platform_setenv` | `int` | `const char *name, const char *value` |
| `platform_get_attributes` | `int` | `const char *path` |
| `platform_set_attributes` | `int` | `const char *path, int attr` |
| `platform_lock_file` | `int` | `FILE *fp` |
| `platform_unlock_file` | `int` | `FILE *fp` |
| `platform_find_next_file` | `int` | `BppDirSearch *search, char *out_name, size_t out_size` |
| `platform_find_close` | `void` | `BppDirSearch *search` |
| `platform_get_timer` | `double` | `void` |
| `platform_mutex_init` | `void` | `BppMutex *mutex` |
| `platform_mutex_lock` | `void` | `BppMutex *mutex` |
| `platform_mutex_unlock` | `void` | `BppMutex *mutex` |
| `platform_mutex_destroy` | `void` | `BppMutex *mutex` |
| `platform_thread_join` | `int` | `BppThread *thread` |
| `platform_free_library` | `void` | `void *library_handle` |
| `platform_net_init` | `int` | `void` |
| `platform_net_cleanup` | `void` | `void` |
| `platform_socket_connect` | `BppSocket` | `const char *host, int port, int socktype, BppError *err` |
| `platform_socket_listen` | `BppSocket` | `int port, BppError *err` |
| `platform_socket_accept` | `BppSocket` | `BppSocket listen_sock, char *client_ip_buf, int ip_buf_len, BppError *err` |
| `platform_socket_send` | `int` | `BppSocket sock, const void *buf, int len` |
| `platform_socket_recv` | `int` | `BppSocket sock, void *buf, int len, int *err_code` |
| `platform_socket_close` | `void` | `BppSocket sock` |
| `platform_socket_set_nonblocking` | `int` | `BppSocket sock, int nonblock` |
| `platform_socket_poll_readable` | `int` | `BppSocket sock, int timeout_ms` |
| `platform_get_executable_path` | `int` | `char *buf, size_t size` |
| `platform_regex_match` | `int` | `const char *text, const char *pattern` |
| `platform_tui_init` | `void` | `void` |
| `platform_tui_shutdown` | `void` | `void` |
| `platform_screen_get_char` | `int` | `int row, int col` |
| `platform_screen_get_attr` | `int` | `int row, int col` |
| `platform_cleanup_workspace` | `void` | `bool full_cleanup` |
| `platform_clipboard_set` | `void` | `const char *text` |

---

### `segmented_mem.h`
#### Structs:
- `VMemContext VMemContext`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vmem_shutdown` | `void` | `VMemContext *ctx` |
| `vmem_set_def_seg` | `void` | `VMemContext *ctx, uint16_t seg` |
| `vmem_get_def_seg` | `uint16_t` | `VMemContext *ctx` |
| `vmem_register_handle` | `uint32_t` | `VMemContext *ctx, BValue *val, bool is_string_data` |
| `vmem_peek` | `int` | `VMemContext *ctx, uint16_t address, uint8_t *out_val` |
| `vmem_poke` | `int` | `VMemContext *ctx, uint16_t address, uint8_t val` |

---

## Subsystem Layer 8 — Dialect & Configuration

### `dialect.h`
#### Structs:
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|

---

### `editor.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `tui_multiplexer_init` | `void` | `void` |
| `tui_multiplexer_shutdown` | `void` | `void` |
| `editor_manager_init` | `void` | `VMContext *vm` |
| `editor_manager_run` | `int` | `VMContext *vm, const char *editor_name, const char *filename` |
| `mod_ws_main` | `int` | `VMContext *vm, const char *filename` |
| `mod_vi_main` | `int` | `VMContext *vm, const char *filename` |
| `mod_edit_main` | `int` | `VMContext *vm, const char *filename` |
| `mod_edlin_main` | `int` | `VMContext *vm, const char *filename` |

---

### `logger.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `logger_init` | `bool` | `const char *log_path, const char *out_path` |
| `logger_close` | `void` | `void` |
| `log_info` | `void` | `const char *fmt, ...` |
| `log_warn` | `void` | `const char *fmt, ...` |
| `log_error` | `void` | `const char *fmt, ...` |
| `log_write_out` | `void` | `const char *buf, size_t len` |
| `logger_set_debug` | `void` | `bool debug` |
| `logger_is_debug` | `bool` | `void` |
| `logger_set_dry_run` | `void` | `bool dry_run` |
| `logger_is_dry_run` | `bool` | `void` |
| `logger_set_trace` | `void` | `bool trace` |
| `logger_is_trace` | `bool` | `void` |

---

### `state.h`
#### Functions:
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vm_state_save` | `BppError` | `VMContext *vm, const char *filename` |
| `vm_state_load` | `BppError` | `VMContext *vm, const char *filename` |

---

### `custom_dialect_static.h`

---

