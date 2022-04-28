#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "common_types.h"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

struct timeval  now;
struct tm      *local;

const char *ing[3] = {"ntomata piperia", "ntomata kremudi", "piperia ntomata"};

char* gtime(){
    gettimeofday(&now, NULL);
    local = localtime(&now.tv_sec);
    char *s = (char*)malloc(40 * sizeof(char));
    sprintf(s, "%02d:%02d:%02d.%03ld", local->tm_hour, local->tm_min, 
                                        local->tm_sec, (now.tv_usec) / 1000);
    return s;
}

int main(int argc, char *argv[]){
    int N, M;
    if(argv[1][1] == 'n') N = atoi(argv[2]), M = atoi(argv[4]);
    else N = atoi(argv[4]), N = atoi(argv[3]);

    FILE *fp = fopen("LogFile", "a");
    int id = shmget(IPC_PRIVATE, sizeof(struct data), 0666);

    data* Data = (data*)shmat(id, (void*)0, 0);
    sem_init(&(Data->mutex), 1, 1);
    sem_init(&(Data->c_exec), 1, 0);
    sem_init(&(Data->start_chef), 1, 0);
    Data->N = N;
    int i;
    for(i=1; i<=3; ++i){
        sem_init(&(Data->available[i]), 1, 0);
        sem_init(&(Data->got[i]), 1, 0);
        Data->salads_made[i] = 0;
    }
    Data->waiting = 0;
    Data->exit_p = 0;
    sem_init(&(Data->start_printing), 1, 0);
    printf("%d\n", id);

    sem_wait(&(Data->start_chef));

    usleep(10000);

    int prev_ing = -1;
    for(i=1; i<=N; ++i){
        fprintf(fp, "%d ---------------------------\n", i);
        fflush(fp);
        int x = rand() % 3;
        if(x == prev_ing) x = (x + rand() % 2 + 1) % 3;
        prev_ing = x;
        x++;
        fprintf(fp, "[%s][1][Chef][Selecting Ingredients %s]\n", gtime(), ing[x - 1]);
        fprintf(fp, "[%s][1][Chef][Notify saladmaker #%d]\n", gtime(), x);
        fflush(fp);
        sem_post(&(Data->available[x]));
        sem_wait(&(Data->got[x]));
        fprintf(fp, "[%s][1][Chef][Man time for resting]\n", gtime());
        fflush(fp);
        usleep(M * 1000);
    }

    sem_wait(&(Data->start_printing));

    printf("Total number of Salads: %d\n", Data->salads_made[1] + Data->salads_made[2] + Data->salads_made[3]);
    
    for(i=1; i<=3; ++i){
        printf("Number of Salads of salad_maker%d: %d\n", i, Data->salads_made[i]);
    }
    printf("Time Intervals:\n");

    FILE *inter = fopen("TimeIntervals", "r");
    char **ptimes = malloc((2 * N + 3) * sizeof(char*));
    char *prev = malloc(sizeof(char) * 20);
    strcpy(prev, "----");
    int cnt = -1;
    int *arr = malloc((2 * N + 5) * sizeof(int));
    for(i=0; i<=2*N; ++i) arr[i] = 0;

    for(i=0; i<2*N; ++i){
        ptimes[i] = malloc(sizeof(char) * 20);
        int type;
        fscanf(inter, "%s %d", ptimes[i], &type);
        if(strcmp(ptimes[i], prev) == 0) arr[cnt] += type;
        else{
            arr[++cnt] += type;
            strcpy(ptimes[cnt], ptimes[i]);
        }
    }
    int sum = 0, pr = -1;
    for(i=0; i<=cnt; ++i){
        if(sum < 2 && sum + arr[i] >= 2){
            pr = i;
        }
        else if(sum >= 2 && sum + arr[i] < 2){
            printf("[%s %s]\n", ptimes[pr], ptimes[i]);
        }
        sum += arr[i];
    }
    shmdt((void*)Data);
    shmctl(id, IPC_RMID , 0);
    fclose(inter);
    free(prev);
    for(i=0; i<2*N+3; ++i) free(ptimes[i]);
    free(ptimes);
    free(arr);
    return 0;
}