#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TOTAL_DELIVARIES 4
#define REINDEER_COUNT 9

int reindeer_ready_count = 0;
pthread_mutex_t reindeer_ready_count_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;

pthread_t reindeer_threads[REINDEER_COUNT];
sem_t reindeer_sems[REINDEER_COUNT];

pthread_t santa_thread;

void* reindeer(void* arg)
{
    int reindeer_id = *(int*)arg;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (1) {
        sem_wait(&reindeer_sems[reindeer_id]);

        printf("Renifer %d leci na wakacje\n", reindeer_id);
        sleep(rand() % 6 + 5);

        pthread_mutex_lock(&reindeer_ready_count_mutex);
        reindeer_ready_count++;

        printf("Renifer: czeka %d reniferów na Mikołaja, %d\n", reindeer_ready_count, reindeer_id);

        if (reindeer_ready_count == REINDEER_COUNT) {
            pthread_cond_signal(&santa_cond);
            reindeer_ready_count = 0;
        }

        pthread_mutex_unlock(&reindeer_ready_count_mutex);
    }

    return NULL;
}

void* santa(__attribute__((unused)) void* arg)
{
    for (int delivery = 0; delivery < TOTAL_DELIVARIES; delivery++) {
        pthread_cond_wait(&santa_cond, &santa_mutex);
        printf("Mikołaj: budzę się\n");

        printf("Mikolaj: dostarczam zabawki\n");
        sleep(rand() % 3 + 2);
        printf("Mikołaj: zasypiam\n");
        for (int i = 0; i < REINDEER_COUNT; i++) {
            sem_post(&reindeer_sems[i]);
        }
    }

    // cancel all reindeer threads
    for (int i = 0; i < REINDEER_COUNT; i++) {
        pthread_cancel(reindeer_threads[i]);
    }

    return NULL;
}

int main()
{
    int reindeer_ids[REINDEER_COUNT];
    for (int i = 0; i < REINDEER_COUNT; i++) {
        reindeer_ids[i] = i;
        sem_init(&reindeer_sems[i], 0, 1);
    }

    pthread_create(&santa_thread, NULL, santa, NULL);
    for (int i = 0; i < REINDEER_COUNT; i++) {
        pthread_create(&reindeer_threads[i], NULL, reindeer, &reindeer_ids[i]);
    }

    pthread_join(santa_thread, NULL);
    for (size_t i = 0; i < REINDEER_COUNT; i++) {
        pthread_join(reindeer_threads[i], NULL);
    }

    for (int i = 0; i < REINDEER_COUNT; i++) {
        sem_destroy(&reindeer_sems[i]);
    }
    pthread_mutex_destroy(&reindeer_ready_count_mutex);
    pthread_mutex_destroy(&santa_mutex);
    pthread_cond_destroy(&santa_cond);

    return 0;
}