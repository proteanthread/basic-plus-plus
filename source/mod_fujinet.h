 // ---
 // BASIC++ Interpreter - mod_fujinet.h
 // ---
 //
 // FujiNet Virtual Device Module.
 //
 // Registers three virtual devices that expose the FujiNet
 // hardware interface through the VDev2 system:
 //
 //   N:      Network adapter (TCP, UDP, HTTP, TNFS, etc.)
 //   FUJI:   Configuration device (WiFi, host slots, AppKey)
 //   CLOCK:  Network time protocol clock
 //
 // On desktop platforms (Windows, Linux, macOS), the N: device
 // operates through native TCP/UDP sockets and an HTTP client.
 // The FUJI: and CLOCK: devices provide platform-appropriate
 // equivalents (system time, local config storage, etc.).
 //
 // On real FujiNet hardware (Atari 8-bit, Apple II, C64, CoCo),
 // these devices map directly to the FujiNet SIO/SmartPort/IEC
 // interface via fujinet-lib function calls.
 //
 // DEVICESPEC FORMAT:
 //   N:PROTO://host:port/path
 //
 // Supported protocols:
 //   TCP    - Raw TCP stream
 //   UDP    - Datagram socket
 //   HTTP   - HTTP/1.1 GET/POST
 //   HTTPS  - HTTP over TLS (desktop only)
 //   TNFS   - Trivial Network File System
 //   FTP    - File Transfer Protocol (command channel)
 //   TELNET - Telnet with IAC negotiation
 //
 // BASIC USAGE:
 //   MODULE "FUJINET"
 //   OPEN #1, "N:TCP://irata.online:8005/", "RW"
 //   FPRINT #1, "HELLO"
 //   FINPUT #1, A$
 //   CLOSE #1
 //
 // ---

#ifndef BASICPP_MOD_FUJINET_H
#define BASICPP_MOD_FUJINET_H

// ---- Maximum counts and buffer sizes ----
#define FN_MAX_CHANNELS     8
#define FN_MAX_DEVICESPEC   256
#define FN_MAX_URL          256
#define FN_MAX_HOST_SLOTS   8
#define FN_MAX_DEVICE_SLOTS 8
#define FN_HOST_SLOT_LEN    32
#define FN_FILE_MAXLEN      36
#define FN_SSID_MAXLEN      33
#define FN_PASSWORD_MAXLEN  64
#define FN_APPKEY_MAXLEN    64
#define FN_JSON_QUERY_MAX   256
#define FN_RECV_BUF_SIZE    1024
#define FN_HTTP_HEADER_MAX  512
#define FN_HTTP_BODY_MAX    4096
#define FN_TNFS_PACKET_MAX  532
#define FN_DIR_ENTRY_MAX    36

// ---- Network channel modes ----
#define FN_MODE_READ        0x04
#define FN_MODE_WRITE       0x08
#define FN_MODE_READWRITE   0x0C
#define FN_MODE_APPEND      0x09

// ---- Translation modes ----
#define FN_TRANS_NONE       0
#define FN_TRANS_CR         1
#define FN_TRANS_LF         2
#define FN_TRANS_CRLF       3
#define FN_TRANS_PETSCII    4

// ---- Protocol identifiers ----
typedef enum FnProto {
    FN_PROTO_UNKNOWN = 0,
    FN_PROTO_TCP,
    FN_PROTO_UDP,
    FN_PROTO_HTTP,
    FN_PROTO_HTTPS,
    FN_PROTO_TNFS,
    FN_PROTO_FTP,
    FN_PROTO_TELNET,
    FN_PROTO_SSH
} FnProto;

