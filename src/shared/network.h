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

#pragma pack(push,1) // TLV Type-Length-Value

typedef enum Type{
        INPUT,
        POSITION_UPDATE,
        OBJECT
}Type;

typedef struct PacketHeader{
        u16 type;
        u16 size;
}PacketHeader;

#pragma pack(pop)

// BOTH
void net_init();
void net_update();
void net_shutdown();
void net_send(Type type, u16 size, const void* buf);
void net_recv();

// SERVER
NetSocket net_listen(u_int16_t port);

// CLIENT
NetConn net_connect(u_int16_t port);

#ifdef __cplusplus
}
#endif

#endif
