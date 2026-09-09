// FILENAME: msg_broker.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel, libengine, libcore
// NEEDS: libkernel (device/msg_broker.h), libcore (runtime/format/runtime_snprintf.h, runtime/string/memops.h, runtime/string/strops.h)
// In-Memory Message Bus, Pub/Sub Broker & Point-to-Point IPC Queues Implementation.
//
// ---- Includes ----

#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include <stdbool.h>
#include "device/msg_broker.h"
#include "hal/hal.h"

static MsgTopicChannel g_topics[MSG_MAX_TOPICS];
static MsgIpcChannel   g_endpoints[MSG_MAX_ENDPOINTS];
static bool g_broker_initialized = false;

static uint64_t get_time_ms(void) {
    HalContext *hal = hal_get();
    if (hal && hal->time.monotonic_ms) {
        return hal->time.monotonic_ms();
    }
    return 0;
}

void msg_broker_init(void) {
    if (g_broker_initialized) return;
    runtime_memset(g_topics, 0, sizeof(g_topics));
    runtime_memset(g_endpoints, 0, sizeof(g_endpoints));
    g_broker_initialized = true;
}

void msg_broker_reset(void) {
    runtime_memset(g_topics, 0, sizeof(g_topics));
    runtime_memset(g_endpoints, 0, sizeof(g_endpoints));
    g_broker_initialized = true;
}

static MsgTopicChannel *find_or_create_topic(const char *topic, bool create) {
    if (!topic || !*topic) return NULL;
    msg_broker_init();

    // 1. Search existing
    for (int i = 0; i < MSG_MAX_TOPICS; i++) {
        if (g_topics[i].active && runtime_strcasecmp(g_topics[i].topic, topic) == 0) {
            return &g_topics[i];
        }
    }

    if (!create) return NULL;

    // 2. Allocate new
    for (int i = 0; i < MSG_MAX_TOPICS; i++) {
        if (!g_topics[i].active) {
            runtime_memset(&g_topics[i], 0, sizeof(MsgTopicChannel));
            runtime_snprintf(g_topics[i].topic, sizeof(g_topics[i].topic), "%s", topic);
            g_topics[i].active = true;
            return &g_topics[i];
        }
    }

    return NULL;
}

static MsgIpcChannel *find_or_create_endpoint(const char *endpoint, bool create) {
    if (!endpoint || !*endpoint) return NULL;
    msg_broker_init();

    // 1. Search existing
    for (int i = 0; i < MSG_MAX_ENDPOINTS; i++) {
        if (g_endpoints[i].active && runtime_strcasecmp(g_endpoints[i].endpoint, endpoint) == 0) {
            return &g_endpoints[i];
        }
    }

    if (!create) return NULL;

    // 2. Allocate new
    for (int i = 0; i < MSG_MAX_ENDPOINTS; i++) {
        if (!g_endpoints[i].active) {
            runtime_memset(&g_endpoints[i], 0, sizeof(MsgIpcChannel));
            runtime_snprintf(g_endpoints[i].endpoint, sizeof(g_endpoints[i].endpoint), "%s", endpoint);
            g_endpoints[i].active = true;
            return &g_endpoints[i];
        }
    }

    return NULL;
}

bool msg_broker_publish(const char *topic, const char *payload, size_t len) {
    if (!topic || !payload) return false;
    MsgTopicChannel *tc = find_or_create_topic(topic, true);
    if (!tc) return false;

    // 1. Store into circular message ring buffer
    int idx = (tc->head + tc->count) % MSG_QUEUE_CAPACITY;
    if (tc->count == MSG_QUEUE_CAPACITY) {
        tc->head = (tc->head + 1) % MSG_QUEUE_CAPACITY;
    } else {
        tc->count++;
    }

    size_t copy_len = len;
    if (copy_len >= MSG_PAYLOAD_MAX_LEN) copy_len = MSG_PAYLOAD_MAX_LEN - 1;
    runtime_memcpy(tc->queue[idx].payload, payload, copy_len);
    tc->queue[idx].payload[copy_len] = '\0';
    tc->queue[idx].length = copy_len;
    tc->queue[idx].timestamp_ms = get_time_ms();

    // 2. Deliver synchronously to registered subscribers
    for (int i = 0; i < MSG_MAX_SUBSCRIBERS; i++) {
        if (tc->subscribers[i].active && tc->subscribers[i].callback) {
            tc->subscribers[i].callback(topic, payload, len, tc->subscribers[i].userdata);
        }
    }

    return true;
}

