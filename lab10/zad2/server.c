//
// Created by przjab98 on 31.05.19.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "common.h"

#define MAX_BUFFER_SIZE 256
size_t getFileSize(const char *file_name) {
    int fd;
    if ((fd = open(file_name, O_RDONLY)) == -1) {
        fprintf(stderr, "Unable to open file for size \n");
        return (size_t) -1;
    }
    struct stat stats;
    fstat(fd, &stats);
    size_t size = (size_t) stats.st_size;
    close(fd);
    return size;
}

size_t read_whole_file(const char *file_name, char *buffer) {
    size_t size = getFileSize(file_name);
    if (size == -1) {
        return size;
    }
    FILE *file;
    if ((file = fopen(file_name, "r")) == NULL) {
        fprintf(stderr, "Unable to open file \n");
        return (size_t) -1;
    }
    size_t read_size;
    if ((read_size = fread(buffer, sizeof(char), size, file)) != size) {
        fprintf(stderr, "Unable to read file\n");
        return (size_t) -1;
    }
    fclose(file);
    return read_size;
}

int currentClient = 0;

void handleSignal(int);

void init(char *, char *);

void handleMessage(int);

void register_client(char *, int);

void unregisterClient(char *);

void clean();

void *ping_routine(void *);

void *handle_terminal(void *);

void handle_connection(int);

void deleteClient(int);

void delete_socket(int);

int in(void *const a, void *const pbase, size_t total_elems, size_t size, __compar_fn_t cmp) {
    char *base_ptr = (char *) pbase;
    if (total_elems > 0) {
        for (int i = 0; i < total_elems; ++i) {
            if ((*cmp)(a, (void *) (base_ptr + i * size)) == 0) return i;
        }
    }
    return -1;
}

int compareName(char *name, Client *client) {
    return strcmp(name, client->name);
}

int webSocket;
int localSocket;
int epoll;
char *localPath;
int id;

pthread_t ping;
pthread_t command;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Client clients[CLIENT_MAX];
int clientsAmount = 0;


int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 3)
        raise_error("\nUsage: %s <port number> <unix path>\n");
    if (atexit(clean) == -1)
        raise_error(" Could not set AtExit\n");

    init(argv[1], argv[2]);

    struct epoll_event event;
    int x = 1;
    while (x) {
        if (epoll_wait(epoll, &event, 1, -1) == -1)
            raise_error(" epoll_wait failed\n");
        if (event.data.fd < 0)
            handle_connection(-event.data.fd);
        else
            handleMessage(event.data.fd);
    }


}


void *ping_routine(void *arg) {
    uint8_t message_type = PING;
    int x = 1;
    while (x) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < clientsAmount; ++i) {
            if (clients[i].activeCounter != 0) {
                printf("num: %d \n",clients[i].activeCounter );
                printf("Client \"%s\" do not respond. Removing from registered clients\n", clients[i].name);
                deleteClient(i--);
            } else {
                if (write(clients[i].fd, &message_type, 1) != 1)
                    raise_error(" Could not PING client");

                clients[i].activeCounter++;
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(10);
    }
    return NULL;
}

void send_msg(int type, int len, Request *req, int i) {
    if (write(clients[i].fd, &type, 1) != 1) {
        raise_error("cannot send");
    }
    if (write(clients[i].fd, &len, 2) != 2) {
        raise_error("cannot send");
    }
    if (write(clients[i].fd, req, len) != len) {
        raise_error("cannot send");
    }

}

