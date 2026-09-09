// FILENAME: msg_broker.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel, libengine, libcore
// Declarations for In-Memory Message Bus, Pub/Sub Broker & Point-to-Point IPC Queues.
//
// ---- Includes ----

#ifndef MSG_BROKER_H
#define MSG_BROKER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_MAX_TOPICS       32
#define MSG_MAX_ENDPOINTS    32
#define MSG_MAX_SUBSCRIBERS  16
#define MSG_QUEUE_CAPACITY   32
#define MSG_NAME_MAX_LEN     64
#define MSG_PAYLOAD_MAX_LEN  1024

typedef void (*MsgBusCallback)(const char *topic, const char *payload, size_t len, void *userdata);

typedef struct {
    char payload[MSG_PAYLOAD_MAX_LEN];
    size_t length;
    uint64_t timestamp_ms;
} MsgQueueEntry;

typedef struct {
    MsgBusCallback callback;
    void *userdata;
    bool active;
} MsgSubscriber;

typedef struct {
    char topic[MSG_NAME_MAX_LEN];
    MsgQueueEntry queue[MSG_QUEUE_CAPACITY];
    int head;
    int count;
    MsgSubscriber subscribers[MSG_MAX_SUBSCRIBERS];
    int sub_count;
    bool active;
} MsgTopicChannel;

typedef struct {
    char endpoint[MSG_NAME_MAX_LEN];
    MsgQueueEntry queue[MSG_QUEUE_CAPACITY];
    int head;
    int count;
    bool active;
} MsgIpcChannel;

typedef struct {
    char topic[MSG_NAME_MAX_LEN];
    int line_number;
    bool is_gosub;
    bool active;
} MsgSignalTrap;

// Lifecycle Management
void msg_broker_init(void);
void msg_broker_reset(void);

// Publish / Subscribe Operations
bool msg_broker_publish(const char *topic, const char *payload, size_t len);
bool msg_broker_unpublish(const char *topic, const char *payload);
void msg_broker_unpublish_all(void);
bool msg_broker_poll_topic(const char *topic, char *out_buf, size_t max_len);
bool msg_broker_subscribe(const char *topic, MsgBusCallback cb, void *userdata);
bool msg_broker_unsubscribe(const char *topic, MsgBusCallback cb);
int  msg_broker_topic_msg_count(const char *topic);
void msg_broker_clear_topic(const char *topic);

// Point-to-Point IPC Operations
bool msg_broker_send_ipc(const char *endpoint, const char *payload, size_t len);
bool msg_broker_recv_ipc(const char *endpoint, char *out_buf, size_t max_len, int timeout_ms);
int  msg_broker_ipc_msg_count(const char *endpoint);
void msg_broker_clear_ipc(const char *endpoint);

// Signal Trapping Operations
void msg_broker_set_signal_trap(const char *topic, int line_number, bool is_gosub);
void msg_broker_clear_signal_trap(const char *topic);
int  msg_broker_get_signal_trap(const char *topic, bool *out_is_gosub);

#ifdef __cplusplus
}
#endif

#endif // MSG_BROKER_H
