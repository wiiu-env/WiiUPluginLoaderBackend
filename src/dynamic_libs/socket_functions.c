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
#include "coreinit.h"
#include "socket_functions.h"

uint32_t hostIpAddress = 0;

uint32_t nsysnet_handle __attribute__((section(".data"))) = 0;

EXPORT_DECL(int32_t, socket_lib_init, void);
EXPORT_DECL(int32_t, socket_lib_finish, void);
EXPORT_DECL(int32_t, socket, int32_t domain, int32_t type, int32_t protocol);
EXPORT_DECL(int32_t, socketclose, int32_t s);
EXPORT_DECL(int32_t, shutdown, int32_t s, int32_t how);
EXPORT_DECL(int32_t, connect, int32_t s, void *addr, int32_t addrlen);
EXPORT_DECL(int32_t, bind, int32_t s,struct sockaddr *name,int32_t namelen);
EXPORT_DECL(int32_t, listen, int32_t s,uint32_t backlog);
EXPORT_DECL(int32_t, accept, int32_t s,struct sockaddr *addr,int32_t *addrlen);
EXPORT_DECL(int32_t, send, int32_t s, const void *buffer, int32_t size, int32_t flags);
EXPORT_DECL(int32_t, recv, int32_t s, void *buffer, int32_t size, int32_t flags);
EXPORT_DECL(int32_t, recvfrom,int32_t sockfd, void *buf, int32_t len, int32_t flags,struct sockaddr *src_addr, int32_t *addrlen);
EXPORT_DECL(int32_t, sendto, int32_t s, const void *buffer, int32_t size, int32_t flags, const struct sockaddr *dest, int32_t dest_len);
EXPORT_DECL(int32_t, setsockopt, int32_t s, int32_t level, int32_t optname, void *optval, int32_t optlen);
EXPORT_DECL(char *, inet_ntoa, struct in_addr in);
EXPORT_DECL(int32_t, inet_aton, const char *cp, struct in_addr *inp);
EXPORT_DECL(const char *, inet_ntop, int32_t af, const void *src, char *dst, int32_t size);
EXPORT_DECL(int32_t, inet_pton, int32_t af, const char *src, void *dst);
EXPORT_DECL(int32_t, socketlasterr, void);

EXPORT_DECL(int32_t, NSSLInit, void);
EXPORT_DECL(int32_t, NSSLFinish, void);
EXPORT_DECL(int32_t, NSSLCreateContext, int32_t unkwn);
EXPORT_DECL(int32_t, NSSLDestroyContext, int32_t context);
EXPORT_DECL(int32_t, NSSLAddServerPKI, int32_t context, int32_t pki);
EXPORT_DECL(int32_t, NSSLAddServerPKIExternal, int32_t context, const uint8_t* cert, int32_t length, int32_t unkwn);
EXPORT_DECL(int32_t, NSSLWrite, int32_t connection, const void* buf, int32_t len,int32_t * written);
EXPORT_DECL(int32_t, NSSLRead, int32_t connection, const void* buf, int32_t len,int32_t * read);
EXPORT_DECL(int32_t, NSSLCreateConnection, int32_t context, const char* host, int32_t hotlen,int32_t options,int32_t sock,int32_t block);

void InitAcquireSocket(void) {
    OSDynLoad_Acquire("nsysnet.rpl", &nsysnet_handle);
}

void InitSocketFunctionPointers(void) {
    uint32_t *funcPointer = 0;

    InitAcquireSocket();

    uint32_t nn_ac_handle;
    int32_t(*ACInitialize)();
    int32_t(*ACGetStartupId) (uint32_t *id);
    int32_t(*ACConnectWithConfigId) (uint32_t id);
    int32_t(*ACGetAssignedAddress) (uint32_t * ip);
    OSDynLoad_Acquire("nn_ac.rpl", &nn_ac_handle);
    OSDynLoad_FindExport(nn_ac_handle, 0, "ACInitialize", &ACInitialize);
    OSDynLoad_FindExport(nn_ac_handle, 0, "ACGetStartupId", &ACGetStartupId);
    OSDynLoad_FindExport(nn_ac_handle, 0, "ACConnectWithConfigId",&ACConnectWithConfigId);
    OSDynLoad_FindExport(nn_ac_handle, 0, "ACGetAssignedAddress",&ACGetAssignedAddress);

    OS_FIND_EXPORT(nsysnet_handle, socket_lib_init);
    OS_FIND_EXPORT(nsysnet_handle, socket_lib_finish);
    OS_FIND_EXPORT(nsysnet_handle, socketlasterr);
    OS_FIND_EXPORT(nsysnet_handle, socket);
    OS_FIND_EXPORT(nsysnet_handle, socketclose);
    OS_FIND_EXPORT(nsysnet_handle, shutdown);
    OS_FIND_EXPORT(nsysnet_handle, connect);
    OS_FIND_EXPORT(nsysnet_handle, bind);
    OS_FIND_EXPORT(nsysnet_handle, listen);
    OS_FIND_EXPORT(nsysnet_handle, accept);
    OS_FIND_EXPORT(nsysnet_handle, send);
    OS_FIND_EXPORT(nsysnet_handle, recv);
    OS_FIND_EXPORT(nsysnet_handle, recvfrom);
    OS_FIND_EXPORT(nsysnet_handle, sendto);
    OS_FIND_EXPORT(nsysnet_handle, setsockopt);
    OS_FIND_EXPORT(nsysnet_handle, inet_ntoa);
    OS_FIND_EXPORT(nsysnet_handle, inet_aton);
    OS_FIND_EXPORT(nsysnet_handle, inet_ntop);
    OS_FIND_EXPORT(nsysnet_handle, inet_pton);

    OS_FIND_EXPORT(nsysnet_handle, NSSLInit);
    OS_FIND_EXPORT(nsysnet_handle, NSSLFinish);
    OS_FIND_EXPORT(nsysnet_handle, NSSLCreateContext);
    OS_FIND_EXPORT(nsysnet_handle, NSSLDestroyContext);
    OS_FIND_EXPORT(nsysnet_handle, NSSLAddServerPKI);
    OS_FIND_EXPORT(nsysnet_handle, NSSLAddServerPKIExternal);
    OS_FIND_EXPORT(nsysnet_handle, NSSLWrite);
    OS_FIND_EXPORT(nsysnet_handle, NSSLRead);
    OS_FIND_EXPORT(nsysnet_handle, NSSLCreateConnection);

    uint32_t nn_startupid;
    ACInitialize();
    ACGetStartupId(&nn_startupid);
    ACConnectWithConfigId(nn_startupid);
    ACGetAssignedAddress(&hostIpAddress);

    socket_lib_init();
}