void *handle_terminal(void *arg) {

    int true = 1;
    while (true) {
        char buffer[MAX_BUFFER_SIZE];
        memset(buffer,0, MAX_BUFFER_SIZE);
        printf("Enter command: \n");
        fgets( buffer,MAX_BUFFER_SIZE,stdin);
        char file_buffer[MAX_MSG_SIZE];
        memset( file_buffer, '\0', sizeof(char)*MAX_MSG_SIZE);
        sscanf(buffer, "%s", file_buffer);
        Request req;
        id++;
        printf("REQUEST ID: %d \n", id);
        printf("%s \n", file_buffer);
        memset(req.text, 0, sizeof(req.text));
        int status = read_whole_file(file_buffer, req.text);
        req.ID = id;
        if(strlen(file_buffer) <= 0){
            printf("cannot send empty file \n");
            continue;
        }
        if (status < 0) {
            printf("WRONG FILE \n");
            continue;
        }

        int i = 0;
        int min = 90000;
        int index = 0;
        for (i = 0; i < clientsAmount; i++) {
            if (clients[i].reserved  < min) {
                min = clients[i].reserved;
                index = i;
            }
        }
        i = index;

        i = currentClient%clientsAmount;
        currentClient++;

        printf("Request sent to %s \n", clients[i].name);
        clients[i].reserved++;
        send_msg(REQUEST, sizeof(req), &req, i);


    }
    return NULL;
}

void handle_connection(int socket) {
    int client = accept(socket, NULL, NULL);
    if (client == -1) raise_error(" Could not accept new client");

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    event.data.fd = client;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, client, &event) == -1)
        raise_error(" Could not add new client to epoll");

}

void handleMessage(int socket) {
    uint8_t message_type;
    uint16_t message_size;

    if (read(socket, &message_type, TYPE_SIZE) != TYPE_SIZE) raise_error(" Could not read message type\n");
    if (read(socket, &message_size, LEN_SIZE) != LEN_SIZE) raise_error(" Could not read message size\n");
    char *client_name = malloc(message_size);

    switch (message_type) {
        case REGISTER: {
            if (read(socket, client_name, message_size) != message_size)
                raise_error(" Could not read register message name\n");
            register_client(client_name, socket);
            break;
        }
        case UNREGISTER: {
            if (read(socket, client_name, message_size) != message_size)
                raise_error(" Could not read unregister message name\n");
            unregisterClient(client_name);
            break;
        }
        case RESULT: {
            printf("SERVER GOT RESULT \n");
            if (read(socket, client_name, message_size) < 0)
                raise_error("read res name");
            int size;
            if (read(socket, &size, sizeof(int)) < 0)
                raise_error("size of res");
            char* result = malloc(size);
            memset(result, 0, size);
            if (read(socket, result, size) < 0)
                raise_error("result");

            printf("Computations from: %s : %s \n",client_name, result);

            int i;
            for(i = 0; i < CLIENT_MAX; i++){
                if(clients[i].reserved > 0 && strcmp(client_name, clients[i].name) == 0){
//                    clients[i].reserved --;
                    clients[i].activeCounter = 0;
                    printf("Client %s is free now \n", client_name);
                }
            }
            free(result);
            break;
        }
        case PONG: {
            if (read(socket, client_name, message_size) != message_size)
                raise_error(" Could not read PONG message\n");
            pthread_mutex_lock(&mutex);
            int i = in(client_name, clients, (size_t) clientsAmount, sizeof(Client), (__compar_fn_t) compareName);
            if (i >= 0) clients[i].activeCounter = clients[i].activeCounter == 0 ? 0 : clients[i].activeCounter-1;
            pthread_mutex_unlock(&mutex);
            break;
        }
        default:
            printf("Unknown message type\n");
            break;
    }
    free(client_name);
}

void register_client(char *client_name, int socket) {
    uint8_t message_type;
    pthread_mutex_lock(&mutex);
    if (clientsAmount == CLIENT_MAX) {
        message_type = FAILSIZE;
        if (write(socket, &message_type, 1) != 1)
            raise_error(" Could not write FAILSIZE message to client \"%s\"\n");
        delete_socket(socket);
    } else {
        int exists = in(client_name, clients, (size_t) clientsAmount, sizeof(Client), (__compar_fn_t) compareName);
        if (exists != -1) {
            message_type = WRONGNAME;
            if (write(socket, &message_type, 1) != 1)
                raise_error(" Could not write WRONGNAME message to client \"%s\"\n");
            delete_socket(socket);
        } else {
            clients[clientsAmount].fd = socket;
            clients[clientsAmount].name = malloc(strlen(client_name) + 1);
            clients[clientsAmount].activeCounter = 0;
            clients[clientsAmount].reserved = 0;
            strcpy(clients[clientsAmount++].name, client_name);
            message_type = SUCCESS;
            if (write(socket, &message_type, 1) != 1)
                raise_error(" Could not write SUCCESS message to client \"%s\"\n");
        }
    }
    pthread_mutex_unlock(&mutex);
}

