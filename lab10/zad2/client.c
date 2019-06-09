#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include "common.h"
#include <netinet/in.h>
#include <sys/un.h>
#include <endian.h>
#include <arpa/inet.h>
#include <pthread.h>


char *name;
int clientSocket;
int ID = 0;

void initConnections(char *connectionType, char *serverIpPath, char *port);

void handleMessage();

void connectToServer();

void sendMessage(uint8_t messageType);

void clean();

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 5) {
        printError("Provide: name | connection type | socketName OR serverIP port");
    }

    name = argv[1];
    char *connection_type = argv[2];
    char *server_ip = argv[3];
    char *port = NULL;
    if (argc == 5) {
        port = argv[4];
    }
    initConnections(connection_type, server_ip, port);
    connectToServer();
    handleMessage();
    clean();
    return 0;
}

void *handleTask(void *arg) {
    Request *reqTmp = (Request *) arg;
    Request req;
    memset(req.text, 0, sizeof(req.text));
    strcpy(req.text, reqTmp->text);
    int id = reqTmp->ID;
    char *buffer = malloc(100 + 2 * strlen(req.text));
    char *buffer_res = malloc(100 + 2 * strlen(req.text));
    sprintf(buffer, "echo '%s' | awk '{for(x=1;$x;++x)print $x}' | sort | uniq -c", (char *) req.text);
    FILE *result = popen(buffer, "r");
    int n = fread(buffer, 1, 99 + 2 * strlen(req.text), result);
    buffer[n] = '\0';
    int words_count = 1;
    char *res = strtok(req.text, " ");
    while (strtok(NULL, " ") && res) {
        words_count++;
    }
    sprintf(buffer_res, "ID: %d sum: %d || %s", id, words_count, buffer);
    pthread_mutex_lock(&mutex);
    sendMessage(RESULT);
    int len = strlen(buffer_res);
    if (write(clientSocket, &len, sizeof(int)) != sizeof(int))
        printError(" Could not write message type");
    if (write(clientSocket, buffer_res, len) != len)
        printError(" Could not write message type");
    printf("RESULT SENT \n");
    free(buffer);
    free(buffer_res);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void sendMessage(uint8_t messageType) {
    uint16_t messageSize = (uint16_t) (strlen(name) + 1);
    if (write(clientSocket, &messageType, TYPE_SIZE) != TYPE_SIZE)
        printError(" Could not write message type");
    if (write(clientSocket, &messageSize, LEN_SIZE) != LEN_SIZE)
        printError(" Could not write message size");
    if (write(clientSocket, name, messageSize) != messageSize)
        printError(" Could not write name message");
}

void connectToServer() {
    sendMessage(REGISTER);
    uint8_t messageType;
    if (read(clientSocket, &messageType, 1) != 1) printError("\n Could not read response message type\n");
    switch (messageType) {
        case WRONGNAME:
            printError("Name already in use\n");
        case FAILSIZE:
            printError("Too many clients logged\n");
        case SUCCESS:
            printf("Logged\n");
            break;
        default:
            printError("Impossible \n");
    }
}

void handleMessage() {
    uint8_t messageType;
    pthread_t thread;
    while (1) {
        if (read(clientSocket, &messageType, TYPE_SIZE) != TYPE_SIZE)
            printError(" Could not read message type");
        switch (messageType) {
            case REQUEST:
                printf(" ");
                uint16_t req_len;
                if (read(clientSocket, &req_len, 2) <= 0) {
                    printError("cannot read length");
                }
                Request req;
                memset(req.text, '\0', sizeof(req.text));
                if (read(clientSocket, req.text, req_len) < 0) {
                    printError("cannot read whole text");
                }
                pthread_create(&thread, NULL, handleTask, &req);
                pthread_detach(thread);
                break;
            case PING:
                pthread_mutex_lock(&mutex);
                sendMessage(PONG);
                pthread_mutex_unlock(&mutex);
                break;
            default:
                break;
        }
    }
}

void handleSignals(int signo) {
    sendMessage(UNREGISTER);
    exit(1);
}

void initConnections(char *connectionType, char *serverIpPath, char *port) {
    struct sigaction act;
    act.sa_handler = handleSignals;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    if (strcmp("WEB", connectionType) == 0) {
        inet_addr(serverIpPath);
        uint16_t port_num = (uint16_t) atoi(port);
        if (port_num < 1024 || port_num > 65535) {
            printError("wrong port");
        }
        struct sockaddr_in web_address;
        memset(&web_address, 0, sizeof(struct sockaddr_in));
        web_address.sin_family = AF_INET;
        web_address.sin_addr.s_addr = htonl(
                INADDR_ANY); //inet_addr(serverIpPath);//inet_addr("192.168.0.66"); //htonl(ip);
        web_address.sin_port = htons(port_num);
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printError("socket");
        }
        if (connect(clientSocket, (const struct sockaddr *) &web_address, sizeof(web_address)) == -1) {
            printError("connect");
        }
        printf("CONNECTED TO WEB \n");
    } else if (strcmp("LOCAL", connectionType) == 0) {
        char *unix_path = serverIpPath;
        struct sockaddr_un local_address;
        local_address.sun_family = AF_UNIX;
        snprintf(local_address.sun_path, MAX_PATH, "%s", unix_path);
        if ((clientSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
            printError("\n Could not create local socket\n");
        if (connect(clientSocket, (const struct sockaddr *) &local_address, sizeof(local_address)) == -1)
            printError("\n Could not connect to local socket\n");
    } else {
        printError("wrong type of argument");
    }
}

void clean() {
    sendMessage(UNREGISTER);
    if (shutdown(clientSocket, SHUT_RDWR) == -1)
        fprintf(stderr, "\n Could not shutdown Socket\n");
    if (close(clientSocket) == -1)
        fprintf(stderr, "\n Could not close Socket\n");
}