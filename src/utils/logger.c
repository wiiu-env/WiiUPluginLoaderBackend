#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "logger.h"
#include "dynamic_libs/coreinit.h"
#include "dynamic_libs/socket_functions.h"

static int log_socket __attribute__((section(".data")))= -1;
static struct sockaddr_in connect_addr __attribute__((section(".data")));
static volatile int log_lock __attribute__((section(".data"))) = 0;

void log_init() {
    InitOSFunctionPointers();
    InitSocketFunctionPointers();

    int broadcastEnable = 1;
    log_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (log_socket < 0){
        return;
    }

    setsockopt(log_socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    memset(&connect_addr, 0, sizeof(struct sockaddr_in));
    connect_addr.sin_family = AF_INET;
    connect_addr.sin_port = 4405;
    connect_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
}

void log_print(const char *str) {
    // socket is always 0 initially as it is in the BSS
    if(log_socket < 0) {
        return;
    }

    while(log_lock)
        os_usleep(1000);
    log_lock = 1;

    int len = strlen(str);
    int ret;
    while (len > 0) {
        int block = len < 1400 ? len : 1400; // take max 1400 bytes per UDP packet
        ret = sendto(log_socket, str, block, 0, (struct sockaddr *)&connect_addr, sizeof(struct sockaddr_in));
        if(ret < 0)
            break;

        len -= ret;
        str += ret;
    }

    log_lock = 0;
}

void OSFatal_printf(const char *format, ...) {
    char * tmp = NULL;
    va_list va;
    va_start(va, format);
    if((vasprintf(&tmp, format, va) >= 0) && tmp) {
        OSFatal(tmp);
    }
    va_end(va);
}

void log_printf(const char *format, ...) {
    if(log_socket < 0) {
        return;
    }


    char * tmp = NULL;

    va_list va;
    va_start(va, format);
    if((vasprintf(&tmp, format, va) >= 0) && tmp) {
        log_print(tmp);
    }
    va_end(va);

    if(tmp)
        free(tmp);
}

