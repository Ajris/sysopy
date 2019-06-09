#include "common.h"

char *name;
enum ConnectionType connectType;
int clientSocket;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t requestMutex = PTHREAD_MUTEX_INITIALIZER;
int currSignal = 0;

void initConnections(char *connectionType, char *serverIpPath, char *port);

void sendMessage(uint8_t messageType, char *text);

void connectWEB(char *port);

void connectLOCAL(char *serverIpPath);

void handleMessage();

void clean();

void registerOnServer();

void printError(char *);

void addHandlers();

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 5) {
        printError("Wrong num of arguments");
    }
    name = argv[1];
    char *connectionType = argv[2];
    char *serverIp = argv[3];
    char *port = NULL;
    if (argc == 5) {
        port = argv[4];
    }

    atexit(clean);
    addHandlers();
    initConnections(connectionType, serverIp, port);
    registerOnServer();
    handleMessage();
    return 0;
}

void *handleTask(void *arg) {
    printf("Working\n");

    struct Request *arguments = arg;
    struct Request currentRequest;
    strcpy(currentRequest.text, arguments->text);
    currentRequest.ID = arguments->ID;

    char *buffer = malloc(100 + 2 * strlen(currentRequest.text));
    char *text = malloc(100 + 2 * strlen(currentRequest.text));

    sprintf(buffer, "echo '%s' | awk '{for(x=1;$x;++x)print $x}' | sort | uniq -c", (char *) currentRequest.text);
    FILE *result = popen(buffer, "r");

    int n = fread(buffer, 1, 99 + 2 * strlen(currentRequest.text), result);
    buffer[n] = '\0';

    int words_count = 1;
    char *res = strtok(currentRequest.text, " ");
    while (strtok(NULL, " ") && res) {
        words_count++;
    }

    sprintf(text, "ID: %d\tSUM: %d\tWORDS: \n%s", currentRequest.ID, words_count, buffer);

    pthread_mutex_lock(&requestMutex);
    sendMessage(RESULT, text);
    pthread_mutex_unlock(&requestMutex);
    free(buffer);
    free(text);
    return NULL;
}

void sendMessage(uint8_t messageType, char *text) {
    Message msg;
    msg.messageType = messageType;
    snprintf(msg.name, 64, "%s", name);
    msg.connectionType = connectType;
    if (text) {
        snprintf(msg.value, strlen(text), "%s", text);
    }
    pthread_mutex_lock(&mutex);
    if (write(clientSocket, &msg, sizeof(Message)) != sizeof(Message))
        printError("Could not send message");
    pthread_mutex_unlock(&mutex);
}

void registerOnServer() {
    sendMessage(REGISTER, NULL);

    uint8_t messageType;
    if (read(clientSocket, &messageType, 1) != 1)
        printError("Could not read response message");

    switch (messageType) {
        case WRONGNAME:
            printError("Name used");
        case FAILSIZE:
            printError("Too many clients");
        case SUCCESS:
            printf("SUCCESS\n");
            break;
        default:
            printError("Unpredicted message ???");
    }
}

void handleMessage() {
    uint8_t messageType;
    pthread_t thread;
    while (1) {
        if (read(clientSocket, &messageType, sizeof(uint8_t)) != sizeof(uint8_t))
            printError("Could not read message type");
        switch (messageType) {
            case REQUEST:
                printf("Got task\n");
                Request req;
                if (read(clientSocket, &req, sizeof(Request)) <= 0) {
                    printError("Cannot read");
                }
                pthread_create(&thread, NULL, handleTask, &req);
                pthread_detach(thread);
                break;
            case PING:
                pthread_mutex_lock(&requestMutex);
                sendMessage(PONG, NULL);
                pthread_mutex_unlock(&requestMutex);
                break;
            default:
                break;
        }
    }
}

void handleSignals(int signo) {
    currSignal++;
    exit(1);
}

void connectWEB(char *port) {
    connectType = WEB;
    uint16_t portNum = (uint16_t) atoi(port);
    if (portNum < 1024 || portNum > 65535) {
        printError("Wrong port");
    }
    struct sockaddr_in web_address;
    memset(&web_address, 0, sizeof(struct sockaddr_in));

    web_address.sin_family = AF_INET;
    web_address.sin_addr.s_addr = htonl(INADDR_ANY);
    web_address.sin_port = htons(portNum);

    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printError("Socket");
    }
    int yes = 1;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        printError("Setsockopt");
    }

    if (connect(clientSocket, (const struct sockaddr *) &web_address, sizeof(web_address)) == -1) {
        printError("Connect");
    }
}

void connectLOCAL(char *serverIpPath) {
    connectType = LOCAL;
    if ((clientSocket = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
        printError("Couldnt create local socket");


    struct sockaddr_un local_address_bind;
    local_address_bind.sun_family = AF_UNIX;
    snprintf(local_address_bind.sun_path, MAX_PATH, "%s", name);

    if (bind(clientSocket, (const struct sockaddr *) &local_address_bind, sizeof(local_address_bind)))
        printError("Bind");

    struct sockaddr_un local_address;
    local_address.sun_family = AF_UNIX;
    snprintf(local_address.sun_path, MAX_PATH, "%s", serverIpPath);

    if (connect(clientSocket, (const struct sockaddr *) &local_address, sizeof(local_address)) == -1)
        printError("Connect");
}

void addHandlers() {
    struct sigaction act;
    act.sa_handler = handleSignals;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
}

void initConnections(char *connectionType, char *serverIpPath, char *port) {
    if (strcmp("WEB", connectionType) == 0) {
        connectWEB(port);
    } else if (strcmp("LOCAL", connectionType) == 0) {
        connectLOCAL(serverIpPath);
    } else {
        printError("Wrong argument type");
    }
}

void clean() {
    unlink(name);
    if (currSignal > 0) {
        sendMessage(UNREGISTER, NULL);
    }
    if (close(clientSocket) == -1)
        printError("Couldnt close socket");
}
