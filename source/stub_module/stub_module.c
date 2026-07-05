#include <stdio.h>

#ifdef _WIN32
__declspec(dllexport) void bpp_module_init(void) {
    printf("[STUB_MODULE] Stub library initialized.\n");
}
#else
void bpp_module_init(void) {
    printf("[STUB_MODULE] Stub library initialized.\n");
}
#endif
