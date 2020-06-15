#include "linked_hashmap.h"
#include "common.h"

void cmd_put(int sockfd, map_t map, message_t *in){
    void *data = mempool_alloc(mp, in->key_len + in->value_len);
    memset(data, 0, in->key_len + in->value_len);
    // DEBUG("[malloc] in data addr is %llu", data);
    int n = read(sockfd, data, in->key_len + in->value_len);

    int errno = PUT(map, data, data + in->key_len);
    INFO("PUT, errno=%s, key=%s, value=%s, addr=%llu", status_name[errno], data, data + in->key_len, data);

    reply_t out;
    out.key_len = 0;
    out.value_len = 0;
    out.errno = errno;
    n = write(sockfd, (void*)&out, sizeof(reply_t));
}

void cmd_get(int sockfd, map_t map, message_t *in){
    // void *data = calloc(in->key_len, sizeof(char));
    void *data = mempool_alloc(mp, in->key_len);
    memset(data, 0, in->key_len);
    // DEBUG("[malloc] in data addr is %llu", data);
    int n = read(sockfd, data, in->key_len);

    void *value = NULL;
    int errno = GET(map, data, (void**)&value);
    INFO("GET, err_name=%s, key=%s, value=%s, addr=%llu", status_name[errno], data, value, value - in->key_len);
    mempool_free(mp, data);

    reply_t resp;
    resp.errno = errno;
    resp.key_len = 0;
    resp.value_len = 0;
    if (errno == MAP_OK){
        resp.value_len = strlen((char*)value) + 1;
    }
    
    void *out = NULL;
    out = mempool_alloc(mp, sizeof(reply_t) + resp.value_len);
    memset(out, 0, sizeof(reply_t) + resp.value_len);
    // DEBUG("[malloc] out data addr is %llu", out);
    memcpy(out, &resp, sizeof(reply_t));
    if (errno == MAP_OK)
        memcpy(out + sizeof(reply_t), value, resp.value_len);
    n = write(sockfd, out, sizeof(reply_t) + resp.value_len);
    // DEBUG("[free] out data addr is %llu", out);
    mempool_free(mp, out);
}

void cmd_remove(int sockfd, map_t map, message_t *in){
    // print_hashmap(map);
    // void *data = calloc(in->key_len, sizeof(char));
    void *data = mempool_alloc(mp, in->key_len);
    memset(data, 0, in->key_len);
    // DEBUG("[malloc] in data addr is %llu", data);
    int n = read(sockfd, data, in->key_len);

    // free(ptr);
    void *key_addr;
    int errno = REMOVE(map, data, (void**)&key_addr, NULL);
    if (errno == MAP_OK)
        INFO("REMOVE, errno=%s, key=%s, key addr=%llu", status_name[errno], key_addr, key_addr);
    else
        INFO("REMOVE, errno=%s, key=%s", status_name[errno], data);
    // DEBUG("[free] in data addr is %llu", data);
    mempool_free(mp, data);
    if (errno == MAP_OK)
        mempool_free(mp, key_addr);
    // print_hashmap(map);

    reply_t out;
    out.key_len = 0;
    out.value_len = 0;
    out.errno = errno;
    n = write(sockfd, (void*)&out, sizeof(reply_t));
}

void cmd_free(map_t map){
    // free key-value address
    // print_hashmap(map);
    Iterator *it = new_iterator(map);
    while (has_next(it)){
        it = next(it);
        // DEBUG("free start, [%s,%s], addr=%llu", it->entry->key, it->entry->value, it->entry->key);
        INFO("free, key=%s, value=%s, addr=%llu", it->entry->key, it->entry->value, it->entry->key);
        mempool_free(mp, it->entry->key);
    }
    free_iterator(it);
    // free data node

    FREE(map);
    // print_hashmap(map);
    // DEBUG("free hashmap");
}

void cmd_destroy(map_t map){
    DESTROY(map);
}

void cmd_len(int sockfd, map_t map){
    int len = LEN(map);
    // DEBUG("LEN, len is %d", len);
    INFO("LEN, len=%d", len);

    reply_t out;
    out.errno = MAP_OK;
    out.key_len = len;
    out.value_len = 0;
    write(sockfd, (void*)&out, sizeof(reply_t));
}