// FILENAME: fujinet.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDS: libcore, libkernel, libplatform, libserver
// Implements virtual device and graphics rendering logic for fujinet.
//
// ---- Includes ----

#include "device/fujinet.h"
#include "runtime/vnet.h"
#include "runtime/gemini.h"
#include "runtime/gopher.h"
#include "runtime/tnfs.h"
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "runtime/conv/num_parse.h"
#include "platform/platform.h"

// FujiNet Configuration
static struct {
    char ssid[64];
    char slots[8][256];
} g_fuji_config;

typedef struct {
    char buf[128];
    int  len;
    int  pos;
} ClockPriv;

// Initialize and save/load config
static void fuji_config_load(void) {
    HalContext *hal = hal_get();
    runtime_memset(&g_fuji_config, 0, sizeof(g_fuji_config));
    runtime_snprintf(g_fuji_config.ssid, sizeof(g_fuji_config.ssid), "FujiNet-WiFi");
    runtime_snprintf(g_fuji_config.slots[0], sizeof(g_fuji_config.slots[0]), "."); // default slot 1 is current dir

    if (!hal || !hal->io.file_open) return;
    IoHandle h = hal->io.file_open("fujinet.cfg", "r");
    if (h == IO_HANDLE_INVALID) return;

    char file_buf[2048];
    size_t bytes_read = hal->io.file_read ? hal->io.file_read(h, file_buf, 1, sizeof(file_buf) - 1) : 0;
    if (hal->io.file_close) hal->io.file_close(h);
    file_buf[bytes_read] = '\0';

    const char *p = file_buf;
    while (*p) {
        char line[256];
        size_t lidx = 0;
        while (*p && *p != '\n' && *p != '\r' && lidx < sizeof(line) - 1) {
            line[lidx++] = *p++;
        }
        line[lidx] = '\0';
        while (*p == '\n' || *p == '\r') p++;

        char *eq = runtime_strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = line;
        char *val = eq + 1;

        if (runtime_strcmp(key, "SSID") == 0) {
            runtime_strncpy(g_fuji_config.ssid, val, sizeof(g_fuji_config.ssid) - 1);
            g_fuji_config.ssid[sizeof(g_fuji_config.ssid) - 1] = '\0';
        } else if (runtime_strncmp(key, "SLOT", 4) == 0) {
            char *endptr = NULL;
            int slot_idx = (int)runtime_strtol(key + 4, &endptr, 10) - 1;
            if (endptr != key + 4 && slot_idx >= 0 && slot_idx < 8) {
                runtime_strncpy(g_fuji_config.slots[slot_idx], val, sizeof(g_fuji_config.slots[slot_idx]) - 1);
                g_fuji_config.slots[slot_idx][sizeof(g_fuji_config.slots[slot_idx]) - 1] = '\0';
            }
        }
    }
}

static void fuji_config_save(void) {
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_open || !hal->io.file_write) return;
    IoHandle h = hal->io.file_open("fujinet.cfg", "w");
    if (h == IO_HANDLE_INVALID) return;

    char out_buf[512];
    int len = runtime_snprintf(out_buf, sizeof(out_buf), "SSID=%s\n", g_fuji_config.ssid);
    if (len > 0) hal->io.file_write(h, out_buf, 1, (size_t)len);

    for (int i = 0; i < 8; i++) {
        len = runtime_snprintf(out_buf, sizeof(out_buf), "SLOT%d=%s\n", i + 1, g_fuji_config.slots[i]);
        if (len > 0) hal->io.file_write(h, out_buf, 1, (size_t)len);
    }
    if (hal->io.file_close) hal->io.file_close(h);
}

void fujinet_init_system(VMContext *vm) {
    (void)vm;
    fuji_config_load();
}

void fujinet_shutdown_system(void) {
    fuji_config_save();
}

// ================================================================
// N: Network Device Driver Implementation
// ================================================================
typedef struct {
    char *gemini_data;
    int   gemini_len;
    int   gemini_pos;
    bool  is_gemini;
    int   vnet_channel;
} NPriv;


static int n_putc(VDev *dev, int c) {
    NPriv *p = (NPriv *)dev->priv;
    if (!p) return -1;
    if (p->is_gemini) return -1; // read-only
    char ch = (char)c;
    BppError err = vnet_send(NULL, p->vnet_channel, &ch, 1);
    return (err.code == 0) ? c : -1;
}

