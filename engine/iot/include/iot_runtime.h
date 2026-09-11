// FILENAME: iot_runtime.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (iot_main.c)
// NEEDS: platform, memory
// Declares embedded IoT runtime configuration and fluid device profile data.
//
// ---- Includes ----

#ifndef IOT_RUNTIME_H
#define IOT_RUNTIME_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    const char *device_name;
    uint32_t ram_size_bytes;
    bool is_target_configured;
    bool auto_run_enabled;
} IotProfileConfig;

void iot_runtime_init(IotProfileConfig *cfg);
const char *iot_get_active_device_name(void);
bool iot_is_target_active(void);

#endif // IOT_RUNTIME_H
