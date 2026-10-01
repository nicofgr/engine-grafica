// Make it work like a standalone library,
// with no dependencies on the engine,
// just as a translator from cpp to c.
//
#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t NetSocket;
typedef uint32_t NetConn;

#pragma pack(push,1) // TLV Type-Length-Value


typedef struct PacketHeader{
        uint8_t   type; // Max 256 types
        uint16_t  size; // Max 65536 bytes
}PacketHeader;

#pragma pack(pop)

// BOTH
void net_init();
void net_update();
void net_shutdown();
//void net_send(HSteamNetConnection conn, Type type, u16 size, const void* buf);
void net_recv();

// SERVER
NetSocket net_listen(u_int16_t port);

// CLIENT
NetConn net_connect(u_int16_t port);

#ifdef __cplusplus
}
#endif

#endif