static int n_puts(VDev *dev, const char *s) {
    NPriv *p = (NPriv *)dev->priv;
    if (!p || !s) return -1;
    if (p->is_gemini) return -1;
    BppError err = vnet_send(NULL, p->vnet_channel, s, runtime_strlen(s));
    return (err.code == 0) ? (int)runtime_strlen(s) : -1;
}

static int n_getc(VDev *dev) {
    NPriv *p = (NPriv *)dev->priv;
    if (!p) return -1;
    if (p->is_gemini) {
        if (p->gemini_pos < p->gemini_len) {
            return (unsigned char)p->gemini_data[p->gemini_pos++];
        }
        return -1; // EOF
    }
    char ch;
    size_t out_len = 0;
    BppError err = vnet_recv(NULL, p->vnet_channel, &ch, 1, &out_len);
    return (err.code == 0 && out_len > 0) ? (unsigned char)ch : -1;
}

static char *n_gets(VDev *dev, char *buf, size_t size) {
    if (!buf || size == 0) return NULL;
    NPriv *p = (NPriv *)dev->priv;
    if (!p) return NULL;

    size_t idx = 0;
    while (idx < size - 1) {
        int c = n_getc(dev);
        if (c == -1) break;
        buf[idx++] = (char)c;
        if (c == '\n') break;
    }
    buf[idx] = '\0';
    return (idx > 0) ? buf : NULL;
}

static int n_read(VDev *dev, void *buf, int len) {
    NPriv *p = (NPriv *)dev->priv;
    if (!p || !buf || len <= 0) return -1;
    if (p->is_gemini) {
        int rem = p->gemini_len - p->gemini_pos;
        if (rem <= 0) return 0; // EOF
        int copy_len = (len < rem) ? len : rem;
        runtime_memcpy(buf, p->gemini_data + p->gemini_pos, copy_len);
        p->gemini_pos += copy_len;
        return copy_len;
    }
    size_t out_len = 0;
    BppError err = vnet_recv(NULL, p->vnet_channel, (char *)buf, (size_t)len, &out_len);
    return (err.code == 0) ? (int)out_len : -1;
}

static int n_write(VDev *dev, const void *buf, int len) {
    NPriv *p = (NPriv *)dev->priv;
    if (!p || !buf || len <= 0) return -1;
    if (p->is_gemini) return -1;
    BppError err = vnet_send(NULL, p->vnet_channel, (const char *)buf, (size_t)len);
    return (err.code == 0) ? len : -1;
}

