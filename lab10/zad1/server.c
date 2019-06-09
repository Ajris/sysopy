#include "common.h"

#define INPUT_BUFFER_SIZE 256

int webSocket;
int localSocket;
int epoll;
char *localPath;
int id = 0;
pthread_t ping;
pthread_t command;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Client clients[CLIENT_MAX];
int clientsAmount = 0;
int currentClient = 0;

size_t  getFileSize(char* filename);

size_t readFile(char *filename, char *buff);

void handleSignal(int);

void init(char *, char *);

void handleMessage(int);

void registerClient(int socket, Message msg, struct sockaddr *sockaddr, socklen_t socklen);

void unregisterClient(char *);

void clean();

void *pingClients(void *);

void *handlerTerminal(void *);

void deleteClient(int);

void addHandlers();

void bindWebSocket(uint16_t portNum);

void bindLocalSocket();

void doEpoll();

void createThreads();

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 3)
        printError("Provide: port | path\n");
    atexit(clean);
    addHandlers();

    init(argv[1], argv[2]);

    struct epoll_event event;
    while (1) {
        if (epoll_wait(epoll, &event, 1, -1) == -1)
            printError("EPOLL WAIT FAILED\n");
        handleMessage(event.data.fd);
    }
}

size_t getFileSize(char *filename) {
    int fd;
    if ((fd = open(filename, O_RDONLY)) == -1) {
        printError("Couldnt open");
    }
    struct stat stats;
    fstat(fd, &stats);
    size_t size = (size_t) stats.st_size;
    close(fd);
    return size;
}

size_t readFile(char *filename, char *buff) {
    size_t size = getFileSize(filename);
    if (size == -1) {
        return size;
    }
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Coudlnt open file\n");
        return -1;
    }
    size_t read_size;
    if ((read_size = fread(buff, sizeof(char), size, file)) != size) {
        fprintf(stderr, "Coudlnt read file\n");
        return -1;
    }
    fclose(file);
    return read_size;
}

int in(void *const a, void *const pbase, size_t totalElems, size_t size, __compar_fn_t cmp) {
    char *base_ptr = (char *) pbase;
    if (totalElems > 0) {
        for (int i = 0; i < totalElems; ++i) {
            if ((*cmp)(a, (void *) (base_ptr + i * size)) == 0) return i;
        }
    }
    return -1;
}

int compareName(char *name, Client *client) {
    return strcmp(name, client->name);
}

void *pingClients(void *arg) {
    uint8_t message_type = PING;
    printf("PING\n");
    while (1) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < clientsAmount; ++i) {
            if (clients[i].activeCounter != 0) {
                printf("Client %s dont respond, removing\n", clients[i].name);
                deleteClient(i--);
            } else {
                int socket = clients[i].connectionType == WEB ? webSocket : localSocket;
                if (sendto(socket, &message_type, 1, 0, clients[i].sockaddr, clients[i].socklen) != 1)
                    printError("Could not PING client");
                clients[i].activeCounter++;
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(5);
    }
}

void *handlerTerminal(void *arg) {
    Request req;
    while (1) {
        char buffer[INPUT_BUFFER_SIZE];
        printf("FILE: \n");
        fgets(buffer, INPUT_BUFFER_SIZE, stdin);
        memset(req.text, 0, sizeof(req.text));
        sscanf(buffer, "%s", buffer);
        uint8_t messageType = REQUEST;
        int status = readFile(buffer, req.text);
        if (strlen(req.text) <= 0) {
            printf("Cannot send empty file\n");
            continue;
        }
        if (status < 0) {
            printf("WRONG FILE\n");
            continue;
        }
        id++;
        printf("REQUEST ID: %d\n", id);
        req.ID = id;
        int i = currentClient%clientsAmount;
        currentClient++;
        clients[i].reserved++;
        printf("Request sent -> %s\n", clients[i].name);
        int socket = clients[i].connectionType == WEB ? webSocket : localSocket;
        if (sendto(socket, &messageType, 1, 0, clients[i].sockaddr, clients[i].socklen) != 1) {
            printError("Cannot send");
        }
        if (sendto(socket, &req, sizeof(Request), 0, clients[i].sockaddr, clients[i].socklen) !=
            sizeof(Request)) {
            printError("Cannot send");
        }
    }
}

void handleMessage(int socket) {
    struct sockaddr *sockaddr = malloc(sizeof(struct sockaddr));
    socklen_t socklen = sizeof(struct sockaddr);
    Message msg;

    if (recvfrom(socket, &msg, sizeof(Message), 0, sockaddr, &socklen) != sizeof(Message))
        printError("Could not receive new message");
    switch (msg.messageType) {
        case REGISTER: {
            registerClient(socket, msg, sockaddr, socklen);
            break;
        }
        case UNREGISTER: {
            unregisterClient(msg.name);
            break;
        }
        case RESULT: {
            for (int i = 0; i < clientsAmount; i++) {
                if (strcmp(clients[i].name, msg.name) == 0) {
                    clients[i].activeCounter = 0;
                }
            }
            printf("RESULT: %s \n", msg.value);
            printf("FROM: %s \n", msg.name);
            break;
        }
        case PONG: {
            pthread_mutex_lock(&mutex);
            int i = in(msg.name, clients, (size_t) clientsAmount, sizeof(Client), (__compar_fn_t) compareName);
            if (i >= 0)
                clients[i].activeCounter = clients[i].activeCounter == 0 ? 0 : clients[i].activeCounter - 1;
            pthread_mutex_unlock(&mutex);
            break;
        }
        default:
            break;
    }
}

