/*
 Name - Neil Patel
 U-Number - U57880842
 Project 3 - Client program for shared bounded buffer with locks and synchronization.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "shared_mem.h"

//Sender thread takes c2s message buffer as argument. 
void *sender(void *arg)
{
	//Converts argument into message buffer pointer.
	struct message_buffer *c2s = (struct message_buffer *) arg;
	
	//For accepting user input
	char produced[MAX_MSG_LEN];
	
	//To run this thread continuously and never exit.
	while(1)
	{
		//Check for script which has EOF at the end but stdin doesn't
		if(fgets(produced, MAX_MSG_LEN, stdin) != NULL)
		{
			//Locking
			pthread_mutex_lock(&c2s->mutex);
			
			//Waiting if buffer is full
			while ((c2s->tail - c2s->front) == NUMBER_OF_MSG)
			{
				pthread_cond_wait(&c2s->full, &c2s->mutex);
			}
			
			//Copying the user inputted string into messages buffer
			strcpy(c2s->messages[c2s->tail % NUMBER_OF_MSG], produced);
			
			//Increasing say message is added.
			c2s->tail++;
			
			//Signalling
			pthread_cond_signal(&c2s->empty);
			
			//Unlocking
			pthread_mutex_unlock(&c2s->mutex);
		}
		//Used to redirect input to stdin after piping.
		else
		{
			freopen("/dev/tty", "rw", stdin);
		}
	}
	return NULL;
}

//Receiver thread takes s2c as argument.
void *receiver(void *arg)
{
	//Converting void pointer into message buffer.
	struct message_buffer *s2c = (struct message_buffer *) arg;
	
	//String for outputting the received message.
	char output[MAX_MSG_LEN];
	
	//Looping to never exit this thread and go to the main thread.
	while(1)
	{
		//Lock
		pthread_mutex_lock(&s2c->mutex);
		
		//Waiting if no values in buffer.
		while(s2c->front == s2c->tail)
		{
			pthread_cond_wait(&s2c->empty, &s2c->mutex);
		}
		//Copying received message into output string.
		
		strcpy(output, s2c->messages[s2c->front % NUMBER_OF_MSG]);
		s2c->front++;
		
		//Signalling to the thread for receiveing from server.
		pthread_cond_signal(&s2c->full);
		
		//Unlock
		pthread_mutex_unlock(&s2c->mutex);
		
		//Printing the output string.
		printf("Message Received: %s", output);
	}
	return NULL;
} 

int main(int argc, char *argv[])
{
	//If no key provided then print how to use it and exit.
	if (argc == 1)
	{
		printf("USAGE: ./client [key]\n");
		exit(1);
	}

	//Get ID for the shared data from the provodid key.
	//Exit if fails.
	int shm_id = shmget(atoi(argv[1]), sizeof(struct shm_data), 0666);
	if (shm_id == -1)
	{
		printf("ERROR: shmget failed!\n");
		exit(1);
	}
	
	//Attach the shared memory into the address space.
	//Exit if fails.
	struct shm_data *p = (struct shm_data*)shmat(shm_id, NULL, 0);
	if (p == (void *) -1)
	{
		printf("ERROR: shmat failed!\n");
		exit(1);
	}

	//create thread data structure for sender and receiver.
	pthread_t s;
	pthread_t r;
	
	//Create the threads here.
	pthread_create(&r, NULL, &receiver, &p->s2c);
	pthread_create(&s, NULL, &sender, &p->c2s);
	
	//Join statements so they never exit as both receiver and sender are always looping.
	pthread_join(r, NULL);
	pthread_join(s, NULL);
	
	//Detach shared memory
	shmdt(p);

	return 0;
}	
