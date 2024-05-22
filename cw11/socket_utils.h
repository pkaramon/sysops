#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>

#define MAX_CLIENT_NAME 64
#define MAX_MESSAGE_TEXT 512
#define MAX_MSG_NET_BUF 1024
#define MAX_CLIENTS 100

typedef enum {
    CLIENT_NEW,
    CLIENT_LIST,
    CLIENT_TO_ALL,
    CLIENT_TO_ONE,
    CLIENT_STOP,
    CLIENT_ALIVE,
    SERVER_OK_REGISTER,
    SERVER_FAILURE_REGISTER,
    SERVER_CHAT_MESSAGE,
    SERVER_LIST_MESSAGE,
    SERVER_CHECK_ALIVE,
    SERVER_SHUTDOWN
} message_type_t;

typedef struct {
    char sender[MAX_CLIENT_NAME];
    message_type_t type;
    char message[MAX_MESSAGE_TEXT];
    time_t timestamp;
} message_t;

int init_ipv4_address(struct sockaddr_in* address, char const* ipv4, int port);
int create_ipv4_socket();

void init_message_t(message_t* message, char const* sender, message_type_t type,
                    char const* message_text);
int send_message(int socket, message_t* message);
int receive_message(int socket, message_t* message);

void serialize_message_t(message_t* message, char* buffer);
void deserialize_message_t(message_t* message, char* buffer);