#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TOTAL_DELIVARIES 4
#define REINDEER_COUNT 9

int reindeer_ready_count = 0;
pthread_mutex_t reindeer_ready_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_wake_up_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeer_free_to_go_cond = PTHREAD_COND_INITIALIZER;
pthread_t reindeer_threads[REINDEER_COUNT];

void* reindeer(void* arg)
{
    int reindeer_id = *(int*)arg;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    pthread_mutex_lock(&reindeer_ready_count_mutex);
    while (1) {
        printf("Renifer %d leci na wakacje\n", reindeer_id);
        pthread_mutex_unlock(&reindeer_ready_count_mutex);

        sleep(rand() % 6 + 5);

        pthread_mutex_lock(&reindeer_ready_count_mutex);
        reindeer_ready_count++;

        printf("Renifer: czeka %d reniferów na Mikołaja, %d\n", reindeer_ready_count, reindeer_id);

        if (reindeer_ready_count == REINDEER_COUNT) {
            printf("Renifer: wybudzam Mikołaja, %d\n", reindeer_id);
            pthread_cond_signal(&santa_wake_up_cond);
        }

        pthread_cond_wait(&reindeer_free_to_go_cond, &reindeer_ready_count_mutex);
    }

    return NULL;
}

void* santa(__attribute__((unused)) void* arg)
{
    for (int i = 0; i < TOTAL_DELIVARIES; i++) {
        pthread_mutex_lock(&reindeer_ready_count_mutex);

        while (reindeer_ready_count < REINDEER_COUNT) {
            pthread_cond_wait(&santa_wake_up_cond, &reindeer_ready_count_mutex);
        }

        printf("Mikołaj: budzę się\n");

        printf("Mikolaj: dostarczam zabawki\n");
        sleep(rand() % 3 + 2);

        printf("Mikołaj: zasypiam\n");

        reindeer_ready_count = 0;
        if (i != TOTAL_DELIVARIES - 1) {
            pthread_cond_broadcast(&reindeer_free_to_go_cond);
        }
        pthread_mutex_unlock(&reindeer_ready_count_mutex);
    }

    for (int i = 0; i < REINDEER_COUNT; i++) {
        pthread_cancel(reindeer_threads[i]);
    }

    return NULL;
}

int main()
{
    pthread_t santa_thread;
    pthread_create(&santa_thread, NULL, santa, NULL);

    int reindeer_ids[REINDEER_COUNT];
    for (int i = 0; i < REINDEER_COUNT; i++) {
        reindeer_ids[i] = i;
        pthread_create(&reindeer_threads[i], NULL, reindeer, &reindeer_ids[i]);
    }

    pthread_join(santa_thread, NULL);

    pthread_mutex_destroy(&reindeer_ready_count_mutex);
    pthread_cond_destroy(&santa_wake_up_cond);
    pthread_cond_destroy(&reindeer_free_to_go_cond);

    return 0;
}