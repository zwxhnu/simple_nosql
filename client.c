#include "common.h"
#include "client_api.h"
#include "kv_log.h"
// DEBUG_SET_LEVEL(DEBUG_LEVEL_DEBUG);

char *ip = DEFAULT_SERVER;
int port = DEFAULT_PORT;
unsigned long max_mempool_size = MAX_MEMPOOL_SIZE;
unsigned long each_chunk_size = EACH_CHUNK_SIZE;
int num_data = 100;

void usage(char *program);
void parse_args(int argc, char *argv[]);
int connect_setup();
void *handle_connection(void* ptr);
void test_func1(int sockfd);
void test_func2(int sockfd);


int main(int argc, char *argv[]){
    parse_args(argc, argv);
    INFO("each chunk size=%lu, max mempool size=%lu, number of put data=%d", each_chunk_size, max_mempool_size, num_data);
    int sockfd = connect_setup();
    mp = mempool_init(each_chunk_size, max_mempool_size);
    handle_connection((void*)&sockfd);
    mempool_clear(mp);
    mempool_destroy(mp);
    return 0;
}

void *handle_connection(void *ptr){
    int sockfd = *(int*)ptr;
    // INFO("--------------------------test_func1--------------------------");
    // test_func1(sockfd);
    INFO("--------------------------test_func2--------------------------");
    test_func2(sockfd);
    map_free(sockfd);
    INFO("map_free");
    map_destroy(sockfd);
    INFO("map_destroy");
}

void test_func2(int sockfd){
    item_t *key, *value;
    key = (item_t*)mempool_alloc(mp, sizeof(item_t));
    memset(key, 0, sizeof(item_t));
    key->data = "mykey111";
    key->len = strlen(key->data) + 1;

    item_t *v = (item_t*)mempool_alloc(mp, sizeof(item_t));
    memset(v, 0, sizeof(item_t));
    reply_t reply = map_get(sockfd, key, (item_t**)&v);
    if (reply.errno == MAP_OK){
        INFO("map_get, reply.errno=%s, key=%s, reple.value=%s", status_name[reply.errno], key->data, v->data);
        mempool_free(mp, v->data);
    }else{
        INFO("map_get, reply.errno=%s, key=%s", status_name[reply.errno], key->data);
    }
    mempool_free(mp, v);
    mempool_free(mp, key);

    int i;
    char *key_prefix = "my-key", *value_prefix = "my-value", data1[1024], data2[1024];
    for (i = 0; i < num_data; ++i){
        key = (item_t*)mempool_alloc(mp, sizeof(item_t));
        value = (item_t*)mempool_alloc(mp, sizeof(item_t));
        memset(key, 0, sizeof(item_t));
        memset(value, 0, sizeof(item_t));
        snprintf(data1, 1024, "%s-%d", key_prefix, i);
        key->len = strlen(data1) + 1;
        key->data = data1;
        // printf("key_len=%d, key_data=%s\n", key->len, key->data);
        snprintf(data2, 1024, "%s-%d", value_prefix, i);
        value->len = strlen(data2) + 1;
        value->data = data2;
        // printf("value_len=%d, value_data=%s\n", value->len, value->data);

        reply = map_put(sockfd, key, value);
        INFO("map_put[%d], reply.errno=%s, key=%s, value=%s", i, status_name[reply.errno], key->data, value->data);
        mempool_free(mp, key);
        mempool_free(mp, value);
    }
    // print_mempool(mp);
    for (i = 0; i < num_data; ++i){
        key = (item_t*)mempool_alloc(mp, sizeof(item_t));
        value = (item_t*)mempool_alloc(mp, sizeof(item_t));
        memset(key, 0, sizeof(item_t));
        memset(value, 0, sizeof(item_t));
        snprintf(data1, 1024, "%s-%d", key_prefix, i);
        key->len = strlen(data1) + 1;
        key->data = data1;
        snprintf(data2, 1024, "%s-%d", value_prefix, i);
        value->len = strlen(data2) + 1;
        value->data = data2;

        reply = map_get(sockfd, key, (item_t**)&v);
        if (reply.errno == MAP_OK){
            INFO("map_get[%d], reply.errno=%s, key=%s, reple.value=%s", i, status_name[reply.errno], key->data, v->data);
            mempool_free(mp, v->data);
        }else{
            INFO("map_get[%d], reply.errno=%s, key=%s", i, status_name[reply.errno], key->data);
        }
        mempool_free(mp, key);
        mempool_free(mp, value);
    }
    // print_mempool(mp);
}

