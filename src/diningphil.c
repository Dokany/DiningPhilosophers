/*
 CSCE3401 Project 3: 900131538
 
 Yasmin ElDokany
 CSCE 3401 | Fall 2016
 
 Project 3: The Dining Philosophers Problem
 */

#include <pthread.h>
#include <stdlib.h>             /* Needed for random() and srandom() usage */
#include <stdio.h>
#include <unistd.h>             /* Needed for system time and sleep based operations */

#define max_block 5            /* Maximum tolerable block duration for a philosopher */


enum {THINKING, HUNGRY, EATING} phil_state[5];          /* Philosopher states */

int phil_block_duration[5];

pthread_cond_t self[5];
pthread_mutex_t mutex_forks, mutex_block;               /* Mutex locks used */
pthread_t phil[5];              /* Philosopher threads */


/* Function prototypes */

int request_eating(int phil_id);            /* Boolean function that returns a none zero value if the philosopher of index phil_id can be granted their request for eating */

void *dining_phil(void *arg);
void initializing_phil();                   /* Function that initializes the philosopher attributes*/
void pickup_forks(int phil_id);             /* Function used by the philosopher to pick up forks to eat */
void return_forks(int phil_id);             /* Function used by the philosopher to return forks */


int main (void)
{
    srandom(time(0));           /* Seeding the later used random() */
    
    printf("Setting up the table and preparing the philosophers...\n");
    fflush(stdout);
    
    initializing_phil();       /* Initializing philosopher states and creating their threads */
    
    sleep(120);                 /* Allowing the code to run for 2 minutes */
    
    return 0;
}


void initializing_phil()
{
    pthread_mutex_init(&mutex_forks, NULL);      /* Initializing mutex_forks lock */
    pthread_mutex_init(&mutex_block, NULL);      /* Initializing mutex_block lock */

    for (int i = 0; i < 5; ++i)
    {
        pthread_create(&phil[i], NULL, dining_phil, (void *) &i);           /* Creating the philosopher threads and linking each thread to the void dining_phil function that handles their operations and takes a void pointer to the philosopher's index */
        phil_state[i] = THINKING;           /* Philosophers are set initially to thinking states */
        
        pthread_cond_init(&self[i], NULL);      /* Initializing the philosophers' condition variables */
    }
}

int request_eating(int phil_id)             /* This function returns a none zero value (grants eating request) if neither of the philosophers to the left or right of the philosopher at index phil_id are eating, and if neither of them is hungry and has been blocked for a longer time than the philosopher at index phil_id (more than the max_block time) */
{
    if (phil_state[(phil_id + 1) % 5] == EATING || phil_state[(phil_id + 4) % 5] == EATING || (phil_state[(phil_id + 1) % 5] == HUNGRY && phil_block_duration[(phil_id + 1) % 5] > phil_block_duration[phil_id] && phil_block_duration[(phil_id + 1) % 5] > max_block) || (phil_state[(phil_id + 4) % 5] == HUNGRY && phil_block_duration[(phil_id + 4) % 5] > phil_block_duration[phil_id] && phil_block_duration[(phil_id + 4) % 5] > max_block) ) return 0;
    else return 1;
}

void pickup_forks(int phil_id)
{
    /* Entry Section */
    pthread_mutex_lock(&mutex_forks);
    
    /* Critical Section */
    phil_state[phil_id] = HUNGRY;          /* Philosopher state becomes hungry */
    
    while (!request_eating(phil_id)) pthread_cond_wait(&self[phil_id], &mutex_forks);           /* While the eating request is not granted, the philosopher thread remains blocked */
    
    phil_state[phil_id] = EATING;          /* Philosopher state becomes eating after being granted their eating request */
    
    /* Exit Section */
    pthread_mutex_unlock(&mutex_forks);
}


void return_forks(int phil_id)
{
    /* Entry Section */
    pthread_mutex_lock(&mutex_forks);
    
    /* Critical Section */
    phil_state[phil_id] = THINKING;          /* Philosopher state returns to thinking */
    
    if (phil_state[(phil_id + 1) % 5] == HUNGRY) pthread_cond_signal(&self[(phil_id + 1) % 5]);         /* Signaling the philosopher to the right of the philosopher at index phil_id */
    
    if (phil_state[(phil_id + 4) % 5] == HUNGRY) pthread_cond_signal(&self[(phil_id + 4) % 5]);         /* Signaling the philosopher to the left of the philosopher at index phil_id */
    
    /* Exit Section */
    pthread_mutex_unlock(&mutex_forks);
}


void *dining_phil(void *arg)
{
    int temp;
    int phil_id = *((int *) arg);          /* Dereferencing arg to reobtain the philosopher's index */
    
    while (1)
    {
        temp = (random() % 3) + 1;         /* Random value from 1 - 3 */
        
        printf("Philosopher no. %d is thinking for %d seconds\n", phil_id + 1, temp);
        fflush(stdout);
        
        sleep(temp);         /* The philosopher thinks for a random duration temp */
        
        long hunger_init_time= time(0);    /* Initial time the philosopher became hungry at */
        
        pthread_mutex_lock(&mutex_block);
        
        phil_block_duration[phil_id] = 0;       /* Initializing the block duration of the philosopher to 0 to initiate new block duration */
        
        pthread_mutex_unlock(&mutex_block);     /* Lock used to prevent possible race condition on value in the phil_block_duration array */
        
        /* Philosopher attempts to pickup forks */
        pickup_forks(phil_id);
        
        pthread_mutex_lock(&mutex_block);
        
        phil_block_duration[phil_id] += time(0) - hunger_init_time;        /* Recalculating current block duration for the philosopher by comparing current and initial hunger times */
        
        pthread_mutex_unlock(&mutex_block);
        
        temp = (random() % 3) + 1;         /* Random value from 1 - 3 */
        
        printf("Philosopher no. %d is eating for %d seconds\n", phil_id + 1, temp);
        fflush(stdout);
        
        sleep(temp);         /* The philosopher eats for a random duration temp */
        
        printf("Philosopher no. %d has finished eating\n", phil_id + 1);
        fflush(stdout);
        
        /* Philosopher returns forks after eating */
        return_forks(phil_id);
    }
}
