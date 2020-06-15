#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <endian.h>
#include <byteswap.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>
#include "kv_errno.h"
#include "kv_log.h"
#include "mempool.h"

// DEBUG_SET_LEVEL(DEBUG_LEVEL_DEBUG);

#define DEFAULT_SERVER "10.10.88.201"
#define DEFAULT_PORT 8099
#define SOCK_BACKLOG_CONN SOMAXCONN
#define MAX_MEMPOOL_SIZE 32 * GB
#define EACH_CHUNK_SIZE 0.5 * GB

enum CMD{
    CMD_PUT,
    CMD_GET,
    CMD_REMOVE,
    CMD_FREE,
    CMD_DESTROY,
    CMD_LEN
};

struct item{
    int len;
    void *data;
};
typedef struct item item_t;

struct message{
    uint8_t cmd;
    uint16_t key_len;
    uint16_t value_len;
    uint8_t data[0];
}__attribute__ ((packed));
typedef struct message message_t;

struct reply{
    uint8_t errno;
    uint16_t key_len;
    uint16_t value_len;
    uint8_t data[0];
}__attribute__ ((packed));
typedef struct reply reply_t;

#endif