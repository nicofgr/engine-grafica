#ifndef NETWORK_H
#define NETWORK_H

#include "types.h"
#include <stdint.h>
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t NetSocket;
typedef uint32_t NetConn;

// BOTH
void net_init();
void net_update();

// SERVER
NetSocket net_listen(u_int16_t port);

// CLIENT
NetConn net_connect(u_int16_t port);

#ifdef __cplusplus
}
#endif

#endif