static int n_open(VDev *d, const char *path, int mode) {
    HalContext *hal = hal_get();
    NPriv *p = (NPriv *)(hal && hal->mem.alloc ? hal->mem.alloc(sizeof(NPriv)) : NULL);
    if (!p) return -1;
    runtime_memset(p, 0, sizeof(NPriv));
    d->priv = p;

    if (runtime_strncasecmp(path, "gemini://", 9) == 0) {
        BppError fetch_err;
        char *resp = net_gemini_fetch(NULL, path, &fetch_err);
        if (resp) {
            p->is_gemini = true;
            p->gemini_data = resp;
            p->gemini_len = (int)runtime_strlen(resp);
            p->gemini_pos = 0;
            return 0;
        }
        if (hal && hal->mem.free) hal->mem.free(p);
        d->priv = NULL;
        return -1;
    }

    if (runtime_strncasecmp(path, "gopher://", 9) == 0) {
        BppError fetch_err;
        char *resp = net_gopher_fetch(NULL, path, &fetch_err);
        if (resp) {
            p->is_gemini = true;
            p->gemini_data = resp;
            p->gemini_len = (int)runtime_strlen(resp);
            p->gemini_pos = 0;
            return 0;
        }
        if (hal && hal->mem.free) hal->mem.free(p);
        d->priv = NULL;
        return -1;
    }

    if (runtime_strncasecmp(path, "tnfs://", 7) == 0) {
        size_t out_sz = 0;
        char *resp = tnfs_read_file(path, &out_sz);
        if (resp) {
            p->is_gemini = true;
            p->gemini_data = resp;
            p->gemini_len = (int)out_sz;
            p->gemini_pos = 0;
            return 0;
        }
        if (hal && hal->mem.free) hal->mem.free(p);
        d->priv = NULL;
        return -1;
    }

    bool is_http = (runtime_strncmp(path, "http://", 7) == 0);
    bool is_https = (runtime_strncmp(path, "https://", 8) == 0);
    bool is_ftp = (runtime_strncmp(path, "ftp://", 6) == 0);

    if (is_http || is_https || is_ftp) {
        p->is_gemini = false;
        char host[256] = "";
        char resource[512] = "/";
        int port = is_https ? 443 : (is_ftp ? 21 : 80);

        const char *url_body = path + (is_http ? 7 : (is_https ? 8 : 6));
        const char *slash = runtime_strchr(url_body, '/');
        size_t hlen = slash ? (size_t)(slash - url_body) : runtime_strlen(url_body);
        if (hlen > 255) hlen = 255;
        runtime_memcpy(host, url_body, hlen);
        host[hlen] = '\0';

        if (slash) {
            runtime_strncpy(resource, slash, sizeof(resource) - 1);
            resource[sizeof(resource) - 1] = '\0';
        }

        char *colon = runtime_strchr(host, ':');
        if (colon) {
            *colon = '\0';
            char *endptr = NULL;
            port = (int)runtime_strtol(colon + 1, &endptr, 10);
            if (endptr == colon + 1) {
                port = 80;
            }
        }

        p->vnet_channel = 15;
        BppError err = vnet_open(NULL, p->vnet_channel, "TCP", host, port);
        if (err.code != 0) {
            if (hal && hal->mem.free) hal->mem.free(p);
            d->priv = NULL;
            return -1;
        }

        if (is_http || is_https) {
            char request[1024];
            runtime_snprintf(request, sizeof(request),
                     "%s %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "Connection: close\r\n"
                     "User-Agent: BASIC++ FujiNet\r\n"
                     "\r\n",
                     (mode == 1) ? "GET" : "POST",
                     resource, host);
            vnet_send(NULL, p->vnet_channel, request, runtime_strlen(request));

            // Skip headers for input mode
            if (mode == 1) {
                char header_buf[4096];
                size_t h_idx = 0;
                char last_chars[4] = "";
                while (h_idx < sizeof(header_buf) - 1) {
                    char ch;
                    size_t out_len = 0;
                    BppError r_err;
                    do {
                        r_err = vnet_recv(NULL, p->vnet_channel, &ch, 1, &out_len);
                    } while (r_err.code == 0 && out_len == 0 && vnet_connected(NULL, p->vnet_channel));

                    if (r_err.code != 0 || out_len == 0) {
                        break;
                    }
                    header_buf[h_idx++] = ch;
                    last_chars[0] = last_chars[1];
                    last_chars[1] = last_chars[2];
                    last_chars[2] = last_chars[3];
                    last_chars[3] = ch;
                    if (runtime_strcmp(last_chars, "\r\n\r\n") == 0) {
                        break;
                    }
                }
            }
        }

        if (is_ftp) {
            char greeting[1024];
            size_t out_len = 0;
            vnet_recv(NULL, p->vnet_channel, greeting, sizeof(greeting) - 1, &out_len);

            vnet_send(NULL, p->vnet_channel, "USER anonymous\r\n", 16);
            vnet_recv(NULL, p->vnet_channel, greeting, sizeof(greeting) - 1, &out_len);

            vnet_send(NULL, p->vnet_channel, "PASS anonymous@basicpp.org\r\n", 28);
            vnet_recv(NULL, p->vnet_channel, greeting, sizeof(greeting) - 1, &out_len);
        }

        return 0;
    }

    // TCP/UDP protocol connection
    p->is_gemini = false;
    char proto[16] = "TCP";
    char host[256] = "";
    int port = 80;

    const char *colon1 = runtime_strchr(path, ':');
    if (colon1) {
        int plen = (int)(colon1 - path);
        if (plen < 15) {
            runtime_memcpy(proto, path, plen);
            proto[plen] = '\0';
        }
        const char *host_part = colon1 + 1;
        const char *colon2 = runtime_strchr(host_part, ':');
        if (colon2) {
            int hlen = (int)(colon2 - host_part);
            if (hlen < 255) {
                runtime_memcpy(host, host_part, hlen);
                host[hlen] = '\0';
            }
            char *endptr = NULL;
            port = (int)runtime_strtol(colon2 + 1, &endptr, 10);
            if (endptr == colon2 + 1) {
                port = 80;
            }
        } else {
            runtime_strncpy(host, host_part, sizeof(host) - 1);
            host[sizeof(host) - 1] = '\0';
        }
    }

    // Use channel 15 for N: network virtual device
    p->vnet_channel = 15;
    BppError err = vnet_open(NULL, p->vnet_channel, proto, host, port);
    if (err.code != 0) {
        if (hal && hal->mem.free) hal->mem.free(p);
        d->priv = NULL;
        return -1;
    }
    return 0;
}

