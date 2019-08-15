/****************************************************************************
 * Copyright (C) 2015
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#ifndef __SOCKET_FUNCTIONS_H_
#define __SOCKET_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

extern uint32_t nsysnet_handle;

extern uint32_t hostIpAddress;

#define INADDR_ANY      0
#define INADDR_BROADCAST 0xFFFFFFFF

#define AF_INET         2

#define SHUT_RD         0
#define SHUT_WR         1
#define SHUT_RDWR       2

#define SOCK_STREAM     1
#define SOCK_DGRAM      2

#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17

#define TCP_NODELAY     0x2004

#define SOL_SOCKET      -1
#define SO_REUSEADDR    0x0004
#define SO_BROADCAST    0x0020      // broadcast
#define SO_NONBLOCK     0x1016
#define SO_MYADDR       0x1013
#define SO_RCVTIMEO     0x1006

#define SOL_SOCKET      -1
#define MSG_DONTWAIT    32

#define htonl(x) x
#define htons(x) x
#define ntohl(x) x
#define ntohs(x) x

struct in_addr {
    uint32_t s_addr;
};
struct sockaddr_in {
    short sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
};

struct sockaddr {
    unsigned short sa_family;
    char sa_data[14];
};


void InitSocketFunctionPointers(void);
void InitAcquireSocket(void);

extern int32_t (*socket_lib_init)(void);
extern int32_t (*socket_lib_finish)(void);
extern int32_t (*socket)(int32_t domain, int32_t type, int32_t protocol);
extern int32_t (*socketclose)(int32_t s);
extern int32_t (*shutdown)(int32_t s, int32_t how);
extern int32_t (*connect)(int32_t s, void *addr, int32_t addrlen);
extern int32_t (*bind)(int32_t s,struct sockaddr *name,int32_t namelen);
extern int32_t (*listen)(int32_t s,uint32_t backlog);
extern int32_t (*accept)(int32_t s,struct sockaddr *addr,int32_t *addrlen);
extern int32_t (*send)(int32_t s, const void *buffer, int32_t size, int32_t flags);
extern int32_t (*recv)(int32_t s, void *buffer, int32_t size, int32_t flags);
extern int32_t (*recvfrom)(int32_t sockfd, void *buf, int32_t len, int32_t flags,struct sockaddr *src_addr, int32_t *addrlen);
extern int32_t (*socketlasterr)(void);

extern int32_t (*sendto)(int32_t s, const void *buffer, int32_t size, int32_t flags, const struct sockaddr *dest, int32_t dest_len);
extern int32_t (*setsockopt)(int32_t s, int32_t level, int32_t optname, void *optval, int32_t optlen);

extern int32_t (* NSSLInit)(void);
extern int32_t (* NSSLFinish)(void);
extern int32_t (* NSSLCreateContext)(int32_t unkwn);
extern int32_t (* NSSLDestroyContext)(int32_t context);
extern int32_t (* NSSLAddServerPKIExternal)(int32_t context, const uint8_t* cert, int32_t length, int32_t unkwn);
extern int32_t (* NSSLAddServerPKI)(int32_t context, int32_t pki);
extern int32_t (* NSSLWrite)(int32_t connection, const void* buf, int32_t len,int32_t * written);
extern int32_t (* NSSLRead)(int32_t connection, const void* buf, int32_t len,int32_t * read);
extern int32_t (* NSSLCreateConnection)(int32_t context, const char* host, int32_t hotlen,int32_t options,int32_t sock,int32_t block);

extern char * (*inet_ntoa)(struct in_addr in);
extern int32_t (*inet_aton)(const char *cp, struct in_addr *inp);
extern const char * (*inet_ntop)(int32_t af, const void *src, char *dst, int32_t size);
extern int32_t (*inet_pton)(int32_t af, const char *src, void *dst);

#ifdef __cplusplus
}
#endif

#endif // __SOCKET_FUNCTIONS_H_
