#define _DEFAULT_SOURCE 1

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "socket_utils.h"

#define MAX_EVENTS 100

typedef struct {
    char name[MAX_CLIENT_NAME];
    int socket;
    bool valid;
    bool awaiting_alive_response;
} client_t;

int server_socket = -1;
int epoll_fd = -1;

client_t clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void cleanup()
{
    pthread_mutex_trylock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].valid) {
            message_t message;
            init_message_t(&message, "SERVER", SERVER_SHUTDOWN, "");
            if (send_message(clients[i].socket, &message) == -1) {
                fprintf(stderr, "failed to send message\n");
            }
            close(clients[i].socket);
        }
    }
    shutdown(server_socket, SHUT_RDWR);
    if (close(server_socket) == -1) {
        perror("close server_socket");
    }
    if (close(epoll_fd) == -1) {
        perror("close epoll_fd");
    }
}

void sigint_handler(__attribute__((unused)) int signum)
{
    cleanup();
    exit(EXIT_SUCCESS);
}

void init_clients()
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].valid = false;
        clients[i].name[0] = '\0';
        clients[i].socket = -1;
        clients[i].awaiting_alive_response = false;
    }
    pthread_mutex_unlock(&clients_mutex);
}

int add_client(const char* name, int socket)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].valid && strncmp(clients[i].name, name, MAX_CLIENT_NAME) == 0) {
            return -1;
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].valid) {
            clients[i].valid = true;
            clients[i].socket = socket;
            memcpy(clients[i].name, name, MAX_CLIENT_NAME);
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
    return 0;
}

void handle_client_delete(client_t* clients, const char* name)
{
    pthread_mutex_lock(&clients_mutex);
    int to_close = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].valid && strncmp(clients[i].name, name, MAX_CLIENT_NAME) == 0) {
            to_close = clients[i].socket;
            clients[i].valid = false;
            clients[i].name[0] = '\0';
            clients[i].socket = -1;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (to_close != -1) close(to_close);
}

void handle_to_all_message(client_t* clients, const char* sender, const char* message)
{
    message_t response;
    init_message_t(&response, sender, SERVER_CHAT_MESSAGE, message);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].valid && strncmp(clients[i].name, sender, MAX_CLIENT_NAME) != 0) {
            if (send_message(clients[i].socket, &response) == -1) {
                fprintf(stderr, "failed to send message\n");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void handle_to_one_message(client_t* clients, const char* sender, char* message)
{
    const char* receiver_name = strtok(message, " ");
    const char* message_text = strtok(NULL, "");

    message_t msg;
    init_message_t(&msg, sender, SERVER_CHAT_MESSAGE, message_text);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].valid && strncmp(clients[i].name, receiver_name, MAX_CLIENT_NAME) == 0) {
            if (send_message(clients[i].socket, &msg) == -1) {
                fprintf(stderr, "failed to send message\n");
            }
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void handle_new_client(char* sender, int client_fd)
{
    bool okay = add_client(sender, client_fd) == 0;
    message_t response;
    init_message_t(&response, "SERVER", okay ? SERVER_OK_REGISTER : SERVER_FAILURE_REGISTER,
                   okay ? "successfully registered" : "identifier already taken");
    if (send_message(client_fd, &response) == -1) {
        fprintf(stderr, "failed to send message\n");
    }
}

void handle_list_request(int client_fd)
{
    char buffer[MAX_MESSAGE_TEXT];
    buffer[0] = '\0';

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].valid) {
            strcat(buffer, clients[i].name);
            strcat(buffer, "\n");
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    message_t list_response;
    init_message_t(&list_response, "SERVER", SERVER_LIST_MESSAGE, buffer);
    if (send_message(client_fd, &list_response) == -1) {
        fprintf(stderr, "failed to send message\n");
    }
}

void handle_got_alive_message(int client_fd)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == client_fd) {
            clients[i].awaiting_alive_response = false;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

int setup_server_socket(const char* address, int port)
{
    struct sockaddr_in server_address;
    if (init_ipv4_address(&server_address, address, port) == -1) {
        fprintf(stderr, "failed to create address\n");
        return -1;
    }

    int socket_fd = create_ipv4_socket();
    if (socket_fd == -1) {
        perror("socket");
        return -1;
    }

    if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }

    if (bind(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("bind");
        return -1;
    }

    if (listen(socket_fd, 10) == -1) {
        perror("listen");
        return -1;
    }

    return socket_fd;
}

void accept_new_connection_requests()
{
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    struct epoll_event event;

    while (true) {
        int client_socket =
            accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket == -1) {
            if (!(errno == EAGAIN || errno == EWOULDBLOCK)) perror("accept");
            break;
        }

        if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1) {
            perror("fcntl");
            return;
        }

        event.events = EPOLLIN | EPOLLET;
        event.data.fd = client_socket;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
            perror("epoll_ctl");
            return;
        }
    }
}

