// FILENAME: bios_cpu8086.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libhardware (bios_cpu8086.h)
// Implements virtual BIOS interrupt and hardware emulation for bios_cpu8086.
//
// ---- Includes ----

#include "bios/bios_cpu8086.h"
#include "runtime/string/memops.h"

void bios_cpu8086_reset(BiosCpu8086Context* cpu) {
    if (!cpu) return;
    runtime_memset(cpu, 0, sizeof(BiosCpu8086Context));
    cpu->cs = 0xF000;
    cpu->ip = 0xFFF0;
    cpu->flags = 0x0002;
}

void bios_cpu8086_int(BiosCpu8086Context* cpu, uint8_t int_num, uint8_t* phys_mem_1mb) {
    if (!cpu || !phys_mem_1mb) return;

    // Read IVT vector address at 0x0000:(int_num * 4)
    uint32_t ivt_addr = (uint32_t)int_num * 4U;
    uint16_t target_ip = (uint16_t)(phys_mem_1mb[ivt_addr] | (phys_mem_1mb[ivt_addr + 1] << 8));
    uint16_t target_cs = (uint16_t)(phys_mem_1mb[ivt_addr + 2] | (phys_mem_1mb[ivt_addr + 3] << 8));

    // Push FLAGS, CS, IP onto stack at SS:SP
    uint32_t stack_base = (uint32_t)cpu->ss << 4U;
    cpu->sp -= 2;
    phys_mem_1mb[stack_base + cpu->sp] = (uint8_t)(cpu->flags & 0xFF);
    phys_mem_1mb[stack_base + cpu->sp + 1] = (uint8_t)(cpu->flags >> 8);

    cpu->sp -= 2;
    phys_mem_1mb[stack_base + cpu->sp] = (uint8_t)(cpu->cs & 0xFF);
    phys_mem_1mb[stack_base + cpu->sp + 1] = (uint8_t)(cpu->cs >> 8);

    cpu->sp -= 2;
    phys_mem_1mb[stack_base + cpu->sp] = (uint8_t)(cpu->ip & 0xFF);
    phys_mem_1mb[stack_base + cpu->sp + 1] = (uint8_t)(cpu->ip >> 8);

    cpu->cs = target_cs;
    cpu->ip = target_ip;
}

bool bios_cpu8086_step(BiosCpu8086Context* cpu, uint8_t* phys_mem_1mb) {
    if (!cpu || !phys_mem_1mb || cpu->halted) return false;

    uint32_t phys_addr = ((uint32_t)cpu->cs << 4U) + cpu->ip;
    if (phys_addr >= 1048576U) {
        cpu->halted = true;
        return false;
    }

    uint8_t opcode = phys_mem_1mb[phys_addr];
    cpu->ip++;

    switch (opcode) {
        case 0x90: // NOP
            break;
        case 0xF4: // HLT
            cpu->halted = true;
            break;
        case 0xCF: // IRET
            if (1) {
                uint32_t stack_base = (uint32_t)cpu->ss << 4U;
                cpu->ip = (uint16_t)(phys_mem_1mb[stack_base + cpu->sp] | (phys_mem_1mb[stack_base + cpu->sp + 1] << 8));
                cpu->sp += 2;
                cpu->cs = (uint16_t)(phys_mem_1mb[stack_base + cpu->sp] | (phys_mem_1mb[stack_base + cpu->sp + 1] << 8));
                cpu->sp += 2;
                cpu->flags = (uint16_t)(phys_mem_1mb[stack_base + cpu->sp] | (phys_mem_1mb[stack_base + cpu->sp + 1] << 8));
                cpu->sp += 2;
            }
            break;
        default:
            // Basic instruction passthrough
            break;
    }

    return true;
}