void registerClient(int socket, Message msg, struct sockaddr *sockaddr, socklen_t socklen) {
    uint8_t messageType;
    pthread_mutex_lock(&mutex);
    if (clientsAmount == CLIENT_MAX) {
        messageType = FAILSIZE;
        if (sendto(socket, &messageType, 1, 0, sockaddr, socklen) != 1)
            printError("Could not write FAILSIZE message");
        free(sockaddr);
    } else {
        int exists = in(msg.name, clients, (size_t) clientsAmount, sizeof(Client), (__compar_fn_t) compareName);
        if (exists != -1) {
            messageType = WRONGNAME;
            if (sendto(socket, &messageType, 1, 0, sockaddr, socklen) != 1)
                printError(" Could not write WRONGNAME");
            free(sockaddr);
        } else {
            printf("REGISTERING...\n");
            clients[clientsAmount].name = malloc(strlen(msg.name) + 1);
            clients[clientsAmount].connectionType = msg.connectionType;
            clients[clientsAmount].activeCounter = 0;
            clients[clientsAmount].reserved = 0;
            clients[clientsAmount].socklen = socklen;
            clients[clientsAmount].sockaddr = malloc(sizeof(struct sockaddr_un));
            memcpy(clients[clientsAmount].sockaddr, sockaddr, socklen);
            messageType = SUCCESS;

            strcpy(clients[clientsAmount++].name, msg.name);
            if (sendto(socket, &messageType, 1, 0, sockaddr, socklen) != 1)
                printError(" Could not write SUCCESS message to client \"%s\"\n");
            printf("SUCCESS \n");
        }
    }
    pthread_mutex_unlock(&mutex);
}

void unregisterClient(char *name) {
    pthread_mutex_lock(&mutex);
    int i = in(name, clients, (size_t) clientsAmount, sizeof(Client), (__compar_fn_t) compareName);
    if (i >= 0) {
        deleteClient(i);
        printf("Client: %s unregistered\n", name);
    }
    pthread_mutex_unlock(&mutex);
}

void deleteClient(int i) {
    free(clients[i].sockaddr);
    free(clients[i].name);
    clientsAmount--;
    for (int j = i; j < clientsAmount; j++)
        clients[j] = clients[j + 1];
}

void handleSignal(int signo) {
    printf("SIGINT\n");
    exit(1);
}

void addHandlers(){
    struct sigaction act;
    act.sa_handler = handleSignal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
}

void init(char *port, char *path) {
    for (int i = 0; i < CLIENT_MAX; i++) {
        clients[i].reserved = -1;
    }
    localPath = path;
    bindWebSocket(atoi(port));
    bindLocalSocket();
    doEpoll();
    createThreads();
}

void createThreads() {
    if (pthread_create(&ping, NULL, pingClients, NULL) != 0)
        printError("Could not create ping thread");
    if (pthread_create(&command, NULL, handlerTerminal, NULL) != 0)
        printError("Could not create terminal");
}

void doEpoll() {
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    if ((epoll = epoll_create1(0)) == -1)
        printError("Could not create epoll");
    event.data.fd = webSocket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, webSocket, &event) == -1)
        printError("Could not add web to epoll");
    event.data.fd = localSocket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, localSocket, &event) == -1)
        printError("Could not add local to epoll");
}

void bindLocalSocket() {
    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    sprintf(address.sun_path, "%s", localPath);

    if ((localSocket = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
        printError("Couldnt create local socket");

    if (bind(localSocket, (const struct sockaddr *) &address, sizeof(address)))
        printError("Couldnt bind local socket");
}

void bindWebSocket(uint16_t portNum) {
    struct sockaddr_in web_address;
    memset(&web_address, 0, sizeof(struct sockaddr_in));
    web_address.sin_family = AF_INET;
    web_address.sin_addr.s_addr = htonl(INADDR_ANY);
    web_address.sin_port = htons(portNum);

    if ((webSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        printError("Coudlnt create web socket");

    if (bind(webSocket, (const struct sockaddr *) &web_address, sizeof(web_address)))
        printError("Coudlnt create local bind");
}

void clean() {
    pthread_cancel(ping);
    pthread_cancel(command);
    for (int i = 0; i < clientsAmount; i++) {
        if (clients[i].reserved >= 0) {
            uint8_t messageType = END;
            int socket = clients[i].connectionType == WEB ? webSocket : localSocket;
            if (sendto(socket, &messageType, 1, 0, clients[i].sockaddr, clients[i].socklen) != 1) {
                printError("Coudlnt send");
            }
        }
    }

    if (close(webSocket) == -1)
        fprintf(stderr, " Could not close Web Socket\n");
    if (close(localSocket) == -1)
        fprintf(stderr, " Could not close Local Socket\n");
    if (unlink(localPath) == -1)
        fprintf(stderr, " Could not unlink Unix Path\n");
    if (close(epoll) == -1)
        fprintf(stderr, " Could not close epoll\n");
}