void test_func1(int sockfd){
    item_t *key, *value;
    // key = (item_t*)malloc(sizeof(item_t));
    key = (item_t*)mempool_alloc(mp, sizeof(item_t));
    memset(key, 0, sizeof(item_t));
    // value = (item_t*)malloc(sizeof(item_t));
    value = (item_t*)mempool_alloc(mp, sizeof(item_t));
    memset(value, 0, sizeof(item_t));
    key->data = "mykey111";
    key->len = strlen(key->data) + 1;
    value->data = "mydata111";
    value->len = strlen(value->data) + 1;
    
    reply_t reply = map_put(sockfd, key, value);
    INFO("map_put, reply.errno=%s, key=%s, value=%s", status_name[reply.errno], key->data, value->data);
    // item_t *v = (item_t*)malloc(sizeof(item_t));
    item_t *v = (item_t*)mempool_alloc(mp, sizeof(item_t));
    memset(v, 0, sizeof(item_t));
    reply = map_get(sockfd, key, (item_t**)&v);
    if (reply.errno == MAP_OK){
        INFO("map_get, reply.errno=%s, key=%s, reple.value=%s", status_name[reply.errno], key->data, v->data);
        // free(v->data);
        mempool_free(mp, v->data);
    }else{
        INFO("map_get, reply.errno=%s, key=%s", status_name[reply.errno], key->data);
    }
    key->data = "mykey222";
    key->len = strlen(key->data) + 1;
    value->data = "mydata222";
    value->len = strlen(value->data) + 1;

    reply = map_put(sockfd, key, value);
    INFO("map_put, reply.errno=%s, key=%s, value=%s", status_name[reply.errno], key->data, value->data);
    reply = map_get(sockfd, key, (item_t**)&v);
    if (reply.errno == MAP_OK){
        INFO("map_get, reply.errno=%s, key=%s, reple.value=%s", status_name[reply.errno], key->data, v->data);
        // free(v->data);
        mempool_free(mp, v->data);
    }else{
        INFO("map_get, reply.errno=%s, key=%s", status_name[reply.errno], key->data);
    }
    value->data = "mydata333";
    reply = map_put(sockfd, key, value);
    INFO("map_put, reply.errno=%s, key=%s, value=%s", status_name[reply.errno], key->data, value->data);

    INFO("map size, reply.len=%lu", map_size(sockfd));
    int errno = map_remove(sockfd, key);
    INFO("map remove, reply.errno=%s, key=%s", status_name[errno], key->data);
    
    INFO("map size, reply.len=%lu", map_size(sockfd));
    reply = map_get(sockfd, key, (item_t**)&v);
    if (reply.errno == MAP_OK){
        INFO("map_get, reply.errno=%s, key=%s, reple.value=%s", status_name[reply.errno], key->data, v->data);
        // free(v->data);
        mempool_free(mp, v->data);
    }else {
        INFO("map_get, reply.errno=%s, key=%s", status_name[reply.errno], key->data);
    }
    errno = map_remove(sockfd, key);
    INFO("map remove, reply.errno=%s, key=%s", status_name[errno], key->data);

    // free(v);
    mempool_free(mp, v);
    // free(key);
    mempool_free(mp, key);
    // free(value);
    mempool_free(mp, value);
}

int connect_setup(){
    int socket_fd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("create socket failed");
        exit(EXIT_FAILURE);
    }
    // bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if((server = gethostbyname(ip)) == NULL){
        perror("server is NULL");
        exit(EXIT_FAILURE);
    }
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    if(connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect failed");
        exit(EXIT_FAILURE);
    }
    printf("connected to %s:%d\n", ip, port);
    return socket_fd;
}

void usage(char *program){
    printf("Usage: \n");
    printf("%s\tconnect to %s:%d\n", program, ip, port);
    printf("Options:\n");
    printf(" -s <server>                        connect to server address(default %s)\n", DEFAULT_SERVER);
    printf(" -p <port>                          connect to server port(default %d)\n", DEFAULT_PORT);
    printf(" -m <max-mempool-size(GB/MB/KB)>    maximum memory pool size(default %d)\n", MAX_MEMPOOL_SIZE);
    printf(" -e <each-chunk-size(GB/MB/KB)>     each chunk size(default %d)\n", EACH_CHUNK_SIZE);
    printf(" -n <num-data>                      the number of put data\n");
    printf(" -h                                 display the help information\n");
}

void parse_args(int argc, char *argv[]){
    for (int i = 1; i < argc; ++i){
        if (strlen(argv[i]) == 2 && strcmp(argv[i], "-p") == 0){
            if (i+1 < argc){
                port = atoi(argv[i+1]);
                if (port < 0 || port > 65535){
                    printf("invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                i++;
            }else {
                printf("cannot read port number\n");
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-s") == 0){
            if(i+1 < argc){
                ip = argv[i+1];
                // TODO: check the validation of th ip address
                i++;
            }else {
                printf("cannot read ip address\n");
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-m") == 0){
            if (i+1 < argc){
                char str[128], *p;
                max_mempool_size = strtoul(argv[i+1], &p, 10);
                memcpy(str, p, strlen(p) + 1);
                trim(str);
                strupr(str);
                if (strcmp(str, "GB") == 0){
                    max_mempool_size *= GB;
                }else if (strcmp(str, "MB") == 0){
                    max_mempool_size *= MB;
                }else if (strcmp(str, "KB") == 0){
                    max_mempool_size *= KB;
                }
                if (max_mempool_size <=0 || max_mempool_size > 64 * GB){
                    printf("invalid max mempool size\n");
                    exit(EXIT_FAILURE);
                }
                i++;
            }else {
                printf("cannot read max mempool size\n");
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-e") == 0){
            if (i+1 < argc){
                char str[128], *p;
                each_chunk_size = strtoul(argv[i+1], &p, 10);
                memcpy(str, p, strlen(p) + 1);
                trim(str);
                strupr(str);
                if (strcmp(str, "GB") == 0){
                    each_chunk_size *= GB;
                }else if (strcmp(str, "MB") == 0){
                    each_chunk_size *= MB;
                }else if (strcmp(str, "KB") == 0){
                    each_chunk_size *= KB;
                }
                if (each_chunk_size <=0 || each_chunk_size > 64 * GB){
                    printf("invalid each chunk size\n");
                    exit(EXIT_FAILURE);
                }
                i++;
            }else {
                printf("cannot read each chunk size\n");
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-n") == 0){
            if (i+1 < argc){
                num_data = atoi(argv[i+1]);
                if (num_data <= 0 || num_data > INT32_MAX){
                    printf("invalid number of data\n");
                    exit(EXIT_FAILURE);
                }
                i++;
            }else {
                printf("cannot read number of data\n");
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-h") == 0){
            usage(argv[0]);
            exit(EXIT_SUCCESS);
        }else {
            printf("invalid option: %s\n", argv[i]);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}