#include <semaphore.h>

typedef struct data{
    int N;
    sem_t mutex, c_exec, start_chef;
    sem_t available[5], got[5];
    sem_t start_printing;
    int waiting, exit_p;
    int salads_made[5], a, b, c;
} data;
