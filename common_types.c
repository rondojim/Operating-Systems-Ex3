#include <semaphore.h>

struct data{
    int cur_ing;
    int prev_ing;
    int rem_slds;
    sem_t mutex;
};