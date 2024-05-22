#define _DEFAULT_SOURCE 1

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "socket_utils.h"

#define MAX_COMMAND_LENGTH 10

int socket_fd = -1;
char* client_id = NULL;

void clean_exit()
{
    message_t message;
    init_message_t(&message, client_id, CLIENT_STOP, "");

    if (send_message(socket_fd, &message) == -1) {
        fprintf(stderr, "failed to send message\n");
    }

    close(socket_fd);
    exit(EXIT_SUCCESS);
}

void sigint_handler(__attribute__((unused)) int signum) { clean_exit(); }

int connect_to_server(const char* server_ip, int server_port)
{
    struct sockaddr_in server_address;
    if (init_ipv4_address(&server_address, server_ip, server_port) == -1) {
        fprintf(stderr, "failed to create address\n");
        return -1;
    }

    socket_fd = create_ipv4_socket();
    if (socket_fd == -1) {
        perror("socket");
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("connect");
        return -1;
    }
    return 0;
}

int register_on_server()
{
    message_t new_client_request;
    init_message_t(&new_client_request, client_id, CLIENT_NEW, "");

    if (send_message(socket_fd, &new_client_request) == -1) {
        fprintf(stderr, "failed to send message\n");
        return -1;
    }

    message_t new_client_response;

    if (receive_message(socket_fd, &new_client_response) == -1) {
        fprintf(stderr, "failed to receive message\n");
        return -1;
    }

    if (new_client_response.type == SERVER_FAILURE_REGISTER) {
        fprintf(stderr, "name already taken\n");
        return -1;
    }
    else if (new_client_response.type != SERVER_OK_REGISTER) {
        fprintf(stderr, "unexpected message type\n");
        return -1;
    }
    return 0;
}

void handle_user_input()
{
    message_t message;

    char line[MAX_MESSAGE_TEXT];
    while (1) {
        fgets(line, MAX_MESSAGE_TEXT, stdin);
        line[strlen(line) - 1] = '\0';

        const char* command = strtok(line, " ");

        if (strncmp(command, "list", MAX_COMMAND_LENGTH) == 0) {
            init_message_t(&message, client_id, CLIENT_LIST, "");
        }
        else if (strncmp(command, "to_all", MAX_COMMAND_LENGTH) == 0) {
            const char* message_text = strtok(NULL, "");
            init_message_t(&message, client_id, CLIENT_TO_ALL, message_text);
        }
        else if (strncmp(command, "to_one", MAX_COMMAND_LENGTH) == 0) {
            const char* message_text = strtok(NULL, "");
            init_message_t(&message, client_id, CLIENT_TO_ONE, message_text);
        }
        else if (strncmp(command, "stop", MAX_COMMAND_LENGTH) == 0) {
            init_message_t(&message, client_id, CLIENT_STOP, "");
            if (send_message(socket_fd, &message) == -1) {
                fprintf(stderr, "failed to send message\n");
            }
            return;
        }

        else {
            fprintf(stderr, "unknown command\n");
            continue;
        }

        if (send_message(socket_fd, &message) == -1) {
            fprintf(stderr, "failed to send message\n");
        }
        else {
            printf("message sent\n");
        }
    }
}

void* wait_for_messages(void*)
{
    message_t response;
    while (true) {
        if (receive_message(socket_fd, &response) == -1) {
            fprintf(stderr, "failed to receive message\n");
            clean_exit();
        }

        switch (response.type) {
            case SERVER_CHAT_MESSAGE:
                time_t timestamp = response.timestamp;
                char time_buffer[26];
                ctime_r(&timestamp, time_buffer);
                time_buffer[strlen(time_buffer) - 1] = '\0';

                printf("%s %s: %s\n", time_buffer, response.sender, response.message);
                break;
            case SERVER_LIST_MESSAGE:
                printf("clients: \n%s\n", response.message);
                break;

            case SERVER_CHECK_ALIVE:
                message_t alive_response;
                init_message_t(&alive_response, client_id, CLIENT_ALIVE, "");
                if (send_message(socket_fd, &alive_response) == -1) {
                    fprintf(stderr, "failed to send message\n");
                }
                break;

            case SERVER_SHUTDOWN:
                printf("server shutting down\n");
                clean_exit();
                break;
            default:
                fprintf(stderr, "unexpected message type\n");
                clean_exit();
                return NULL;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <client_id> <server_address> <server_port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    client_id = argv[1];
    char const* server_ip = argv[2];
    int server_port = atoi(argv[3]);

    if (strlen(client_id) >= MAX_CLIENT_NAME) {
        fprintf(stderr, "client id too long\n");
        return EXIT_FAILURE;
    }

    if (connect_to_server(server_ip, server_port) == -1) {
        fprintf(stderr, "failed to connect to server\n");
        return EXIT_FAILURE;
    }
    signal(SIGINT, sigint_handler);
    if (register_on_server() == -1) {
        fprintf(stderr, "failed to register on server\n");
        return EXIT_FAILURE;
    }

    printf("registered client\n");

    pthread_t listener_thread;
    if (pthread_create(&listener_thread, NULL, wait_for_messages, NULL) != 0) {
        fprintf(stderr, "failed to create listener thread\n");
        return EXIT_FAILURE;
    }

    handle_user_input();

    close(socket_fd);

    return 0;
}