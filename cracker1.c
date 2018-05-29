/**
 * Machine Problem: Password Cracker
 * CS 241 - Spring 2016
 */

/*things to add
1. Free shit
2. add format.h statements
*/
#include "cracker1.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"
#include "utils.h"
#include "format.h"
#include <string.h>
#include <crypt.h>
#include <unistd.h>

Queue myqueue;
int correc_pass;
int incorrect_pass;
int totalinputs=0;
//double CPUtime;

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m3 = PTHREAD_MUTEX_INITIALIZER;

void * helpme(void *ptr)
{	

	double startthreadtime = getThreadCPUTime();
	//double overallCPUstart = getCPUTime();
	int* mynum = (int *) (ptr);
	int threadnum = *mynum;

	while(totalinputs!=0)
	{
		pthread_mutex_lock(&m3);
		totalinputs--;
		pthread_mutex_unlock(&m3);
		
		void *pulled = Queue_pull(&myqueue);

		char *full = (char *) pulled;

		//char *word;

		char *username;
		char *crypted_pass;
		char *unhashed_pass;

		int hashcount = 0;


		char * traverser = full;

		username  = traverser;
		while(*traverser!=' ')
		{
			traverser++;
		}

		*traverser = 0;
		traverser++;

		crypted_pass = traverser;

		while(*traverser!=' ')
		{
			traverser++;
		}

		*traverser = 0;
		traverser++;

		unhashed_pass = traverser;

		unhashed_pass[strlen(unhashed_pass)-1] = 0;

		char *secondhalf = unhashed_pass;	//...

		while(*secondhalf!='.')
				secondhalf++;

		char * temp = secondhalf;
		while(*temp!=0)				//after while loop unhashed_pass = helloaaa and secondhalf = aaa
		{
			*temp = 'a';
			temp++;
		}


		v1_print_thread_start(threadnum, username);

		struct crypt_data cdata;
		cdata.initialized = 0;

		int flag = 0;
		char* hashed;

		do
		{
			hashcount++;
			hashed = crypt_r(unhashed_pass, "xx", &cdata);		//check if hashed needs to be freed
			if(strcmp(hashed, crypted_pass)==0)
			{
				pthread_mutex_lock(&m1);
				flag=1;
				correc_pass++;
				pthread_mutex_unlock(&m1);

				double endthreadtime = getThreadCPUTime();
				v1_print_thread_result(threadnum, username, unhashed_pass, hashcount, endthreadtime - startthreadtime, 0);		// to change
				//free(hashed);
				break;
			}
		}while(incrementString(secondhalf)==1);

		if(flag==0)
		{
			pthread_mutex_lock(&m2);
			incorrect_pass++;
			pthread_mutex_unlock(&m2);
			
			double endthreadtime = getThreadCPUTime();
			v1_print_thread_result(threadnum, username, NULL, hashcount, endthreadtime - startthreadtime, 1); //what to pass in here  ... to change

		}
	}
		//double overallCPUend = getCPUTime();
		//CPUtime = overallCPUend - overallCPUstart;
		return NULL;
}

int start(size_t thread_count) 
{
  // your code here
  // make sure to make thread_count threads
	double starttime = getTime();
	double startCPUtime = getCPUTime();
	Queue_init(&myqueue, -1);

	char *line = NULL;
	char * arrayofpointers[400];
	while(1)
	{
		size_t size;
		int temp = getline(&line, &size, stdin);

    	if(temp==-1)
        	break;
        else
        {
        	arrayofpointers[totalinputs] = strdup(line);
        	char *tempstring = arrayofpointers[totalinputs];
        	Queue_push(&myqueue, tempstring);
        	totalinputs++;
        }
    }
    free(line);
    int temptotal = totalinputs;
    pthread_t tid[thread_count];
    int vals[thread_count];

    for(int i=0;i< (int)thread_count;i++)
    {
    	vals[i] = i+1;
    	pthread_create(&tid[i], NULL, helpme, &vals[i]);
    }

    for(int i=0;i<(int)thread_count;i++)
    {
    	pthread_join(tid[i], NULL);
    }

    Queue_destroy(&myqueue);
    for(int i =0; i<temptotal; i++)
    {
    	free(arrayofpointers[i]);
    }
    double endtime = getTime();
    double endCPUtime  = getCPUTime();
    double wallclock = endtime - starttime;
    double totalcputime = endCPUtime - startCPUtime;

    v1_print_summary(correc_pass, incorrect_pass, wallclock, totalcputime);		//to change

	return 0;
}






