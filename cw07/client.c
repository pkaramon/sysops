#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./queue_spec.h"

_Noreturn void listen_for_messages(mqd_t client_q_fd)
{
    while (1) {
        msg_t msg;
        if (mq_receive(client_q_fd, (char*)&msg, sizeof(msg), NULL) < 0) {
            perror("client: mg_receive from server text msg");
        }
        if (msg.type == TEXT) {
            printf("received message: author id: %d, message: %s\n", msg.id, msg.text);
        }
    }
}

void handle_text_input(mqd_t server_q_fd, int client_id)
{
    while (1) {
        char buf[MAX_MSG_BUF_SIZE];
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            fprintf(stderr, "could not read in your message");
            continue;
        }
        size_t len = strlen(buf);
        if (len + 1 > MAX_MSG_BUF_SIZE) {
            printf("the string is too long");
            continue;
        }

        buf[strcspn(buf, "\n")] = '\0';

        if (strncmp(buf, ":exit", MAX_MSG_BUF_SIZE) == 0) {
            return;
        }

        msg_t msg = {.id = client_id, .type = TEXT};
        memcpy(msg.text, buf, MAX_MSG_BUF_SIZE);

        if (mq_send(server_q_fd, (char*)&msg, sizeof(msg), 1) < 0) {
            perror("client: mq_send message");
        }
        printf("sent\n");
    }
}

void cleanup(mqd_t server_q_fd, mqd_t client_q_fd, const char* client_q_name)
{
    mq_close(server_q_fd);
    mq_close(client_q_fd);
    mq_unlink(client_q_name);
}

int main()
{
    pid_t pid = getpid();
    char client_q_name[MAX_MSG_BUF_SIZE];
    sprintf(client_q_name, CLIENT_QUEUE_NAME_FORMAT, (int)pid);

    struct mq_attr q_attrs = {
        .mq_curmsgs = 0, .mq_flags = 0, .mq_maxmsg = MAX_MESSAGES, .mq_msgsize = sizeof(msg_t)};

    mqd_t client_q_fd = mq_open(client_q_name, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, &q_attrs);
    if (client_q_fd < 0) {
        perror("client: mq_open");
        return EXIT_FAILURE;
    }

    mqd_t server_q_fd = mq_open(SERVER_QUEUE_NAME, O_WRONLY);
    if (server_q_fd < 0) {
        perror("client: mq_open with server");
        mq_close(client_q_fd);
        mq_unlink(client_q_name);
        return EXIT_FAILURE;
    }

    msg_t init_msg = {.id = -1, .type = INIT};
    memcpy(init_msg.text, client_q_name, MAX_MSG_BUF_SIZE);

    if (mq_send(server_q_fd, (char*)&init_msg, sizeof(init_msg), 1) < 0) {
        perror("client: mq_send INIT");
        return EXIT_FAILURE;
    }

    msg_t id_msg;
    if (mq_receive(client_q_fd, (char*)&id_msg, sizeof(id_msg), NULL) < 0) {
        perror("client: mq_receive id msg");
        cleanup(server_q_fd, client_q_fd, client_q_name);
        return EXIT_FAILURE;
    }

    int client_id = id_msg.id;
    printf("got id: %d\n", client_id);

    pid_t listener_pid = fork();

    if (listener_pid < 0) {
        perror("client: fork");
        return EXIT_FAILURE;
    }
    else if (listener_pid == 0) {
        listen_for_messages(client_q_fd);
    }
    else {
        handle_text_input(server_q_fd, client_id);
        kill(listener_pid, SIGTERM);

        msg_t msg = {.id = client_id, .type = CLIENT_EXIT};
        if (mq_send(server_q_fd, (char*)&msg, sizeof(msg), 1) < 0) {
            perror("mq_send client exit");
        }

        cleanup(server_q_fd, client_q_fd, client_q_name);

        return EXIT_SUCCESS;
    }
}
