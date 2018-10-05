#ifndef HTTP_RESPONSE
#define HTTP_RESPONSE

#include "http.h"
#include "httpMessage.h"

typedef struct http_response HttpResponse;
struct http_response {
    // i.e. HTTP/${VERSION}
    char* protocolVersion;
    // i.e. 200
    char* statusCode;
    // i.e. OK
    char* reasonPhrase;

    char* statusLine;
    // Will be replaced by entity body given a "Transfer-Encoding" header.
    // All responses to the HEAD request method MUST NOT include a message-body,
    // even though the presence of entity- header fields might lead one to believe they do.
    // All 1xx (informational), 204 (no content), and 304 (not modified) responses MUST NOT include a message-body.
    // All other responses do include a message-body, although it MAY be of zero length. 
    char* responseBody;

    void (*constructStatusLine)(HttpResponse*);

    // i.e. `field-name:field-value`. "field-name"s should be case-insensitive
    char* headers[][2];
};

void constructStatusLine(HttpResponse* response);
void constructStatusLine(HttpResponse* response) {
    response->statusLine = '\0';

    if(strcmp(response->protocolVersion, "1.0") != 0 || strcmp(response->protocolVersion, "1.1") != 0) {
        fprintf(stderr, "EXCEPTION: httpResponse::constructStatusLine() - Invalid HTTP protocol version. Defaulting to 1.0.\n");
        response->protocolVersion = (char*)malloc(strlen("1.0") + 1);

        if(response->protocolVersion == NULL) {
            fprintf(stderr, "MEMORY ERROR: httpResponse::constructStatusLine() - Could not allocate memory for response protocol version.");
            exit(-1);
        }

        response->protocolVersion = "1.0";
    }

    response->statusLine = concat(response->statusLine, response->protocolVersion);
    response->statusLine = concat(response->statusLine, " ");

    int isStatusCodeValid = 0;
    for(int i = 0; i < HTTP_STATUS_CODES_SIZE; i = i + 1) {
        if(strcmp(response->statusCode, HTTP_STATUS_CODES[i]) == 0) {
            isStatusCodeValid = 1;
            break;
        }
    }

    if(!isStatusCodeValid) {
        fprintf(stderr, "EXCEPTION: httpResponse::constructStatusLine() - Invalid HTTP status code. Defaulting to 500.\n");
        response->statusCode = (char*)malloc(strlen("500") + 1);

        if(response->statusCode == NULL) {
            fprintf(stderr, "MEMORY ERROR: httpResponse::constructStatusLine() - Could not allocate memory for response status code.");
            exit(-1);
        }

        response->statusCode = "500";
    } 

    response->statusLine = concat(response->statusLine, response->statusCode);
    response->statusLine = concat(response->statusLine, " ");

    int isReasonPhraseValid = 0;
    for(int i = 0; i < HTTP_REASON_PHRASES_SIZE; i = i + 1) {
        if(strcmp(response->reasonPhrase, HTTP_REASON_PHRASES[i]) == 0) {
            isReasonPhraseValid = 1;
            break;
        }
    }

    if(!isReasonPhraseValid) {
        fprintf(stderr, "EXCEPTION: httpResponse::constructStatusLine() - Invalid HTTP reason phrase. Defaulting to \"Internal Server Error\".\n");
        response->reasonPhrase = (char*)malloc(strlen("Internal Server Error") + 1);
        
        if(response->reasonPhrase == NULL) {
            fprintf(stderr, "MEMORY ERROR: httpResponse::constructStatusLine() - Could not allocate memory for response reason phrase.");
            exit(-1);
        }
        
        response->reasonPhrase = "Internal Server Error";
    } 

    response->statusLine = concat(response->statusLine, response->reasonPhrase);
    response->statusLine = concat(response->statusLine, " ");

    response->statusLine = concat(response->statusLine, "\r\n");
}

HttpMessage* constructHttpMessageFromResponse(HttpResponse* response);
HttpMessage* constructHttpMessageFromResponse(HttpResponse* response) {
    HttpMessage message;
    HttpMessage* messagePtr = &message;
    messagePtr->messageType = RESPONSE;

    response->constructStatusLine(response);
    messagePtr->startLine = response->statusLine;
}

#endif