// ---- Error codes (match fujinet-lib FN_ERR values) ----
#define FN_ERR_OK               0x00
#define FN_ERR_IO_ERROR         0x01
#define FN_ERR_BAD_CMD          0x02
#define FN_ERR_NO_DEVICE        0x03
#define FN_ERR_TIMEOUT          0x04
#define FN_ERR_BAD_MODE         0x05
#define FN_ERR_OFFLINE          0x06
#define FN_ERR_NOT_IMPL         0x07
#define FN_ERR_NO_CONNECTION    0x08
#define FN_ERR_NOT_OPEN         0x09
#define FN_ERR_ALREADY_OPEN     0x0A
#define FN_ERR_DNS_FAIL         0x0B
#define FN_ERR_CONN_REFUSED     0x0C
#define FN_ERR_CONN_RESET       0x0D
#define FN_ERR_BUF_OVERFLOW     0x0E
#define FN_ERR_CHANNEL_FULL     0x0F
#define FN_ERR_EOF              0x88
#define FN_ERR_JSON_PARSE       0x10
#define FN_ERR_INVALID_URL      0x11

// ---- IOCTL command codes for the N: device ----
 // These map directly to fujinet-lib network operations that
 // go beyond simple read/write. Usage from BASIC:
 //   IOCTL #ch, cmd [, arg$]
#define FNIO_JSON_PARSE         (256 + 0)
#define FNIO_JSON_QUERY         (256 + 1)
#define FNIO_SET_CHANNEL_MODE   (256 + 2)
#define FNIO_SET_TRANSLATION    (256 + 3)
#define FNIO_GET_BYTES_WAITING  (256 + 4)
#define FNIO_GET_CONNECTED      (256 + 5)
#define FNIO_GET_ERROR          (256 + 6)
#define FNIO_HTTP_SET_HEADER    (256 + 7)
#define FNIO_HTTP_GET_HEADER    (256 + 8)
#define FNIO_HTTP_POST          (256 + 9)
#define FNIO_HTTP_PUT           (256 + 10)
#define FNIO_HTTP_DELETE        (256 + 11)
#define FNIO_SET_AUX            (256 + 12)
#define FNIO_PARSE_URL          (256 + 13)

// ---- IOCTL command codes for the FUJI: device ----
 // These map to fujinet-lib fuji device commands: WiFi
 // configuration, host/device slot management, AppKey
 // storage, directory browsing, and hardware queries.
#define FNIO_RESET              (256 + 32)
#define FNIO_SCAN_NETWORKS      (256 + 33)
#define FNIO_GET_SCAN_RESULT    (256 + 34)
#define FNIO_GET_SSID           (256 + 35)
#define FNIO_SET_SSID           (256 + 36)
#define FNIO_GET_WIFI_STATUS    (256 + 37)
#define FNIO_GET_WIFI_ENABLED   (256 + 38)
#define FNIO_MOUNT_HOST         (256 + 39)
#define FNIO_UNMOUNT_HOST       (256 + 40)
#define FNIO_MOUNT_IMAGE        (256 + 41)
#define FNIO_UNMOUNT_IMAGE      (256 + 42)
#define FNIO_READ_HOST_SLOTS    (256 + 43)
#define FNIO_WRITE_HOST_SLOTS   (256 + 44)
#define FNIO_READ_DEVICE_SLOTS  (256 + 45)
#define FNIO_WRITE_DEVICE_SLOTS (256 + 46)
#define FNIO_GET_ADAPTER_CONFIG (256 + 47)
#define FNIO_OPEN_DIRECTORY     (256 + 48)
#define FNIO_READ_DIR_ENTRY     (256 + 49)
#define FNIO_CLOSE_DIRECTORY    (256 + 50)
#define FNIO_SET_DIR_POSITION   (256 + 51)
#define FNIO_GET_DIR_POSITION   (256 + 52)
#define FNIO_WRITE_APPKEY       (256 + 53)
#define FNIO_READ_APPKEY        (256 + 54)
#define FNIO_OPEN_APPKEY        (256 + 55)
#define FNIO_CLOSE_APPKEY       (256 + 56)
#define FNIO_COPY_FILE          (256 + 57)
#define FNIO_NEW_DISK           (256 + 58)
#define FNIO_SET_BOOT_MODE      (256 + 59)
#define FNIO_SET_DEVICE_PATH    (256 + 60)
#define FNIO_GET_DEVICE_PATH    (256 + 61)
#define FNIO_SET_HOST_PREFIX    (256 + 62)
#define FNIO_GET_HOST_PREFIX    (256 + 63)
#define FNIO_BASE64_ENCODE      (256 + 64)
#define FNIO_BASE64_DECODE      (256 + 65)
#define FNIO_HASH_COMPUTE       (256 + 66)
#define FNIO_RANDOM_NUMBER      (256 + 67)
#define FNIO_DEVICE_ENABLE      (256 + 68)
#define FNIO_DEVICE_DISABLE     (256 + 69)
#define FNIO_DEVICE_STATUS      (256 + 70)
#define FNIO_GENERATE_GUID      (256 + 71)

