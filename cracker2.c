/**
 * Machine Problem: Password Cracker
 * CS 241 - Spring 2016
*/
//things to be updated
//2 questions 1 is whe my threads update secondhalf... will they formt heir own entire unhashed_passwords or not
 // show the whole thing to the TA
 //what all do i have to allocate memory for
 //priniting result in main or helper(I would prefer helper)

#include "cracker2.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"
#include "utils.h"
#include "format.h"
#include <string.h>
#include <crypt.h>
#include <unistd.h>

typedef struct extracted_data {
  char **username;
  char **crypted_pass;
  char **unhashed_pass;
  char  **secondhalves;
  int totalinputs;
} e_data;
e_data eData;


pthread_barrier_t startbarrier;
pthread_barrier_t endbarrier;

int tracker=0;
int totalthreads;
int totaliterations;

double startCPUTime;
double startclocktime;

int usernameflag=0;
int foundflag=0;
int resultflag=0;

char * finalpassword=NULL;

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m4 = PTHREAD_MUTEX_INITIALIZER;

void * helpme (void *ptr)
{
    int* mynum = (int *) (ptr);
    int threadnum = *mynum;

    while(tracker<eData.totalinputs)
    {

        long *threadstart=malloc(sizeof(long));

        long *numtries= malloc(sizeof(long));

        getSubrange(strlen(eData.secondhalves[tracker]), totalthreads, threadnum, threadstart, numtries); // threadstart and numtries are updated.

        pthread_mutex_lock(&m2);
        char *stringtotest = strdup(eData.unhashed_pass[tracker]);        //stringtotest = aaaaaaa

        char *tempsecondhalf = stringtotest;                        //tempsecondhalf = a......

        while(*tempsecondhalf!='.')
            tempsecondhalf++;                           //tempsecondhalf  = .....

        setStringPosition(tempsecondhalf, *threadstart);            //tempsecondhalf = aaaaa  or equivalent and stringtotest = aaaaaaa
        pthread_mutex_unlock(&m2);


        struct crypt_data cdata;
        cdata.initialized = 0;

        int hashcount=0;

        pthread_mutex_lock(&m1);
        if(usernameflag==0)
        {   
            startclocktime = getTime();
            startCPUTime = getCPUTime();
            resultflag=0;
            totaliterations=0;
            foundflag=0;
            v2_print_start_user(eData.username[tracker]);
            usernameflag = 1;
        }
        v2_print_thread_start(threadnum, eData.username[tracker], *threadstart, stringtotest);
        pthread_mutex_unlock(&m1);

        pthread_barrier_wait(&startbarrier);
        char *hashed;
        int localfoundflag;
        do
        {
            hashcount++;
            pthread_mutex_lock(&m1);
            localfoundflag = foundflag;
            pthread_mutex_unlock(&m1);
            if(localfoundflag==1)
            {   
                pthread_mutex_lock(&m3);
                totaliterations+=hashcount;
                v2_print_thread_result(threadnum, hashcount, 1);
                pthread_mutex_unlock(&m3);
                break;
            }
            else if(localfoundflag==0 && hashcount==*numtries)
            {
                pthread_mutex_lock(&m3);
                totaliterations+=hashcount;
                v2_print_thread_result(threadnum, hashcount, 2);
                pthread_mutex_unlock(&m3);
                break;
            }

            hashed = crypt_r(stringtotest, "xx", &cdata);

            if(strcmp(hashed, eData.crypted_pass[tracker])==0)
            {
                pthread_mutex_lock(&m1);
                pthread_mutex_lock(&m3);
                totaliterations+=hashcount;
                finalpassword = strdup(stringtotest);
                foundflag=1;
                localfoundflag = foundflag;
                usernameflag=0;
                pthread_mutex_lock(&m4);
                v2_print_thread_result(threadnum, hashcount, 0);
                pthread_mutex_unlock(&m4);
                pthread_mutex_unlock(&m3);
                pthread_mutex_unlock(&m1);
                break;
            }
        }while((incrementString(tempsecondhalf)==1) && (hashcount<=*numtries));
        
        free(threadstart);
        free(numtries);
        free(stringtotest);

        pthread_barrier_wait (&endbarrier);

        pthread_mutex_lock(&m1);
        if(resultflag==0)
        {
            resultflag=1;
            if(localfoundflag==1)
            {
                pthread_mutex_lock(&m3);
                v2_print_summary(eData.username[tracker], finalpassword, totaliterations, getTime()-startclocktime, getCPUTime()-startCPUTime, 0);
                pthread_mutex_unlock(&m3);
            }
            else
            {
                pthread_mutex_lock(&m3);
                v2_print_summary(eData.username[tracker], finalpassword, totaliterations, getTime()-startclocktime, getCPUTime()-startCPUTime, 1);
                pthread_mutex_unlock(&m3);
            }
            
            free(finalpassword);
            finalpassword=NULL;
            usernameflag=0;
            tracker++;

        }   
        
        pthread_mutex_unlock(&m1);
        pthread_barrier_wait (&endbarrier);
        //printf("Reached here\n");
    }
    return NULL;
}


