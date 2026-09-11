// FILENAME: iot_net.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (iot_main.c)
// NEEDED BY: libengine (func_http.c, stmt_bluetooth.c, stmt_espnow.c)
// NEEDED BY: libengine (stmt_mqtt.c, stmt_webrepl.c, stmt_wifi.c)
// NEEDED BY: libkernel (vdev_esp32.c)
// NEEDED BY: libserver (iot_net.c)
// NEEDS: platform, memory
// Declares wireless networking, MQTT, HTTP client, and WebREPL server interfaces.
//
// ---- Includes ----

#ifndef IOT_NET_H
#define IOT_NET_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void iot_net_init(void);

// ---- WiFi Management ----
bool iot_wifi_connect(const char *ssid, const char *password);
void iot_wifi_disconnect(void);
int  iot_wifi_status(void);
const char *iot_wifi_get_ip(void);

// ---- WiFi Advanced (AP, Scan, Sniffer) ----
bool iot_wifi_ap_start(const char *ssid, const char *password, int channel, int max_clients);
void iot_wifi_ap_stop(void);
const char *iot_wifi_scan(void);
bool iot_wifi_sniff_start(int channel);
void iot_wifi_sniff_stop(void);
int  iot_wifi_sniff_get_count(void);
int  iot_wifi_sniff_read_frame(void *buf, size_t max_len);

// ---- ESP-NOW Connectionless Protocol ----
bool iot_espnow_init(int channel);
bool iot_espnow_add_peer(const char *mac_addr, int channel, const char *key);
bool iot_espnow_send(const char *mac_addr, const void *data, size_t len);
int  iot_espnow_recv(void *buf, size_t max_len, char *sender_mac, size_t mac_buf_len);

// ---- Bluetooth Classic (SPP) & BLE ----
bool iot_bt_spp_start(const char *device_name);
bool iot_bt_spp_connect(const char *mac_or_name);
bool iot_ble_adv_start(const char *device_name, const char *service_uuid);
void iot_ble_adv_stop(void);
const char *iot_ble_scan(int duration_ms);

// ---- MQTT Client ----
bool iot_mqtt_publish(const char *topic, const char *payload);
bool iot_mqtt_subscribe(const char *topic);

// ---- HTTP Client ----
char *iot_http_get(const char *url);
bool iot_http_post(const char *url, const char *data);

// ---- WebREPL Wireless Server ----
bool iot_webrepl_start(int port);
void iot_webrepl_stop(void);

#endif // IOT_NET_H
