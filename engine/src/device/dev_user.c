// FILENAME: dev_user.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c, stmt_user_ipc.c
// NEEDS: libkernel (dev_user.h, vdev.h), libplatform (plat_sys.h)
// Implementation for User Virtual Device & IPC Mailbox (USER1..8:, USER:username$, USER0:, USER9:).
//
// ---- Includes ----

#include "device/dev_user.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/strings.h"
#include "platform/platform.h"
#include "runtime/memory/alloc.h"

#define MAX_USER_MAILBOXES 32
#define MAX_MESSAGES_PER_BOX 128
#define MAX_MSG_LEN 512

typedef struct {
    char sender[32];
    char text[MAX_MSG_LEN];
} QueuedMessage;

typedef struct {
    char name[32];
    bool active;
    bool mesg_allowed;
    QueuedMessage messages[MAX_MESSAGES_PER_BOX];
    int head;
    int tail;
    int count;
} UserMailbox;

static UserMailbox s_mailboxes[MAX_USER_MAILBOXES];
static bool s_user_inited = false;
static char s_current_user[32] = "user";

static void user_init_all(void) {
    if (s_user_inited) return;
    runtime_memset(s_mailboxes, 0, sizeof(s_mailboxes));

    // Initialize USER0: (syslog/logger), USER1..8: (sessions), USER9: (broadcast)
    for (int i = 0; i < 10; i++) {
        s_mailboxes[i].active = true;
        s_mailboxes[i].mesg_allowed = true;
        runtime_snprintf(s_mailboxes[i].name, sizeof(s_mailboxes[i].name), "USER%d:", i);
    }

    // Default current username from environment
    const char *env_user = platform_getenv("USERNAME");
    if (!env_user) env_user = platform_getenv("USER");
    if (env_user && env_user[0]) {
        runtime_snprintf(s_current_user, sizeof(s_current_user), "%s", env_user);
    }

    s_user_inited = true;
}

static UserMailbox *user_find_or_create_mailbox(const char *name) {
    user_init_all();
    if (!name || !name[0]) return &s_mailboxes[1]; // default to USER1:

    // Match existing
    for (int i = 0; i < MAX_USER_MAILBOXES; i++) {
        if (s_mailboxes[i].active && runtime_strcasecmp(s_mailboxes[i].name, name) == 0) {
            return &s_mailboxes[i];
        }
    }

    // Allocate new named user slot
    for (int i = 10; i < MAX_USER_MAILBOXES; i++) {
        if (!s_mailboxes[i].active) {
            s_mailboxes[i].active = true;
            s_mailboxes[i].mesg_allowed = true;
            s_mailboxes[i].head = 0;
            s_mailboxes[i].tail = 0;
            s_mailboxes[i].count = 0;
            runtime_snprintf(s_mailboxes[i].name, sizeof(s_mailboxes[i].name), "%s", name);
            return &s_mailboxes[i];
        }
    }

    return &s_mailboxes[1];
}

const char *dev_user_current_username(void) {
    user_init_all();
    return s_current_user;
}

void dev_user_set_mesg(const char *user, bool allow) {
    UserMailbox *box = user_find_or_create_mailbox(user);
    if (box) {
        box->mesg_allowed = allow;
    }
}

bool dev_user_get_mesg(const char *user) {
    UserMailbox *box = user_find_or_create_mailbox(user);
    return box ? box->mesg_allowed : true;
}

bool dev_user_post_message(const char *target_user, const char *sender, const char *msg) {
    if (!target_user || !msg) return false;
    UserMailbox *box = user_find_or_create_mailbox(target_user);
    if (!box || !box->mesg_allowed) return false;

    if (box->count >= MAX_MESSAGES_PER_BOX) {
        // Drop oldest message on overflow
        box->tail = (box->tail + 1) % MAX_MESSAGES_PER_BOX;
        box->count--;
    }

    QueuedMessage *qm = &box->messages[box->head];
    runtime_snprintf(qm->sender, sizeof(qm->sender), "%s", sender ? sender : s_current_user);
    runtime_snprintf(qm->text, sizeof(qm->text), "%s", msg);

    box->head = (box->head + 1) % MAX_MESSAGES_PER_BOX;
    box->count++;
    return true;
}

bool dev_user_fetch_message(const char *user, char *out_sender, size_t sender_size, char *out_msg, size_t msg_size) {
    user_init_all();
    UserMailbox *box = user_find_or_create_mailbox(user);
    if (!box || box->count <= 0) return false;

    QueuedMessage *qm = &box->messages[box->tail];
    if (out_sender && sender_size > 0) {
        runtime_snprintf(out_sender, sender_size, "%s", qm->sender);
    }
    if (out_msg && msg_size > 0) {
        runtime_snprintf(out_msg, msg_size, "%s", qm->text);
    }

    box->tail = (box->tail + 1) % MAX_MESSAGES_PER_BOX;
    box->count--;
    return true;
}

int dev_user_broadcast(const char *sender, const char *msg) {
    user_init_all();
    if (!msg) return 0;
    int delivered = 0;
    for (int i = 1; i < MAX_USER_MAILBOXES; i++) {
        if (s_mailboxes[i].active && s_mailboxes[i].mesg_allowed && i != 9) {
            if (dev_user_post_message(s_mailboxes[i].name, sender, msg)) {
                delivered++;
            }
        }
    }
    // Also post to USER9: broadcast bus
    dev_user_post_message("USER9:", sender, msg);
    return delivered;
}

