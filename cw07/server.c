#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "./queue_spec.h"

int curr_max_client_num = 0;
mqd_t client_queues[MAX_CLIENTS] = {0};


void handle_init_message(mqd_t *client_queues, msg_t *message)
{
    printf("got INIT message\n");
    int new_client_num = curr_max_client_num;
    client_queues[new_client_num] = mq_open(message->text, O_WRONLY);
    if (client_queues[new_client_num] < 0) {
        perror("mq_open new client");
        return;
    }

    curr_max_client_num++;
    msg_t id_msg = {.type = RECEIVE_ID, .id = new_client_num};
    if (mq_send(client_queues[new_client_num], (char *)&id_msg, sizeof(id_msg), 1) < 0) {
        perror("server: mq_send back id");
    }
}

void handle_text_message(msg_t *message, mqd_t *client_queues)
{
    printf("got TEXT message %d %s\n", message->id, message->text);
    for (int i = 0; i < curr_max_client_num; i++) {
        if (client_queues[i] == 0) continue;

        if (mq_send(client_queues[i], (char *)message, sizeof(*message), 1) < 0) {
            fprintf(stderr, "could not broadcast the message to client with id: %d\n", i);
        }
        else {
            printf("sent message %s to client %d\n", message->text, i);
        }
    }
}

void handle_client_exit_message(msg_t *message, mqd_t *client_queues)
{
    printf("got CLIENT EXIT message\n");
    if (0 <= message->id && message->id < MAX_CLIENTS) {
        mq_close(client_queues[message->id]);
        client_queues[message->id] = 0;
    }
}

void dispatch_message_type(mqd_t *client_queues, msg_t *message)
{
    switch (message->type) {
        case INIT:
            handle_init_message(client_queues, message);
            break;
        case TEXT:
            handle_text_message(message, client_queues);
            break;
        case CLIENT_EXIT:
            handle_client_exit_message(message, client_queues);
            break;
        default:
            fprintf(stderr, "message type %d is unknown\n", message->type);
            break;
    }
}


void interupt_handler(int sig_no)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_queues[i] != 0) {
            printf("there are still clients connected to the server\n");
            return;
        }
    }

    mq_unlink(SERVER_QUEUE_NAME);
    printf("exiting...");
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, interupt_handler);

    struct mq_attr q_attrs = {
        .mq_flags = 0, .mq_maxmsg = MAX_MESSAGES, .mq_msgsize = sizeof(msg_t), .mq_curmsgs = 0};

    mqd_t queue = mq_open(SERVER_QUEUE_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, &q_attrs);
    if (queue < 0) {
        perror("server: mq_open");
        return EXIT_FAILURE;
    }

    printf("Started listening...\n");
    while (1) {
        msg_t message;

        printf("waiting for messages\n");
        if (mq_receive(queue, (char *)&message, sizeof(message), NULL) < 0) {
            perror("server: mq_receive");
            mq_close(queue);
            mq_unlink(SERVER_QUEUE_NAME);
            return EXIT_FAILURE;
        }

        dispatch_message_type(client_queues, &message);
    }
}