static int n_close(VDev *d) {
    HalContext *hal = hal_get();
    NPriv *p = (NPriv *)d->priv;
    if (p) {
        if (p->is_gemini) {
            if (p->gemini_data && hal && hal->mem.free) hal->mem.free(p->gemini_data);
        } else {
            vnet_close(NULL, p->vnet_channel);
        }
        if (hal && hal->mem.free) hal->mem.free(p);
        d->priv = NULL;
    }
    return 0;
}

static int n_status(VDev *d) {
    NPriv *p = (NPriv *)d->priv;
    if (!p) return 0;
    if (p->is_gemini) {
        return (p->gemini_pos < p->gemini_len) ? 1 : 0;
    }
    return vnet_connected(NULL, p->vnet_channel) ? 1 : 0;
}

VDev fujinet_create_n_dev(VMContext *vm) {
    (void)vm;
    VDev dev;
    runtime_memset(&dev, 0, sizeof(VDev));
    dev.name = "N:";
    dev.dev_class = VDCLASS_NETWORK;
    dev.dev_caps = VDCAP_RW | VDCAP_BINARY | VDCAP_STREAM;
    dev.dev_version = "1.0";
    dev.dev_description = "FujiNet Network Adapter Interface";

    dev.ops.putc = n_putc;
    dev.ops.puts = n_puts;
    dev.ops.getc = n_getc;
    dev.ops.gets = n_gets;
    
    dev.dev_read = n_read;
    dev.dev_write = n_write;
    dev.dev_open = n_open;
    dev.dev_close = n_close;
    dev.dev_status = n_status;

    return dev;
}

// ================================================================
// FUJI: wifi & Configuration virtual device implementation
// ================================================================
static int fuji_ioctl(VDev *d, int cmd, void *arg) {
    (void)d;
    if (!arg) return -1;
    // Custom IOCTL codes for WiFi/Slot setup
    if (cmd == 1) { // Set Wifi SSID
        runtime_strncpy(g_fuji_config.ssid, (const char *)arg, sizeof(g_fuji_config.ssid) - 1);
        g_fuji_config.ssid[sizeof(g_fuji_config.ssid) - 1] = '\0';
        fuji_config_save();
        return 0;
    }
    if (cmd == 2) { // Mount slot (arg = slot_num,slot_path)
        const char *comma = runtime_strchr((const char *)arg, ',');
        if (comma) {
            char *endptr = NULL;
            int slot = (int)runtime_strtol((const char *)arg, &endptr, 10) - 1;
            if (endptr != (const char *)arg && slot >= 0 && slot < 8) {
                runtime_strncpy(g_fuji_config.slots[slot], comma + 1, sizeof(g_fuji_config.slots[slot]) - 1);
                g_fuji_config.slots[slot][sizeof(g_fuji_config.slots[slot]) - 1] = '\0';
                fuji_config_save();
                return 0;
            }
        }
    }
    return -1;
}

static const char *fuji_info(VDev *d, const char *key) {
    (void)d;
    if (!key) return NULL;
    if (runtime_strcmp(key, "SSID") == 0) return g_fuji_config.ssid;
    if (runtime_strncmp(key, "SLOT", 4) == 0) {
        char *endptr = NULL;
        int idx = (int)runtime_strtol(key + 4, &endptr, 10) - 1;
        if (endptr != key + 4 && idx >= 0 && idx < 8) return g_fuji_config.slots[idx];
    }
    return NULL;
}

static int fuji_open(VDev *d, const char *path, int mode) {
    (void)d; (void)path; (void)mode;
    return 0;
}

static int fuji_close(VDev *d) {
    (void)d;
    return 0;
}