// ---- IOCTL command codes for the CLOCK: device ----
#define FNIO_CLOCK_SET_FORMAT   (256 + 80)
#define FNIO_CLOCK_SET_TZ       (256 + 81)

// ---- Clock time format constants ----
#define FN_TIME_BINARY_SIMPLE   0
#define FN_TIME_BINARY_PRODOS   1
#define FN_TIME_BINARY_APETIME  2
#define FN_TIME_ISO_STRING      3

// ---- WiFi status values (match fujinet-lib enum) ----
#define FN_WIFI_NO_SSID         1
#define FN_WIFI_CONNECTED       3
#define FN_WIFI_CONNECT_FAILED  4
#define FN_WIFI_CONNECTION_LOST 5

// ---- Network channel state ----
typedef struct FnChannel {
    int      in_use;
    int      sock_fd;
    FnProto  proto;
    int      mode;
    int      translation;
    int      connected;
    int      eof_flag;
    int      last_error;
    unsigned int bytes_waiting;
    char     devicespec[FN_MAX_DEVICESPEC];
    char     host[128];
    int      port;
    char     path[FN_MAX_URL];
    // HTTP state
    char     http_headers[FN_HTTP_HEADER_MAX];
    char     *http_body;
    int      http_body_len;
    int      http_body_pos;
    int      http_status_code;
    // JSON state
    char     *json_data;
    int      json_data_len;
    int      json_parsed;
    // TNFS state
    unsigned char tnfs_session[2];
    unsigned char tnfs_seq;
    int      tnfs_fd;
    // Receive buffer
    char     recv_buf[FN_RECV_BUF_SIZE];
    int      recv_pos;
    int      recv_len;
    // SSH state
    char     ssh_version[128]; // server banner
    int      ssh_exchanged; // 1 = version exchange done
} FnChannel;

// ---- FUJI device state ----
typedef struct FnFujiState {
    int      initialized;
    int      wifi_status;
    char     ssid[FN_SSID_MAXLEN];
    char     hostname[64];
    unsigned char local_ip[4];
    unsigned char gateway[4];
    unsigned char netmask[4];
    unsigned char dns[4];
    unsigned char mac[6];
    char     fn_version[16];
    char     host_slots[FN_MAX_HOST_SLOTS][FN_HOST_SLOT_LEN];
    // AppKey file-backed storage
    unsigned int appkey_creator;
    unsigned int appkey_app;
    unsigned char appkey_mode;
    // Device slots (desktop: local path aliases)
    char     device_slots[FN_MAX_DEVICE_SLOTS][FN_FILE_MAXLEN];
    int      device_enabled[FN_MAX_DEVICE_SLOTS];
    char     host_prefix[FN_MAX_HOST_SLOTS][FN_HOST_SLOT_LEN];
    char     device_path[FN_MAX_DEVICE_SLOTS][FN_FILE_MAXLEN];
    int      boot_mode;
    // Directory browsing state
    void    *dir_handle; // platform dir handle
    char     dir_path[256];
    int      dir_position;
    int      dir_open;
} FnFujiState;

// ---- Clock device state ----
typedef struct FnClockState {
    int      format;
    char     timezone[64];
} FnClockState;

// ---- Module registration ----

 // mod_fujinet_register - Register the FujiNet module.
 //
 // Call during interpreter boot (from main.c) to make the
 // FUJINET module available. The module registers itself but
 // does not activate until the BASIC program issues:
 //   MODULE "FUJINET"
void mod_fujinet_register(void);

#endif // BASICPP_MOD_FUJINET_H
