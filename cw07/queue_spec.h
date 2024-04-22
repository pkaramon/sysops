#pragma once

#define SERVER_QUEUE_NAME "/chat-server-queue"
#define CLIENT_QUEUE_NAME_FORMAT "/chat-client-queue-%d"
#define MAX_CLIENTS 50
#define MAX_MSG_BUF_SIZE 1024
#define MAX_MESSAGES 10


typedef enum { INIT, RECEIVE_ID, TEXT } msg_type_t;

typedef struct {
    msg_type_t type;
    int id;
    char text[MAX_MSG_BUF_SIZE];
} msg_t;
