#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "./queue_spec.h"


int curr_client_num = 0;

void dispatch_message_type(mqd_t *client_queues, msg_t *message);

int main() {
    mqd_t client_queues[MAX_CLIENTS];

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
        if (mq_receive(queue, (char *) &message, sizeof(message), NULL) < 0) {
            perror("server: mq_receive");
            mq_close(queue);
            return EXIT_FAILURE;
        }

        dispatch_message_type(client_queues, &message);
    }
}

void dispatch_message_type(mqd_t *client_queues, msg_t *message) {
    switch (message->type) {
        case INIT:
            printf("got INIT message\n");
            int new_client_num = curr_client_num;
            client_queues[new_client_num] = mq_open(message->text, O_RDWR);
            if (client_queues[new_client_num] < 0) {
                perror("mq_open new client");
                break;
            }

            curr_client_num++;
            msg_t id_msg = {.type = RECEIVE_ID, .id = new_client_num};
            if (mq_send(client_queues[new_client_num], (char *) &id_msg, sizeof(id_msg), 1) < 0) {
                perror("server: mq_send back id");
            }

            break;
        case TEXT:
            printf("got TEXT message %d %s\n", message->id, message->text);
            for (int i = 0; i < curr_client_num; i++) {
                if (mq_send(client_queues[i], (char *) message, sizeof(*message), 1) < 0) {
                    fprintf(stderr, "could not broadcast the message to client with id: %d\n", i);
                } else {
                    printf("sent message %s to client %d\n", message->text, i);
                }
            }

            break;
        default:
            fprintf(stderr, "message type %d is unknown\n", message->type);
            break;
    }
}
