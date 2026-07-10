#include "vm_snapshot.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VMS_MAGIC 0x534D5650 // "PVMS" (Plus Virtual Machine Snapshot)

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t mem_size;
    MockBiosModel model;
    MockBiosContext bios;
} VMS_Header;
#pragma pack(pop)

int vm_snapshot_save(VirtualMachine* vm, const char* filename, int format) {
    if (!vm || !vm->mem_ctx) return -1;

    if (format == 0) { // Binary VMS format
        FILE* f = fopen(filename, "wb");
        if (!f) return -1;

        VMS_Header hdr;
        memset(&hdr, 0, sizeof(hdr));
        hdr.magic = VMS_MAGIC;
        hdr.version = 1;
        // In order to not depend on internals of GW_Memory if possible, we should add a size getter.
        // Assuming we have gw_mem_get_size(vm->mem_ctx). But since we might not, let's look it up or add it.
        // We will just read the buffer and size using a helper we add to segmented_mem.c
        extern size_t gw_mem_get_size(GW_Memory *mem);
        hdr.mem_size = (uint32_t)gw_mem_get_size(vm->mem_ctx);
        hdr.model = vm->model;
        hdr.bios = vm->bios_ctx;

        fwrite(&hdr, sizeof(VMS_Header), 1, f);
        
        uint8_t* buf = gw_mem_get_buffer(vm->mem_ctx);
        if (buf && hdr.mem_size > 0) {
            fwrite(buf, 1, hdr.mem_size, f);
        }

        fclose(f);
        return 0;
    }
    
    // JSON / RAW format (format == 1) left as an exercise for the reader or implemented minimally.
    return -1;
}

int vm_snapshot_load(VirtualMachine* vm, const char* filename) {
    if (!vm || !vm->mem_ctx) return -1;

    FILE* f = fopen(filename, "rb");
    if (!f) return -1;

    VMS_Header hdr;
    if (fread(&hdr, sizeof(VMS_Header), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    if (hdr.magic != VMS_MAGIC) {
        fclose(f);
        return -1; // Invalid format
    }

    vm->model = hdr.model;
    vm->bios_ctx = hdr.bios;

    extern size_t gw_mem_get_size(GW_Memory *mem);
    size_t current_size = gw_mem_get_size(vm->mem_ctx);

    if (hdr.mem_size <= current_size) {
        uint8_t* buf = gw_mem_get_buffer(vm->mem_ctx);
        if (buf) {
            fread(buf, 1, hdr.mem_size, f);
        }
    } else {
        // If snapshot memory is larger, we would ideally resize or fail.
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}