void handle_client_message(int client_fd)
{
    message_t message;
    if (receive_message(client_fd, &message) == -1) {
        fprintf(stderr, "failed to receive message\n");
        return;
    }

    switch (message.type) {
        case CLIENT_NEW:
            printf("got new client message: %s\n", message.sender);
            handle_new_client(message.sender, client_fd);

            break;
        case CLIENT_STOP:
            printf("got stop message: %s\n", message.sender);
            handle_client_delete(clients, message.sender);
            break;

        case CLIENT_TO_ALL:
            printf("got to all message: %s\n", message.sender);
            handle_to_all_message(clients, message.sender, message.message);
            break;

        case CLIENT_TO_ONE:
            printf("got to one message: %s\n", message.sender);
            handle_to_one_message(clients, message.sender, message.message);
            break;

        case CLIENT_LIST:
            printf("got list message: %s\n", message.sender);
            handle_list_request(client_fd);
            break;

        case CLIENT_ALIVE:
            printf("got alive message: %s\n", message.sender);
            handle_got_alive_message(client_fd);

            break;

        default:
            break;
    }
}

void* check_if_clients_are_alive(void*)
{
    while (true) {
        sleep(10);
        printf("will be sending check alive message\n");

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].valid) {
                message_t message;
                init_message_t(&message, "SERVER", SERVER_CHECK_ALIVE, "");
                if (send_message(clients[i].socket, &message) == -1) {
                    fprintf(stderr, "failed to send message\n");
                }

                clients[i].awaiting_alive_response = true;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        sleep(10);

        printf("will be checking who is still alive\n");

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].valid && clients[i].awaiting_alive_response) {
                // PRITING ONLY FOR SHOWCASING PURPOSES, OUGHT TO BE REMOVED
                // printf(
                //     "Client %s did not respond to heartbeat and will be considered
                //     disconnected.\n", clients[i].name);
                close(clients[i].socket);
                clients[i].valid = false;
                clients[i].awaiting_alive_response = false;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
}

int main(int argc, char const* argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <address> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* address = argv[1];
    int port = atoi(argv[2]);

    server_socket = setup_server_socket(address, port);
    if (server_socket == -1) {
        fprintf(stderr, "failed to create server socket\n");
        return EXIT_FAILURE;
    }

    signal(SIGINT, sigint_handler);

    printf("server is listening on port %d\n", port);

    init_clients();

    struct epoll_event event, events[MAX_EVENTS];
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        return EXIT_FAILURE;
    }
    event.data.fd = server_socket;
    event.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
        perror("epoll_ctl");
        return EXIT_FAILURE;
    }

    pthread_t check_if_alive_thread;
    if (pthread_create(&check_if_alive_thread, NULL, check_if_clients_are_alive, NULL) != 0) {
        fprintf(stderr, "failed to create alive thread\n");
        return EXIT_FAILURE;
    }

    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
        }

        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;
            if (fd == server_socket) {
                accept_new_connection_requests(epoll_fd);
            }
            else if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||
                     (!(events[i].events & EPOLLIN))) {
                perror("epoll error");
                close(fd);
            }
            else {
                handle_client_message(fd);
            }
        }
    }

    cleanup();

    return 0;
}