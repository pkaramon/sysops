#include "socket_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int init_ipv4_address(struct sockaddr_in* address, const char* ipv4, int port)
{
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    if (ipv4 == NULL) {
        address->sin_addr.s_addr = INADDR_ANY;
        return 0;
    }

    if (inet_pton(AF_INET, ipv4, &address->sin_addr) <= 0) {
        perror("inet_pton");
        return -1;
    }

    return 0;
}

int create_ipv4_socket() { return socket(AF_INET, SOCK_STREAM, 0); }

void init_message_t(message_t* message, const char* sender, message_type_t type,
                    const char* message_text)

{
    message->timestamp = time(NULL);
    memcpy(message->sender, sender, MAX_CLIENT_NAME);
    message->type = type;
    memcpy(message->message, message_text, MAX_MESSAGE_TEXT);
}

int send_message(int socket, message_t* message)
{
    char buffer[MAX_MSG_NET_BUF];
    serialize_message_t(message, buffer);

    if (send(socket, buffer, MAX_MSG_NET_BUF, 0) == -1) {
        return -1;
    }
    return 0;
}

int receive_message(int socket, message_t* message)
{
    char buffer[MAX_MSG_NET_BUF];
    int received = recv(socket, buffer, MAX_MSG_NET_BUF, 0);
    if (received == -1) {
        perror("recv");
        return -1;
    }
    deserialize_message_t(message, buffer);
    return 0;
}

void serialize_message_t(message_t* input, char* buffer)
{
    memcpy(buffer, input->sender, MAX_CLIENT_NAME);
    buffer += MAX_CLIENT_NAME;

    uint32_t net_type = htonl((uint32_t)input->type);
    memcpy(buffer, &net_type, sizeof(uint32_t));
    buffer += sizeof(uint32_t);

    memcpy(buffer, input->message, MAX_MESSAGE_TEXT);

    buffer += MAX_MESSAGE_TEXT;

    time_t net_timestamp = htonl((uint32_t)input->timestamp);
    memcpy(buffer, &net_timestamp, sizeof(time_t));
}

void deserialize_message_t(message_t* output, char* buffer)
{
    memcpy(output->sender, buffer, MAX_CLIENT_NAME);
    buffer += MAX_CLIENT_NAME;

    uint32_t net_type;
    memcpy(&net_type, buffer, sizeof(uint32_t));
    output->type = (message_type_t)ntohl(net_type);
    buffer += sizeof(uint32_t);

    memcpy(output->message, buffer, MAX_MESSAGE_TEXT);

    buffer += MAX_MESSAGE_TEXT;

    time_t net_timestamp;
    memcpy(&net_timestamp, buffer, sizeof(time_t));
    output->timestamp = (time_t)ntohl(net_timestamp);
}