void unregisterClient(char *client_name) {
    pthread_mutex_lock(&mutex);
    int i = in(client_name, clients, (size_t) clientsAmount, sizeof(Client), (__compar_fn_t) compareName);
    if (i >= 0) {
        deleteClient(i);
        printf("Client \"%s\" unregistered\n", client_name);
    }
    pthread_mutex_unlock(&mutex);
}

void deleteClient(int i) {
    delete_socket(clients[i].fd);
    free(clients[i].name);
    clientsAmount--;
    for (int j = i; j < clientsAmount; ++j)
        clients[j] = clients[j + 1];

}

void delete_socket(int socket) {
    if (epoll_ctl(epoll, EPOLL_CTL_DEL, socket, NULL) == -1)
        raise_error(" Could not remove client's socket from epoll\n");

    if (shutdown(socket, SHUT_RDWR) == -1) raise_error(" Could not shutdown client's socket\n");

    if (close(socket) == -1) raise_error(" Could not close client's socket\n");
}

void handleSignal(int signo) {
    printf("\nSIGINT\n");
    exit(1);
}

void init(char *port, char *path) {

    struct sigaction act;
    act.sa_handler = handleSignal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    int i;
    for (i = 0; i < CLIENT_MAX; i++) {
        clients[i].reserved = -1;
    }

    uint16_t port_num = (uint16_t) atoi(port);
    localPath = path;

    struct sockaddr_in web_address;
    memset(&web_address, 0, sizeof(struct sockaddr_in));
    web_address.sin_family = AF_INET;
    web_address.sin_addr.s_addr =htonl(INADDR_ANY); //inet_addr("path"); //inet_addr("192.168.0.66"); //htonl(INADDR_ANY);
    web_address.sin_port = htons(port_num);

    if ((webSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        raise_error(" Could not create web socket\n");

    int yes = 1;
    if (setsockopt(webSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    if (bind(webSocket, (const struct sockaddr *) &web_address, sizeof(web_address)))
        raise_error(" Could not bind web socket\n");

    if (listen(webSocket, 64) == -1)
        raise_error(" Could not listen to web socket\n");

    struct sockaddr_un local_address;
    local_address.sun_family = AF_UNIX;

    sprintf(local_address.sun_path, "%s", localPath);

    if ((localSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        raise_error(" Could not create local socket\n");
    if (bind(localSocket, (const struct sockaddr *) &local_address, sizeof(local_address)))
        raise_error(" Could not bind local socket\n");
    if (listen(localSocket, 64) == -1)
        raise_error(" Could not listen to local socket\n");

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    if ((epoll = epoll_create1(0)) == -1)
        raise_error(" Could not create epoll\n");
    event.data.fd = -webSocket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, webSocket, &event) == -1)
        raise_error(" Could not add Web Socket to epoll\n");
    event.data.fd = -localSocket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, localSocket, &event) == -1)
        raise_error(" Could not add Local Socket to epoll\n");


    if (pthread_create(&ping, NULL, ping_routine, NULL) != 0)
        raise_error(" Could not create Pinger Thread");
    if (pthread_create(&command, NULL, handle_terminal, NULL) != 0)
        raise_error(" Could not create Commander Thread");
}

void clean() {
    printf("CLEANUP \n");
    pthread_cancel(ping);
    pthread_cancel(command);
    if (close(webSocket) == -1)
        fprintf(stderr, " Could not close Web Socket\n");
    if (close(localSocket) == -1)
        fprintf(stderr, " Could not close Local Socket\n");
    if (unlink(localPath) == -1)
        fprintf(stderr, " Could not unlink Unix Path\n");
    if (close(epoll) == -1)
        fprintf(stderr, " Could not close epoll\n");
}