VDev fujinet_create_fuji_dev(VMContext *vm) {
    (void)vm;
    VDev dev;
    runtime_memset(&dev, 0, sizeof(VDev));
    dev.name = "FUJI:";
    dev.dev_class = VDCLASS_GPIO;
    dev.dev_caps = VDCAP_CONTROL | VDCAP_STATUS | VDCAP_RW;
    dev.dev_version = "1.0";
    dev.dev_description = "FujiNet Configuration Interface";

    dev.dev_ioctl = fuji_ioctl;
    dev.dev_info = fuji_info;
    dev.dev_open = fuji_open;
    dev.dev_close = fuji_close;

    return dev;
}

// ================================================================
// CLOCK: NTP Clock virtual device implementation
// ================================================================
static int clock_open(VDev *d, const char *path, int mode) {
    (void)path; (void)mode;
    HalContext *hal = hal_get();
    ClockPriv *p = (ClockPriv *)(hal && hal->mem.alloc ? hal->mem.alloc(sizeof(ClockPriv)) : NULL);
    if (!p) return -1;
    runtime_memset(p, 0, sizeof(ClockPriv));
    d->priv = p;

    time_t t = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : (int64_t)platform_get_timer());
    struct tm tm_buf;
    struct tm *lt = platform_localtime(&t, &tm_buf);
    if (lt) {
        runtime_snprintf(p->buf, sizeof(p->buf), "%04d-%02d-%02d %02d:%02d:%02d\n",
                 lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
                 lt->tm_hour, lt->tm_min, lt->tm_sec);
        p->len = (int)runtime_strlen(p->buf);
    }

    p->pos = 0;
    return 0;
}

static int clock_close(VDev *d) {
    HalContext *hal = hal_get();
    if (d->priv) {
        if (hal && hal->mem.free) hal->mem.free(d->priv);
        d->priv = NULL;
    }
    return 0;
}

static int clock_getc(VDev *dev) {
    ClockPriv *p = (ClockPriv *)dev->priv;
    if (!p) return -1;
    if (p->pos < p->len) {
        return (unsigned char)p->buf[p->pos++];
    }
    return -1;
}

static char *clock_gets(VDev *dev, char *buf, size_t size) {
    if (!buf || size == 0) return NULL;
    ClockPriv *p = (ClockPriv *)dev->priv;
    if (!p) return NULL;
    size_t idx = 0;
    while (idx < size - 1) {
        int c = clock_getc(dev);
        if (c == -1) break;
        buf[idx++] = (char)c;
        if (c == '\n') break;
    }
    buf[idx] = '\0';
    return (idx > 0) ? buf : NULL;
}

static int clock_read(VDev *d, void *buf, int len) {
    ClockPriv *p = (ClockPriv *)d->priv;
    if (!p || !buf || len <= 0) return -1;
    int rem = p->len - p->pos;
    if (rem <= 0) return 0;
    int copy_len = (len < rem) ? len : rem;
    runtime_memcpy(buf, p->buf + p->pos, copy_len);
    p->pos += copy_len;
    return copy_len;
}

static const char *clock_info(VDev *d, const char *key) {
    (void)d;
    static char res[64];
    HalContext *hal = hal_get();
    time_t t = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : (int64_t)platform_get_timer());
    struct tm tm_buf;
    struct tm *lt = platform_localtime(&t, &tm_buf);
    if (!lt) return NULL;


    if (runtime_strcmp(key, "TIME") == 0) {
        runtime_snprintf(res, sizeof(res), "%02d:%02d:%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
        return res;
    }
    if (runtime_strcmp(key, "DATE") == 0) {
        runtime_snprintf(res, sizeof(res), "%04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
        return res;
    }
    return NULL;
}

VDev fujinet_create_clock_dev(VMContext *vm) {
    (void)vm;
    VDev dev;
    runtime_memset(&dev, 0, sizeof(VDev));
    dev.name = "CLOCK:";
    dev.dev_class = VDCLASS_TIMER;
    dev.dev_caps = VDCAP_READ | VDCAP_STATUS | VDCAP_RW;
    dev.dev_version = "1.0";
    dev.dev_description = "FujiNet Realtime NTP Clock";

    dev.ops.getc = clock_getc;
    dev.ops.gets = clock_gets;

    dev.dev_read = clock_read;
    dev.dev_info = clock_info;
    dev.dev_open = clock_open;
    dev.dev_close = clock_close;

    return dev;
}

