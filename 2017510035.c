#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Define semaphore capacities.
#define MAX_PERSONS 24
#define NUMOFSTAFF 8 


// Function prototypes
void *person(void *num);  
void *staff(void *num);
void randwait(int secs);
void reset();

//Define the semaphores.
// waitingRoom Limits the # of people allowed  to enter the waiting room at one time. - countable (default)
sem_t waitingRoom[8]; // 3

// staffAvailable ensures mutually exclusive access to the unit rooms.
// 3-testing; !(3)-no testing (default)
sem_t staffAvailable[8]; //flag for test start

// vent is used to allow the staff to ventilate the room until a person arrives.
// 0-using; 1-empty (default)
sem_t vent[8]; 

// lock determines people to enter the waiting room one at a time.
// 0-lock(default); 1-unlock
sem_t lock;

// Flag to stop the staff thread when all persons have been serviced.
int allDone = 0;
int counterOfUnit = 0; // to store unit's id
int id[3]; // to store people's id to display which room they are in  
int value; // to get sem's value
int main(int argc, char *argv[])
{   
    reset(); // re-initialize id values

    pthread_t stid[NUMOFSTAFF]; //define process thread
    pthread_t tid[MAX_PERSONS];
    int i;
    int Number[MAX_PERSONS];
    int Number2[MAX_PERSONS]; 

    for (i = 0; i < MAX_PERSONS; i++)
    {
        Number[i] = i;
    }
    for (i = 0; i < NUMOFSTAFF; i++)
    {
        Number2[i] = i;
    }
    // Initialize the semaphores with initial values...
   
    for(int j = 0; j < NUMOFSTAFF; j++)
    {
        sem_init(&waitingRoom[j], 0, 3);
        sem_init(&staffAvailable[j], 0, 0);
        sem_init(&vent[j], 0, 0);
    }
     sem_init(&lock, 0, 1);

    // Create the staff.
    for(int j = 0; j < NUMOFSTAFF; j++)
    {
        pthread_create(&stid[j], NULL, staff, (void *)&Number2[j]);
    }
    // Create the person.
    for (i = 0; i < MAX_PERSONS; i++)
    {
        pthread_create(&tid[i], NULL, person, (void *)&Number[i]);
    }

    // Join each of the threads to wait for them to finish.
    for (i = 0; i < MAX_PERSONS; i++)
    {
        pthread_join(tid[i], NULL);
    }

    // When all of the person are finished, kill the staff thread.

    allDone = 1;
    for(int j = 0; j < NUMOFSTAFF; j++)
    {
        sem_post(&vent[j]);
    }
     // Wake the staffs so he will exit.
    for(int j = 0; j < NUMOFSTAFF; j++)
    {
        
        pthread_join(stid[j], NULL);
    }
    system("PAUSE");

    return 0;
}

void *person(void *number)
{
    randwait(rand()%10 + 1);
    int num = *(int *)number; 

    printf("People %d is entering the hospital.\n", num); 
    sem_wait(&lock);//Abstract lock to the waiting room so that another person may not enter unless the previous person is done. 
    sem_getvalue(&waitingRoom[counterOfUnit], &value); // get the current capacity  of waiting room
    if (value == 0)// reset id values when next people enter another unit.
    {
        reset();
        
    }

    sem_wait(&waitingRoom[counterOfUnit]); //Decrement waiting room capacities.
    id[value - 1] = num; 
    printf("People %d is Covid-19 Test Unit %d' waiting room\n", num, counterOfUnit);
    printf("People %d is filling the form-preparing for test.\n", num);
    
    
    sem_post(&vent[counterOfUnit]);// person warns staff not to ventilate
    
    sem_wait(&staffAvailable[counterOfUnit]);//increment person count for testing
    printf("People %d is leaving from Unit and  Hospital\n", num );
}

void *staff(void *number)
{   
    
    int num = *(int *)number;
   
    
    while (!allDone)
    { 
        // 
        sem_wait(&vent[num]); // Skip this stuff at the end...
        sem_getvalue(&waitingRoom[num], &value);
        if (!allDone)
        { // Take a random amount of time to test the person.
            if (value == 0)
            {
                counterOfUnit++; // when the waiting room is full, skip to the next one
                if(counterOfUnit == 8)
                    counterOfUnit = 0;
                printf("Covid-19 Test Unit %d waiting room:\n", num);
                printf("[X - %d][X - %d][X - %d] \n",id[2], id[1], id[0]);
                sem_post(&lock); //remove the abstract lock
                printf("--> Covid-19 Test Unit %d's medical staff apply the covid test.\n", num); // Release the person when done testing...
                randwait(rand()%3 + 1);
                for (int i = 0; i < 3; i++)
                {   
                    sem_post(&waitingRoom[num]); // when the testing has ended. The semaphore's values are  redefined.  
                    sem_post(&staffAvailable[num]);
                }
                
                printf("--> The staff is ventilating the unit %d \n", num);
                
                
            }
            else if(value == 1)
            {
                printf("Covid-19 Test Unit %d waiting room:\n", num);
                printf("[X - %d][X - %d][]",id[1], id[2]);

                printf(" The last 1 people, lest's start! Please, pay attention to your social distance and hygiene; use a mask\n");
                sem_post(&lock); //remove the abstract lock
            }
            else if(value == 2)
            {   
                printf("Covid-19 Test Unit %d waiting room:\n", num);
                printf("[X - %d][][]",id[2]);

                printf(" The last 2 people, lest's start! Please, pay attention to your social distance and hygiene; use a mask\n");
                sem_post(&lock); //remove the abstract lock
            }
               
        }
        else
        {
           printf("The staff %d is going home for the day.\n", num);
           break;
        }
    }
}
void randwait(int secs)
{
    int len = 1; // Generate an arbit number...
    sleep(secs);
}

void reset()
{
    for(int j = 0; j < 3; j++)
    {
        id[j] = -1;
    }
}