bool msg_broker_unpublish(const char *topic, const char *payload) {
    if (!topic || !*topic) return false;
    MsgTopicChannel *tc = find_or_create_topic(topic, false);
    if (!tc) return false;

    // If payload is specified, remove matching messages from queue
    if (payload && *payload) {
        int original_count = tc->count;
        int new_count = 0;
        MsgQueueEntry temp_queue[MSG_QUEUE_CAPACITY];

        for (int i = 0; i < original_count; i++) {
            int idx = (tc->head + i) % MSG_QUEUE_CAPACITY;
            if (runtime_strcmp(tc->queue[idx].payload, payload) != 0) {
                temp_queue[new_count++] = tc->queue[idx];
            }
        }

        tc->head = 0;
        tc->count = new_count;
        for (int i = 0; i < new_count; i++) {
            tc->queue[i] = temp_queue[i];
        }
        return true;
    }

    // Otherwise unpublish whole topic: clear queue and de-register
    tc->head = 0;
    tc->count = 0;
    tc->sub_count = 0;
    for (int i = 0; i < MSG_MAX_SUBSCRIBERS; i++) {
        tc->subscribers[i].active = false;
        tc->subscribers[i].callback = NULL;
        tc->subscribers[i].userdata = NULL;
    }
    tc->active = false;
    return true;
}

void msg_broker_unpublish_all(void) {
    for (int i = 0; i < MSG_MAX_TOPICS; i++) {
        if (g_topics[i].active) {
            g_topics[i].head = 0;
            g_topics[i].count = 0;
            g_topics[i].sub_count = 0;
            for (int s = 0; s < MSG_MAX_SUBSCRIBERS; s++) {
                g_topics[i].subscribers[s].active = false;
                g_topics[i].subscribers[s].callback = NULL;
                g_topics[i].subscribers[s].userdata = NULL;
            }
            g_topics[i].active = false;
        }
    }
}

bool msg_broker_poll_topic(const char *topic, char *out_buf, size_t max_len) {
    if (!topic || !out_buf || max_len == 0) return false;
    MsgTopicChannel *tc = find_or_create_topic(topic, false);
    if (!tc || tc->count == 0) {
        out_buf[0] = '\0';
        return false;
    }

    // Pull from head
    int idx = tc->head;
    size_t copy_len = tc->queue[idx].length;
    if (copy_len >= max_len) copy_len = max_len - 1;
    runtime_memcpy(out_buf, tc->queue[idx].payload, copy_len);
    out_buf[copy_len] = '\0';

    tc->head = (tc->head + 1) % MSG_QUEUE_CAPACITY;
    tc->count--;
    return true;
}

bool msg_broker_subscribe(const char *topic, MsgBusCallback cb, void *userdata) {
    if (!topic || !cb) return false;
    MsgTopicChannel *tc = find_or_create_topic(topic, true);
    if (!tc) return false;

    // Check if already subscribed
    for (int i = 0; i < MSG_MAX_SUBSCRIBERS; i++) {
        if (tc->subscribers[i].active && tc->subscribers[i].callback == cb && tc->subscribers[i].userdata == userdata) {
            return true;
        }
    }

    // Add new subscriber
    for (int i = 0; i < MSG_MAX_SUBSCRIBERS; i++) {
        if (!tc->subscribers[i].active) {
            tc->subscribers[i].callback = cb;
            tc->subscribers[i].userdata = userdata;
            tc->subscribers[i].active = true;
            tc->sub_count++;
            return true;
        }
    }

    return false;
}

bool msg_broker_unsubscribe(const char *topic, MsgBusCallback cb) {
    if (!topic || !cb) return false;
    MsgTopicChannel *tc = find_or_create_topic(topic, false);
    if (!tc) return false;

    for (int i = 0; i < MSG_MAX_SUBSCRIBERS; i++) {
        if (tc->subscribers[i].active && tc->subscribers[i].callback == cb) {
            tc->subscribers[i].active = false;
            tc->subscribers[i].callback = NULL;
            tc->subscribers[i].userdata = NULL;
            tc->sub_count--;
            return true;
        }
    }

    return false;
}

int msg_broker_topic_msg_count(const char *topic) {
    MsgTopicChannel *tc = find_or_create_topic(topic, false);
    return tc ? tc->count : 0;
}

