#ifndef BASICPP_VDEV_NET_H
#define BASICPP_VDEV_NET_H

#include "../vdev.h"

/* Initialize the networking subsystem (e.g., WinSock) */
int vdev_net_init(void);

/* Clean up the networking subsystem */
void vdev_net_cleanup(void);

/* Open a network device given a URI (TCP:host:port or UDP:host:port).
 * Returns a dynamically allocated VDev on success, or NULL on failure.
 */
VDev *vdev_net_open(const char *uri);

#endif /* BASICPP_VDEV_NET_H */
