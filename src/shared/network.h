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

// SERVER
NetSocket listen(u_int16_t port);

// CLIENT

#ifdef __cplusplus
}
#endif

#endif