void msg_broker_clear_topic(const char *topic) {
    MsgTopicChannel *tc = find_or_create_topic(topic, false);
    if (tc) {
        tc->head = 0;
        tc->count = 0;
    }
}

bool msg_broker_send_ipc(const char *endpoint, const char *payload, size_t len) {
    if (!endpoint || !payload) return false;
    MsgIpcChannel *ep = find_or_create_endpoint(endpoint, true);
    if (!ep) return false;

    int idx = (ep->head + ep->count) % MSG_QUEUE_CAPACITY;
    if (ep->count == MSG_QUEUE_CAPACITY) {
        ep->head = (ep->head + 1) % MSG_QUEUE_CAPACITY;
    } else {
        ep->count++;
    }

    size_t copy_len = len;
    if (copy_len >= MSG_PAYLOAD_MAX_LEN) copy_len = MSG_PAYLOAD_MAX_LEN - 1;
    runtime_memcpy(ep->queue[idx].payload, payload, copy_len);
    ep->queue[idx].payload[copy_len] = '\0';
    ep->queue[idx].length = copy_len;
    ep->queue[idx].timestamp_ms = get_time_ms();

    return true;
}

bool msg_broker_recv_ipc(const char *endpoint, char *out_buf, size_t max_len, int timeout_ms) {
    (void)timeout_ms;
    if (!endpoint || !out_buf || max_len == 0) return false;
    MsgIpcChannel *ep = find_or_create_endpoint(endpoint, false);
    if (!ep || ep->count == 0) {
        out_buf[0] = '\0';
        return false;
    }

    int idx = ep->head;
    size_t copy_len = ep->queue[idx].length;
    if (copy_len >= max_len) copy_len = max_len - 1;
    runtime_memcpy(out_buf, ep->queue[idx].payload, copy_len);
    out_buf[copy_len] = '\0';

    ep->head = (ep->head + 1) % MSG_QUEUE_CAPACITY;
    ep->count--;
    return true;
}

int msg_broker_ipc_msg_count(const char *endpoint) {
    MsgIpcChannel *ep = find_or_create_endpoint(endpoint, false);
    return ep ? ep->count : 0;
}

void msg_broker_clear_ipc(const char *endpoint) {
    MsgIpcChannel *ep = find_or_create_endpoint(endpoint, false);
    if (ep) {
        ep->head = 0;
        ep->count = 0;
    }
}

#define MSG_MAX_SIGNAL_TRAPS 16
static MsgSignalTrap g_signal_traps[MSG_MAX_SIGNAL_TRAPS];

void msg_broker_set_signal_trap(const char *topic, int line_number, bool is_gosub) {
    if (!topic || !*topic) return;
    msg_broker_init();

    // Check if already registered
    for (int i = 0; i < MSG_MAX_SIGNAL_TRAPS; i++) {
        if (g_signal_traps[i].active && runtime_strcasecmp(g_signal_traps[i].topic, topic) == 0) {
            g_signal_traps[i].line_number = line_number;
            g_signal_traps[i].is_gosub = is_gosub;
            return;
        }
    }

    // Allocate new
    for (int i = 0; i < MSG_MAX_SIGNAL_TRAPS; i++) {
        if (!g_signal_traps[i].active) {
            runtime_snprintf(g_signal_traps[i].topic, sizeof(g_signal_traps[i].topic), "%s", topic);
            g_signal_traps[i].line_number = line_number;
            g_signal_traps[i].is_gosub = is_gosub;
            g_signal_traps[i].active = true;
            return;
        }
    }
}

void msg_broker_clear_signal_trap(const char *topic) {
    if (!topic || !*topic) return;
    for (int i = 0; i < MSG_MAX_SIGNAL_TRAPS; i++) {
        if (g_signal_traps[i].active && runtime_strcasecmp(g_signal_traps[i].topic, topic) == 0) {
            g_signal_traps[i].active = false;
            return;
        }
    }
}

int msg_broker_get_signal_trap(const char *topic, bool *out_is_gosub) {
    if (!topic || !*topic) return 0;
    for (int i = 0; i < MSG_MAX_SIGNAL_TRAPS; i++) {
        if (g_signal_traps[i].active && runtime_strcasecmp(g_signal_traps[i].topic, topic) == 0) {
            if (out_is_gosub) *out_is_gosub = g_signal_traps[i].is_gosub;
            return g_signal_traps[i].line_number;
        }
    }
    return 0;
}
