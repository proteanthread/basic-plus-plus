#ifndef BASICPP_VDEV_NET_H
#define BASICPP_VDEV_NET_H

#include "../vdev.h"

// Initialize the networking subsystem (e.g., WinSock)
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
int vdev_net_init(void);

// Clean up the networking subsystem
void vdev_net_cleanup(void);

// Open a network device given a URI (TCP:host:port or UDP:host:port).
 // Returns a dynamically allocated VDev on success, or NULL on failure.
VDev *vdev_net_open(const char *uri);

#endif // BASICPP_VDEV_NET_H
