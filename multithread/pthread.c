// https://www.makeuseof.com/c-linux-thread-create/
// gcc pthread.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int glob = 0;

void *worker(void *data)
{
    char *name = (char *)data;

    for (int i = 0; i < 120; i++)

    {
        usleep(500000);
        glob++;
        printf("Hi from thread name = %s, i = %i\n", name, glob);
    }

    printf("Thread %s done!\n", name);
    return NULL;
}

int main(void)
{
    pthread_t th1, th2;
    pthread_create(&th1, NULL, worker, "X");
    pthread_create(&th2, NULL, worker, "Y");
    sleep(2);
    printf("Cancel X\n");
    pthread_cancel(th1);
    sleep(10);
    printf("Exiting from main program\n");
    return 0;
}