void dev_user_log_event(const char *msg, int level) {
    (void)level;
    dev_user_post_message("USER0:", "LOGGER", msg);
}

static int user_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    UserMailbox *box = (UserMailbox *)dev->priv;
    char s[2] = {(char)c, 0};
    dev_user_post_message(box->name, s_current_user, s);
    return c;
}

static int user_puts(VDev *dev, const char *s) {
    if (!dev || !dev->priv || !s) return -1;
    UserMailbox *box = (UserMailbox *)dev->priv;
    if (runtime_strcasecmp(box->name, "USER9:") == 0 || runtime_strcasecmp(box->name, "WALL:") == 0) {
        return dev_user_broadcast(s_current_user, s);
    }
    dev_user_post_message(box->name, s_current_user, s);
    return (int)runtime_strlen(s);
}

static int user_getc(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    UserMailbox *box = (UserMailbox *)dev->priv;
    if (box->count <= 0) return -1;
    QueuedMessage *qm = &box->messages[box->tail];
    int ch = (unsigned char)qm->text[0];
    // advance queue if single character consumed
    box->tail = (box->tail + 1) % MAX_MESSAGES_PER_BOX;
    box->count--;
    return ch;
}

static char *user_gets(VDev *dev, char *buf, size_t size) {
    if (!dev || !dev->priv || !buf || size == 0) return NULL;
    UserMailbox *box = (UserMailbox *)dev->priv;
    if (box->count <= 0) return NULL;

    QueuedMessage *qm = &box->messages[box->tail];
    runtime_snprintf(buf, size, "[%s] %s", qm->sender, qm->text);

    box->tail = (box->tail + 1) % MAX_MESSAGES_PER_BOX;
    box->count--;
    return buf;
}

static int user_flush(VDev *dev) {
    (void)dev;
    return 0;
}

static int user_cls(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    UserMailbox *box = (UserMailbox *)dev->priv;
    box->head = 0;
    box->tail = 0;
    box->count = 0;
    return 0;
}

static int user_dev_read(VDev *d, void *buf, int len) {
    if (!d || !buf || len <= 0) return 0;
    char line[MAX_MSG_LEN];
    if (user_gets(d, line, sizeof(line))) {
        int slen = (int)runtime_strlen(line);
        int copy_len = (slen < len) ? slen : len;
        runtime_memcpy(buf, line, copy_len);
        return copy_len;
    }
    return 0;
}

static int user_dev_write(VDev *d, const void *buf, int len) {
    if (!d || !buf || len <= 0) return 0;
    char text[MAX_MSG_LEN];
    int copy_len = (len < (int)sizeof(text) - 1) ? len : (int)sizeof(text) - 1;
    runtime_memcpy(text, buf, copy_len);
    text[copy_len] = '\0';
    return user_puts(d, text);
}

static int user_dev_ioctl(VDev *d, int cmd, void *arg) {
    if (!d || !d->priv) return -1;
    UserMailbox *box = (UserMailbox *)d->priv;
    switch (cmd) {
        case 0: // Reset
            box->head = 0;
            box->tail = 0;
            box->count = 0;
            return 0;
        case 1: // Get unread count
            if (arg) *(int *)arg = box->count;
            return box->count;
        case 2: // Set MESG (arg != NULL or non-zero)
            box->mesg_allowed = (arg != NULL);
            return 0;
        case 3: // Get MESG
            return box->mesg_allowed ? 1 : 0;
        case 13: // Status (pending messages)
            return box->count;
        default:
            return 0;
    }
}

static int user_dev_status(VDev *d) {
    if (!d || !d->priv) return 0;
    UserMailbox *box = (UserMailbox *)d->priv;
    return box->count;
}

static int user_dev_poll(VDev *d) {
    if (!d || !d->priv) return 0;
    UserMailbox *box = (UserMailbox *)d->priv;
    return (box->count > 0) ? 1 : 0;
}

static const char *user_dev_info(VDev *d, const char *key) {
    if (!d || !key) return NULL;
    if (runtime_strcasecmp(key, "CLASS") == 0) return "USER";
    if (runtime_strcasecmp(key, "DRIVER") == 0) return "dev_user";
    if (runtime_strcasecmp(key, "VERSION") == 0) return "6.5.2";
    if (runtime_strcasecmp(key, "CAPS") == 0) return "RW,STREAM,DUPLEX,MAILBOX,RATE_LIMITED";
    return NULL;
}

VDev dev_user_create(const char *name) {
    user_init_all();
    UserMailbox *box = user_find_or_create_mailbox(name ? name : "USER1:");

    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = box->name;
    dev.dev_class = VDCLASS_USER;
    dev.dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_DUPLEX | VDCAP_STATUS | VDCAP_CONTROL;
    dev.dev_version = "6.5.2.0";
    dev.dev_description = "User Virtual Device & IPC Mailbox";
    dev.priv = box;

    dev.ops.putc = user_putc;
    dev.ops.puts = user_puts;
    dev.ops.getc = user_getc;
    dev.ops.gets = user_gets;
    dev.ops.flush = user_flush;
    dev.ops.cls = user_cls;

    dev.dev_read = user_dev_read;
    dev.dev_write = user_dev_write;
    dev.dev_seek = NULL;
    dev.dev_ioctl = user_dev_ioctl;
    dev.dev_status = user_dev_status;
    dev.dev_poll = user_dev_poll;
    dev.dev_info = user_dev_info;
    dev.dev_open = NULL;
    dev.dev_close = NULL;

    return dev;
}
