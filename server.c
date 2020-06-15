#include <stdio.h>
#include <stdlib.h>
#include "server_api.h"
#include "mempool.h"

// DEBUG_SET_LEVEL(DEBUG_LEVEL_DEBUG);

char *ip = DEFAULT_SERVER;
int port = DEFAULT_PORT;

void usage(char *program);
void parse_args(int argc, char *argv[]);
int connect_setup();
void *handle_connection(void* ptr);

int main(int argc, char *argv[]){
    parse_args(argc, argv);
    int listen_fd = connect_setup(), sockfd;

    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    if((sockfd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len)) < 0){
        perror("accept failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }
    mp = mempool_init(EACH_CHUNK_SIZE, MAX_MEMPOOL_SIZE);
    handle_connection((void*)&sockfd);
    mempool_clear(mp);
    mempool_destroy(mp);
    close(listen_fd);
    return 0;
}

void *handle_connection(void* ptr){
    int sockfd = *(int*)ptr;
    map_t map = new_hashmap(0, NULL);
    while (1){
        message_t in;
        int n = read(sockfd, &in, sizeof(message_t));
        if (n <= 0)
            continue;
        // INFO("read %d bytes", n);
        switch (in.cmd)
        {
        case CMD_PUT:
            INFO("receive `PUT` command");
            cmd_put(sockfd, map, &in);
            break;
        case CMD_GET:
            INFO("receive `GET` command");
            cmd_get(sockfd, map, &in);
            break;
        case CMD_REMOVE:
            INFO("receive `REMOVE` command");
            cmd_remove(sockfd, map, &in);
            break;
        case CMD_LEN:
            INFO("receive `LEN` command");
            cmd_len(sockfd, map);
            break;
        case CMD_FREE:
            INFO("receive `FREE` command");
            cmd_free(map);
            break;
        case CMD_DESTROY:
            INFO("receive `DESTROY` command");
            cmd_destroy(map);
            return (void*)0;
        default:
            break;
        }
    }
    
}

int connect_setup(){
    int listen_fd, sockfd;
    struct sockaddr_in  address;

    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("create socket failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);
    INFO("bind to %s:%d", ip, port);
    if(bind(listen_fd, (struct sockaddr *)&address, sizeof(address))){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if(listen(listen_fd, SOCK_BACKLOG_CONN) < 0){
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    INFO("server listens on 0.0.0.0:%d", port);
    return listen_fd;
}

void usage(char *program){
    printf("Usage: \n");
    printf("%s\tstart server 0.0.0.0:%d\n", program, port);
    printf("Options:\n");
    printf(" -s <server>    bind to server address(default %s)\n", DEFAULT_SERVER);
    printf(" -p <port>      bind to server port(default %d)\n", DEFAULT_PORT);
    printf(" -h             display the help information\n");
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