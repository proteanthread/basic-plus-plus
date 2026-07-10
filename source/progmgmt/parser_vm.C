#include "parser_internal.H"
#include "../core/vm_host.H"
#include "../core/task.H"
#include <string.h>

void pi_parse_vm_cmd(Lexer *lex, RuntimeState *rt, int line_num) {
    // VM CREATE "name", "model", ram_kb
    // VM START "name", "file"
    // VM STOP "name"
    // VM ATTACH "name"
    
    if (lexer_peek(lex)->type == TOK_IDENTIFIER) {
        char subcmd[32];
        strncpy(subcmd, lexer_peek(lex)->value.identifier, sizeof(subcmd) - 1);
        subcmd[31] = '\0';
        lexer_consume(lex);
        
        // VM CREATE "name", "model", ram_kb
        if (strcmp(subcmd, "CREATE") == 0) {
            char name[64] = {0};
            char model_str[32] = {0};
            int ram = 640;
            
            // "name"
            if (lexer_peek(lex)->type != TOK_STRING) {
                rt_error_set(rt, ERR_SYNTAX);
                return;
            }
            strncpy(name, lexer_peek(lex)->value.str_val, sizeof(name) - 1);
            lexer_consume(lex);
            
            if (lexer_match(lex, TOK_COMMA)) {
                if (lexer_peek(lex)->type == TOK_STRING) {
                    strncpy(model_str, lexer_peek(lex)->value.str_val, sizeof(model_str) - 1);
                    lexer_consume(lex);
                } else {
                    rt_error_set(rt, ERR_SYNTAX);
                    return;
                }
            }
            if (lexer_match(lex, TOK_COMMA)) {
                Value ram_val = parse_expression(lex, rt);
                if (rt_error_check(rt)) return;
                ram = value_to_int(ram_val);
            }
            
            MockBiosModel m = BIOS_MODEL_MSDOS;
            if (strcmp(model_str, "XT") == 0) m = BIOS_MODEL_XT;
            else if (strcmp(model_str, "AT") == 0) m = BIOS_MODEL_AT;
            else if (strcmp(model_str, "PCJR") == 0) m = BIOS_MODEL_PCJR;
            else if (strcmp(model_str, "PC") == 0) m = BIOS_MODEL_PC;
            
            int id = vm_host_create(name, m, ram);
            if (id < 0) {
                rt_error_set(rt, ERR_OUT_OF_MEMORY);
            }
            return;
        }
        
        // VM START "name", "file"
        if (strcmp(subcmd, "START") == 0) {
            char name[64] = {0};
            char file[260] = {0};
            
            if (lexer_peek(lex)->type != TOK_STRING) {
                rt_error_set(rt, ERR_SYNTAX);
                return;
            }
            strncpy(name, lexer_peek(lex)->value.str_val, sizeof(name) - 1);
            lexer_consume(lex);
            
            if (lexer_match(lex, TOK_COMMA)) {
                if (lexer_peek(lex)->type == TOK_STRING) {
                    strncpy(file, lexer_peek(lex)->value.str_val, sizeof(file) - 1);
                    lexer_consume(lex);
                } else {
                    rt_error_set(rt, ERR_SYNTAX);
                    return;
                }
            }
            
            int id = vm_host_find_by_name(name);
            if (id < 0) {
                rt_error_set(rt, ERR_FILE_NOT_FOUND);
                return;
            }
            if (vm_host_start(id, file) < 0) {
                rt_error_set(rt, ERR_INTERNAL);
            }
            return;
        }
        
        // VM STOP "name"
        if (strcmp(subcmd, "STOP") == 0) {
            if (lexer_peek(lex)->type != TOK_STRING) {
                rt_error_set(rt, ERR_SYNTAX);
                return;
            }
            char name[64] = {0};
            strncpy(name, lexer_peek(lex)->value.str_val, sizeof(name) - 1);
            lexer_consume(lex);
            
            int id = vm_host_find_by_name(name);
            if (id >= 0) {
                vm_host_stop(id);
            }
            return;
        }
        
        // VM ATTACH "name"
        if (strcmp(subcmd, "ATTACH") == 0) {
            if (lexer_peek(lex)->type != TOK_STRING) {
                rt_error_set(rt, ERR_SYNTAX);
                return;
            }
            char name[64] = {0};
            strncpy(name, lexer_peek(lex)->value.str_val, sizeof(name) - 1);
            lexer_consume(lex);
            
            int id = vm_host_find_by_name(name);
            if (id >= 0) {
                vm_host_attach(id);
            }
            return;
        }
        

        // VM SNAPSHOT "file"
        if (strcmp(subcmd, "SNAPSHOT") == 0) {
            char file[260] = {0};
            if (lexer_peek(lex)->type != TOK_STRING) { rt_error_set(rt, ERR_SYNTAX); return; }
            strncpy(file, lexer_peek(lex)->value.str_val, sizeof(file) - 1);
            lexer_consume(lex);
            
            VirtualMachine* vm = vm_host_current();
            if (vm) {
                // extern int vm_snapshot_save(VirtualMachine* vm, const char* filename, int format);
                // vm_snapshot_save(vm, file, 0); // format 0 = VMS binary
            }
            return;
        }

        // VM RESTORE "file"
        if (strcmp(subcmd, "RESTORE") == 0) {
            char file[260] = {0};
            if (lexer_peek(lex)->type != TOK_STRING) { rt_error_set(rt, ERR_SYNTAX); return; }
            strncpy(file, lexer_peek(lex)->value.str_val, sizeof(file) - 1);
            lexer_consume(lex);
            
            VirtualMachine* vm = vm_host_current();
            if (vm) {
                // extern int vm_snapshot_load(VirtualMachine* vm, const char* filename);
                // vm_snapshot_load(vm, file);
            }
            return;
        }

        // VM SPEED <pct>
        if (strcmp(subcmd, "SPEED") == 0) {
            Value pct_val = parse_expression(lex, rt);
            if (rt_error_check(rt)) return;
            int pct = value_to_int(pct_val);
            
            VirtualMachine* vm = vm_host_current();
            if (vm) {
                vm_host_set_speed(vm->vm_id, pct);
            }
            return;
        }

        // VM INTERRUPT <num>
        if (strcmp(subcmd, "INTERRUPT") == 0) {
            Value int_val = parse_expression(lex, rt);
            if (rt_error_check(rt)) return;
            int int_num = value_to_int(int_val);
            
            VirtualMachine* vm = vm_host_current();
            if (vm) {
                vm_host_inject_interrupt(vm->vm_id, (uint8_t)int_num);
            }
            return;
        }

        // VM SHARE <bank>
        if (strcmp(subcmd, "SHARE") == 0) {
            Value bank_val = parse_expression(lex, rt);
            if (rt_error_check(rt)) return;
            int bank_id = value_to_int(bank_val);
            
            VirtualMachine* vm = vm_host_current();
            if (vm) {
                extern void segmem_share_bank(void* ctx, int bank_id, int mode);
                segmem_share_bank(vm->mem_ctx, bank_id, 1); // 1 = SHARED_LOCKED
            }
            return;
        }

        rt_error_set(rt, ERR_SYNTAX);
    } else {
        rt_error_set(rt, ERR_SYNTAX);
    }
}
