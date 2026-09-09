// FILENAME: func_packet.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memops.h, memops.c, packet_sniff.h, packet_sniff.c, string.h)
// NEEDS: libcore (strings.h, strings.c, strops.h, strops.c)
// NEEDS: libengine (func_packet.h, string.c)
// Implements PACKET.* built-in metadata inspection functions.
//
// ---- Includes ----

#include "eval/functions/system/hardware/func_packet.h"
#include "runtime/packet_sniff.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_packet_mac_desc = {
    .name = "PACKET.MAC$", .category = "Hardware & Network", .syntax = "PACKET.MAC$()",
    .description = "Returns source MAC address of last captured raw network packet.",
    .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_packet_rssi_desc = {
    .name = "PACKET.RSSI", .category = "Hardware & Network", .syntax = "PACKET.RSSI()",
    .description = "Returns signal strength (RSSI in dBm) of last captured packet.",
    .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_packet_payload_desc = {
    .name = "PACKET.PAYLOAD$", .category = "Hardware & Network", .syntax = "PACKET.PAYLOAD$()",
    .description = "Returns raw payload bytes of last captured packet.",
    .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_packet_len_desc = {
    .name = "PACKET.LEN", .category = "Hardware & Network", .syntax = "PACKET.LEN()",
    .description = "Returns total byte length of last captured network packet.",
    .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_packet_src_desc = {
    .name = "PACKET.SRC$", .category = "Hardware & Network", .syntax = "PACKET.SRC$()",
    .description = "Returns source IP address string of last captured packet.",
    .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_packet_port_desc = {
    .name = "PACKET.PORT", .category = "Hardware & Network", .syntax = "PACKET.PORT()",
    .description = "Returns source port number of last captured packet.",
    .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_packet_type_desc = {
    .name = "PACKET.TYPE$", .category = "Hardware & Network", .syntax = "PACKET.TYPE$()",
    .description = "Returns protocol type string ('TCP', 'UDP', 'RAW') of last captured packet.",
    .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};

void func_packet_register(void) {
    lang_desc_register(&g_packet_mac_desc);
    lang_desc_register(&g_packet_rssi_desc);
    lang_desc_register(&g_packet_payload_desc);
    lang_desc_register(&g_packet_len_desc);
    lang_desc_register(&g_packet_src_desc);
    lang_desc_register(&g_packet_port_desc);
    lang_desc_register(&g_packet_type_desc);
}

BValue func_packet_mac(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc;
    (void)argv;
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    const SniffPacket *p = packet_sniff_get_last();
    const char *mac = (p && p->src_mac[0]) ? p->src_mac : "00:00:00:00:00:00";
    res.as.string = str_create(vm_get_str(vm), mac, runtime_strlen(mac));
    return res;
}

BValue func_packet_rssi(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)vm;
    (void)argc;
    (void)argv;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    const SniffPacket *p = packet_sniff_get_last();
    res.as.number = p ? (double)p->rssi : 0.0;
    return res;
}

BValue func_packet_payload(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc;
    (void)argv;
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    const SniffPacket *p = packet_sniff_get_last();
    if (p && p->payload_len > 0) {
        res.as.string = str_create(vm_get_str(vm), (const char *)p->payload, p->payload_len);
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }
    return res;
}

BValue func_packet_len(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)vm;
    (void)argc;
    (void)argv;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    const SniffPacket *p = packet_sniff_get_last();
    res.as.number = p ? (double)p->payload_len : 0.0;
    return res;
}

BValue func_packet_src(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc;
    (void)argv;
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    const SniffPacket *p = packet_sniff_get_last();
    const char *src = (p && p->src_ip[0]) ? p->src_ip : "0.0.0.0";
    res.as.string = str_create(vm_get_str(vm), src, runtime_strlen(src));
    return res;
}

BValue func_packet_port(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)vm;
    (void)argc;
    (void)argv;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    const SniffPacket *p = packet_sniff_get_last();
    res.as.number = p ? (double)p->src_port : 0.0;
    return res;
}

BValue func_packet_type(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)vm;
    (void)argc;
    (void)argv;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    const SniffPacket *p = packet_sniff_get_last();
    res.as.number = p ? (double)p->frame_type : 0.0;
    return res;
}
