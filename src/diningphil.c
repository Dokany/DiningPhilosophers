/*
 CSCE3401 Project 3: 900131538
 
 Yasmin ElDokany
 CSCE 3401 | Fall 2016
 
 Project 3: The Dining Philosophers Problem
*/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define max_block 10            /* Maximum tolerable block duration for a philosopher*/


enum {THINKING, HUNGRY, EATING} phil_state[5];

int phil_block_duration[5] = {};

pthread_cond_t self[5];
pthread_mutex_t mutex_forks, mutex_block;
pthread_t phil[5];


/* Prototyping */

int request_eating(int phil_id);

void *dining_phil(void *arg);
void initializing_phil();
void pickup_forks(int phil_id);
void return_forks(int phil_id);


int main (void)
{
    srandom(time(0));
    
    printf("Setting up the table and preparing the philosophers...\n");
    fflush(stdout);
           
    initializing_phil();        /* Initializing philosopher states and creating their threads */

    sleep(30);                 /* Allowing the code to run for 30 seconds */
    
	return 0;
}


void initializing_phil()
{
    for (int i = 0; i < 5; ++i)
    {
        pthread_create(&phil[i], NULL, dining_phil, (void *) &i);
        phil_state[i] = THINKING;
    }
}

int request_eating(int phil_id)
{
    if (phil_state[(phil_id + 1) % 5] == EATING || phil_state[(phil_id + 4) % 5] == EATING || (phil_state[(phil_id + 1) % 5] == HUNGRY && phil_block_duration[(phil_id + 1) % 5] < phil_block_duration[phil_id] && max_block <= phil_block_duration[(phil_id + 1) % 5]) || (phil_state[(phil_id + 4) % 5] == HUNGRY && phil_block_duration[(phil_id + 4) % 5] < phil_block_duration[phil_id] && max_block <= phil_block_duration[(phil_id + 4) % 5]) ) return 0;
    else return 1;
}

void pickup_forks(int phil_id)
{
    /* Entry Section */
    pthread_mutex_lock(&mutex_forks);
    
    /* Critical Section */
    phil_state[phil_id] = HUNGRY;
    
    while (!request_eating(phil_id)) pthread_cond_wait(&self[phil_id], &mutex_forks);
    
    phil_state[phil_id] = EATING;
    
    /* Exit Section */
    pthread_mutex_unlock(&mutex_forks);
}


void return_forks(int phil_id)
{
    /* Entry Section */
    pthread_mutex_lock(&mutex_forks);
    
    /* Critical Section */
    phil_state[phil_id] = THINKING;
    
    if (phil_state[(phil_id + 1) % 5] == HUNGRY) pthread_cond_signal(&self[(phil_id + 1) % 5]);
    
    if (phil_state[(phil_id + 4) % 5] == HUNGRY) pthread_cond_signal(&self[(phil_id + 4) % 5]);
    
    /* Exit Section */
    pthread_mutex_unlock(&mutex_forks);
}


void *dining_phil(void *arg)
{
    int temp;
    int phil_id = *((int *) arg);
    
    while (1)
    {
        temp = (random() % 3) + 1;
        
        printf("Philosopher no. %d is thinking for %d seconds\n", phil_id + 1, temp);
        fflush(stdout);
        
        sleep(temp);
        
        long hunger_init_time= time(0);

        pickup_forks(phil_id);
        
        pthread_mutex_lock(&mutex_block);

        phil_block_duration[phil_id] += time(0) - hunger_init_time;
        
        pthread_mutex_unlock(&mutex_block);

        temp = (random() % 3) + 1;
        
        printf("Philosopher no. %d is eating for %d seconds\n", phil_id + 1, temp);
        fflush(stdout);
        
        sleep(temp);
        
        printf("Philosopher no. %d has finished eating\n", phil_id + 1);
        fflush(stdout);
        
        return_forks(phil_id);
    }
}
