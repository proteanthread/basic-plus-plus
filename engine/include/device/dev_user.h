// FILENAME: dev_user.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c, stmt_user_ipc.c
// NEEDS: libkernel (vdev.h)
// Header definitions for User Virtual Device & IPC Mailbox (USER1..8:, USER:username$, USER0:, USER9:).
//
// ---- Includes ----

#ifndef DEVICE_DEV_USER_H
#define DEVICE_DEV_USER_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Creates a concrete VDev instance for a user device (USER1..8:, USER:username$, USER0:, USER9:).
VDev dev_user_create(const char *name);

// @brief Post a message to a specific user's mailbox.
bool dev_user_post_message(const char *target_user, const char *sender, const char *msg);

// @brief Fetch next message from a specific user's mailbox.
bool dev_user_fetch_message(const char *user, char *out_sender, size_t sender_size, char *out_msg, size_t msg_size);

// @brief Broadcast a message to all active user mailboxes (powering WALL).
int  dev_user_broadcast(const char *sender, const char *msg);

// @brief Set the message reception permission state for a user (powering MESG).
void dev_user_set_mesg(const char *user, bool allow);

// @brief Get the message reception permission state for a user.
bool dev_user_get_mesg(const char *user);

// @brief Log a system event to USER0: / syslog (powering LOGGER).
void dev_user_log_event(const char *msg, int level);

// @brief Retrieve the active current session username.
const char *dev_user_current_username(void);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_USER_H
