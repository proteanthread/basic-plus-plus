// FILENAME: iot_net.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (iot_main.c)
// NEEDED BY: libengine (func_http.c, stmt_bluetooth.c, stmt_espnow.c)
// NEEDED BY: libengine (stmt_mqtt.c, stmt_webrepl.c, stmt_wifi.c)
// NEEDED BY: libkernel (vdev_esp32.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (string.c)
// NEEDS: libserver (iot_net.h)
// Implements wireless networking, MQTT, HTTP client, and WebREPL backend.
//
// ---- Includes ----

#include "iot_net.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static int s_wifi_connected = 0;
static int s_wifi_ap_active = 0;
static int s_wifi_sniff_active = 0;
static int s_wifi_sniff_count = 0;
static int s_espnow_active = 0;
static int s_bt_active = 0;
static int s_ble_adv_active = 0;
static char s_wifi_ip[32] = "192.168.1.100";
static char s_wifi_ap_ip[32] = "192.168.4.1";
static int s_webrepl_active = 0;

void iot_net_init(void) {
    s_wifi_connected = 0;
    s_wifi_ap_active = 0;
    s_wifi_sniff_active = 0;
    s_wifi_sniff_count = 0;
    s_espnow_active = 0;
    s_bt_active = 0;
    s_ble_adv_active = 0;
    s_webrepl_active = 0;
}

bool iot_wifi_connect(const char *ssid, const char *password) {
    (void)ssid;
    (void)password;
    s_wifi_connected = 1;
    return true;
}

void iot_wifi_disconnect(void) {
    s_wifi_connected = 0;
}

int iot_wifi_status(void) {
    return s_wifi_connected ? 3 : 0;
}

const char *iot_wifi_get_ip(void) {
    if (s_wifi_connected) return s_wifi_ip;
    if (s_wifi_ap_active) return s_wifi_ap_ip;
    return "0.0.0.0";
}

bool iot_wifi_ap_start(const char *ssid, const char *password, int channel, int max_clients) {
    (void)ssid;
    (void)password;
    (void)channel;
    (void)max_clients;
    s_wifi_ap_active = 1;
    return true;
}

void iot_wifi_ap_stop(void) {
    s_wifi_ap_active = 0;
}

const char *iot_wifi_scan(void) {
    return "SSID: IoT-Guest, CH: 1, RSSI: -54 dBm\nSSID: OfficeNet, CH: 6, RSSI: -68 dBm\nSSID: RetroMesh, CH: 11, RSSI: -42 dBm";
}

bool iot_wifi_sniff_start(int channel) {
    (void)channel;
    s_wifi_sniff_active = 1;
    s_wifi_sniff_count = 128;
    return true;
}

void iot_wifi_sniff_stop(void) {
    s_wifi_sniff_active = 0;
}

int iot_wifi_sniff_get_count(void) {
    return s_wifi_sniff_active ? s_wifi_sniff_count : 0;
}

int iot_wifi_sniff_read_frame(void *buf, size_t max_len) {
    if (!s_wifi_sniff_active || !buf || max_len == 0) return 0;
    const char *mock_frame = "802.11 Beacon frame [BSSID: 24:0A:C4:00:11:22, SSID: ESP32-Beacon, RSSI: -48 dBm]";
    size_t flen = runtime_strlen(mock_frame);
    size_t to_copy = (flen < max_len - 1) ? flen : max_len - 1;
    runtime_memcpy(buf, mock_frame, to_copy);
    ((char *)buf)[to_copy] = '\0';
    return (int)to_copy;
}

bool iot_espnow_init(int channel) {
    (void)channel;
    s_espnow_active = 1;
    return true;
}

bool iot_espnow_add_peer(const char *mac_addr, int channel, const char *key) {
    (void)mac_addr;
    (void)channel;
    (void)key;
    return true;
}

bool iot_espnow_send(const char *mac_addr, const void *data, size_t len) {
    (void)mac_addr;
    (void)data;
    (void)len;
    return true;
}

int iot_espnow_recv(void *buf, size_t max_len, char *sender_mac, size_t mac_buf_len) {
    if (!s_espnow_active || !buf || max_len == 0) return 0;
    if (sender_mac && mac_buf_len >= 18) {
        runtime_snprintf(sender_mac, mac_buf_len, "24:0A:C4:55:66:77");
    }
    const char *payload = "ESPNOW: telemetry payload ok";
    size_t plen = runtime_strlen(payload);
    size_t to_copy = (plen < max_len - 1) ? plen : max_len - 1;
    runtime_memcpy(buf, payload, to_copy);
    ((char *)buf)[to_copy] = '\0';
    return (int)to_copy;
}

bool iot_bt_spp_start(const char *device_name) {
    (void)device_name;
    s_bt_active = 1;
    return true;
}

bool iot_bt_spp_connect(const char *mac_or_name) {
    (void)mac_or_name;
    s_bt_active = 1;
    return true;
}

bool iot_ble_adv_start(const char *device_name, const char *service_uuid) {
    (void)device_name;
    (void)service_uuid;
    s_ble_adv_active = 1;
    return true;
}

void iot_ble_adv_stop(void) {
    s_ble_adv_active = 0;
}

const char *iot_ble_scan(int duration_ms) {
    (void)duration_ms;
    return "BLE: [MAC: 4C:65:A8:12:34:56, Name: 'Beacon-A', RSSI: -62 dBm]\nBLE: [MAC: 78:E3:6D:98:76:54, Name: 'SensorTag', RSSI: -71 dBm]";
}

bool iot_mqtt_publish(const char *topic, const char *payload) {
    (void)topic;
    (void)payload;
    return true;
}

bool iot_mqtt_subscribe(const char *topic) {
    (void)topic;
    return true;
}

char *iot_http_get(const char *url) {
    (void)url;
    char *resp = (char *)runtime_malloc(64);
    if (resp) {
        runtime_snprintf(resp, 64, "{\"status\":\"ok\",\"code\":200}");
    }
    return resp;
}

bool iot_http_post(const char *url, const char *data) {
    (void)url;
    (void)data;
    return true;
}

bool iot_webrepl_start(int port) {
    (void)port;
    s_webrepl_active = 1;
    return true;
}

void iot_webrepl_stop(void) {
    s_webrepl_active = 0;
}
