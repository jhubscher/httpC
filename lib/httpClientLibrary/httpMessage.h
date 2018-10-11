#ifndef HTTP_MESSAGE
#define HTTP_MESSAGE

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "http.h"

typedef struct http_message HttpMessage;
struct http_message {
    HTTP_MESSAGE_TYPES messageType;
    char* host;
    int port;
    int socketFileDescriptor;

    char* startLine;
    char* headers;
    char* messageBody;

    char* message;

    void (*constructHttpMessage)(HttpMessage*);
    int (*sendMessage)(HttpMessage*);
    HttpMessage* (*receiveMessage)();
};

void constructHttpMessage(HttpMessage* message);
void constructHttpMessage(HttpMessage* message) {
    // Start line.
    message->message = concat(message->message, message->startLine);

    message->message = concat(message->message, message->headers);
    message->message = concat(message->message, "\r\n");

    // Message body.
    message->message = concat(message->message, message->messageBody);
}

struct addrinfo* getHostInfo(HttpMessage* message);
struct addrinfo* getHostInfo(HttpMessage* message) {
    int status;
    struct addrinfo hints, *serverInfoResults, *resultsPtr; 

    void *address;
    char *IPVersion;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC; // IPv4 | IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // Fill in IP

    if ((status = getaddrinfo(message->host, "http", &hints, &serverInfoResults)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // serverInfoResults now points to a linked list of 1 or more struct addrinfo s.
    // Do everything until you don't need serverInfo anymore.
    resultsPtr = serverInfoResults;
    while(resultsPtr != NULL) {
        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (resultsPtr->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)resultsPtr->ai_addr;
            address = &(ipv4->sin_addr);
            IPVersion = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)resultsPtr->ai_addr;
            address = &(ipv6->sin6_addr);
            IPVersion = "IPv6";
        }

        resultsPtr = resultsPtr->ai_next;
    }

    return serverInfoResults;
}

void setSocketFileDescriptor(HttpMessage* message, struct addrinfo* serverInfoResults);
void setSocketFileDescriptor(HttpMessage* message, struct addrinfo* serverInfoResults) {
    message->socketFileDescriptor = socket(serverInfoResults->ai_family, serverInfoResults->ai_socktype, serverInfoResults->ai_protocol);
}

int canConnect(HttpMessage* message, struct addrinfo* serverInfoResults);
int canConnect(HttpMessage* message, struct addrinfo* serverInfoResults) {
    int connectResult;

    int tryConnecting = 0;
    while(tryConnecting < 100 && (connectResult = connect(message->socketFileDescriptor, serverInfoResults->ai_addr, serverInfoResults->ai_addrlen) == -1)) {
        tryConnecting = tryConnecting + 1;
    }

    if(connectResult == -1) {
        return 1;
    } else {
        // serverInfoResults now points to a linked list of 1 or more struct addrinfos.
        // Do everything until you don't need serverInfoResults anymore.
        freeaddrinfo(serverInfoResults);
        
        return 0;
    }
}

int sendMessage(HttpMessage* message);
int sendMessage(HttpMessage* message) {
    struct addrinfo *serverInfoResults = getHostInfo(message);
    setSocketFileDescriptor(message, serverInfoResults);

    int isConnected = canConnect(message, serverInfoResults);

    int messageLength = strlen(message->message);
    int tmpMessageLength = messageLength;
    int bytes_sent;

    while(tmpMessageLength > 0) {
        bytes_sent = send(message->socketFileDescriptor, message->message, tmpMessageLength, 0);
        tmpMessageLength = tmpMessageLength - bytes_sent;
    }
}

HttpMessage* receiveMessage();
HttpMessage* receiveMessage() {

}

#endif
