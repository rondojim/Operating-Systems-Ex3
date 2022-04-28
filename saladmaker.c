#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "common_types.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

struct timeval  now;
struct tm      *local;

char* gtime(){
    gettimeofday(&now, NULL);
    local = localtime(&now.tv_sec);
    char *s = (char*)malloc(20 * sizeof(char));
    sprintf(s, "%02d:%02d:%02d.%03ld", local->tm_hour, local->tm_min, 
                                        local->tm_sec, (now.tv_usec) / 1000);
    return s;
}

int main(int argc, char *argv[]){
    int id, lb, ub, p, i;

    for(i=1; i<=7; i+=2){
        char c = argv[i][1];
        if(c == 'p') p = atoi(argv[i+1]);
        else if(c == 't'){
            if(argv[i][2] == '1') lb = atoi(argv[i + 1]);
            else ub = atoi(argv[i + 1]);
        }
        else id = atoi(argv[i + 1]);
    }

    int prep_time = lb + rand() % (ub - lb + 1);

    FILE *fp = fopen("LogFile", "a");   
    char *s = malloc(20 * sizeof(char));
    sprintf(s, "Saladmaker%d", p);
    FILE *fp2 = fopen(s, "a");
    FILE *inter = fopen("TimeIntervals", "a");

    data* Data = (data*)shmat(id, (void*)0, 0);

    sem_t* mutex = &(Data->mutex);
    sem_t* c_exec = &(Data->c_exec);

    sem_wait(mutex);
    Data->waiting++;
    if(Data->waiting == 3){
        for(i=0; i<3; ++i) sem_post(c_exec);
    }
    sem_post(mutex);

    sem_wait(c_exec);

    fprintf(fp, "[%s][%d][Saladmaker%d][Waiting for ingredients]\n", gtime(), p + 1, p);
    fflush(fp);

    fprintf(fp2, "[%s][%d][Saladmaker%d][Waiting for ingredients]\n", gtime(), p + 1, p);
    fflush(fp2);

    sem_post(&(Data->start_chef));
    while(1){
        sem_wait(&(Data->available[p]));
        if(Data->exit_p == 1) break;

        char *ctime = malloc(20 * sizeof(char));
        strcpy(ctime, gtime());

        fprintf(inter, "%s 1\n", ctime);
        fflush(inter);
        fprintf(fp, "[%s][%d][Saladmaker%d][Get ingredients]\n", ctime, p + 1, p);
        fflush(fp);
        fprintf(fp2, "[%s][%d][Saladmaker%d][Get ingredients]\n", ctime, p + 1, p);
        fflush(fp2);

        sem_post(&(Data->got[p]));

        strcpy(ctime, gtime());

        fprintf(fp, "[%s][%d][Saladmaker%d][Start making salad]\n", ctime, p + 1, p);
        fflush(fp);
        fprintf(fp2, "[%s][%d][Saladmaker%d][Start making salad]\n", ctime, p + 1, p);
        fflush(fp2);

        usleep(prep_time * 1000);

        strcpy(ctime, gtime());
        
        fprintf(inter, "%s -1\n", ctime);
        fflush(inter);
        fprintf(fp, "[%s][%d][Saladmaker%d][End making salad]\n", ctime, p + 1, p);
        fflush(fp);
        fprintf(fp2, "[%s][%d][Saladmaker%d][End making salad]\n", ctime, p + 1, p);
        fflush(fp2);
        
        sem_wait(mutex);
        Data->salads_made[p]++;
        if(Data->salads_made[1] + Data->salads_made[2] + Data->salads_made[3] == Data->N){
            Data->exit_p = 1;
            for(i=1; i<=3; ++i) sem_post(&(Data->available[i]));
        }
        sem_post(mutex);
        free(ctime);
    }
    fclose(inter);
    fclose(fp);
    fclose(fp2);
    sem_post(&(Data->start_printing));
    shmdt((void*)Data);
    free(s);
    return 0;
}