int start(size_t thread_count) 
{
  // TODO your code here, make sure to use thread_count!

    //printf("I run");
    int inisize = 500;
    totalthreads = thread_count;
    char *line = NULL;
    char ** arrayofpointers;
    eData.totalinputs=0;
    int ct=0;

    eData.username = malloc(sizeof(char*) * inisize);
    eData.crypted_pass = malloc(sizeof(char*) * inisize);
    eData.unhashed_pass = malloc(sizeof(char*) * inisize);
    eData.secondhalves = malloc(sizeof(char*) * inisize);
    arrayofpointers = malloc(sizeof(char*) * inisize);

    while(1)
    {
        size_t size;
        int temp = getline(&line, &size, stdin);

        if(temp==-1)
            break;
        else
        {   
            if(ct==(inisize-1))
            {
                inisize = inisize*2;
                eData.username = realloc(eData.username, sizeof(char*) * inisize);
                eData.crypted_pass = realloc(eData.crypted_pass, sizeof(char*) * inisize);
                eData.unhashed_pass = realloc(eData.unhashed_pass, sizeof(char*) * inisize);
                eData.secondhalves = realloc(eData.secondhalves, sizeof(char*) * inisize);
                arrayofpointers = realloc(arrayofpointers, sizeof(char*) * inisize);
            }
            arrayofpointers[tracker] = strdup(line);
            char *full = arrayofpointers[tracker];

            char * traverser = full;

            //usernames
            eData.username[tracker]  = traverser;
            while(*traverser!=' ')
            {
                traverser++;
            }

            *traverser = 0;
            traverser++;

            //crypted_pass
            eData.crypted_pass[tracker] = traverser;
            while(*traverser!=' ')
            {
                traverser++;
            }

            *traverser = 0;
            traverser++;

            //unhashed_passes
            eData.unhashed_pass[tracker] = traverser;
            eData.unhashed_pass[tracker][strlen(eData.unhashed_pass[tracker])-1]=0;

            //secondhalf
            eData.secondhalves[tracker] = eData.unhashed_pass[tracker];

            while((eData.secondhalves[tracker][0])!='.')
                eData.secondhalves[tracker]++;


            tracker++;
            eData.totalinputs++;
            ct++;
        }
    }
    tracker = 0;
    free(line);

    pthread_t tid[thread_count];
    int vals[thread_count]; 

    pthread_barrier_init (&startbarrier, NULL, totalthreads);
    pthread_barrier_init(&endbarrier, NULL, totalthreads);
    for(int i=0;i< (int)thread_count;i++)
    {
        vals[i] = i+1;
        pthread_create(&tid[i], NULL, helpme, &vals[i]);
    }

    for(int i=0;i<(int)thread_count;i++)
    {
        pthread_join(tid[i], NULL);
    }
    pthread_barrier_destroy(&endbarrier);
    pthread_barrier_destroy (&startbarrier);

    pthread_mutex_destroy(&m1);
    pthread_mutex_destroy(&m2);
    pthread_mutex_destroy(&m3);
    pthread_mutex_destroy(&m4);

    for(int i =0; i<eData.totalinputs; i++)
    {
        free(arrayofpointers[i]);
    }
    free(arrayofpointers);
    free(eData.username);
    free(eData.crypted_pass);
    free(eData.unhashed_pass);
    free(eData.secondhalves);


    return 0;
}
