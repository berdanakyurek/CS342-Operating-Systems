#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

struct threadArg
{
    int philNo;
    int stTime;
};

// This function creates a random number
// In range [min, max] somehow uniformly
// See function definition below for details
double randomNumber(int min, int max);
void* philFunction(void* p);
int test = 0;

const int NUMBER_OF_PHILOSOPHERS = 5;
pthread_mutex_t mutex;
pthread_cond_t conds[5];

// 0 if chopstick[n] available.
// 1 if occupied
int chopStates[5];

int main()
{

    /* for(int i = 0; i < 5; i ++) */
    /*     printf("%f\n", randomNumber(1, 10)); */
    pthread_t threads[NUMBER_OF_PHILOSOPHERS];

    // Init mutex
    pthread_mutex_init(&mutex, NULL);

    // Init condition variables
    for(int i = 0; i < NUMBER_OF_PHILOSOPHERS; i ++)
        pthread_cond_init(&conds[i], NULL);

    for(int i = 0; i < NUMBER_OF_PHILOSOPHERS; i ++)
        chopStates[i] = 0;

    int currentTime = time(NULL);
    struct threadArg args[NUMBER_OF_PHILOSOPHERS];


    for(int i = 0; i < NUMBER_OF_PHILOSOPHERS; i ++)
    {
        args[i].stTime = currentTime;
        args[i].philNo = i;

        int ret = pthread_create(&threads[i],
                                 NULL, philFunction,
                                 &args[i] );
        if (ret != 0)
        {
            printf("Thread create failed \n");
            exit(1);
        }
    }


    // Wait for child threads to terminate
    for (int i = 0; i < NUMBER_OF_PHILOSOPHERS; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&mutex);

    for(int i = 0; i < NUMBER_OF_PHILOSOPHERS; i ++)
        pthread_cond_destroy(&conds[i]);
    return 0;
}

double randomNumber(int min, int max)
{
    int rn = rand(); // Random and uniform in range [0, RAND_MAX]

    // step satisfies min + k * step  is between min and max
    // for k in range [0, RAND_MAX]
    double step = (double)(max - min) / (double)RAND_MAX;

    // It is guaranteed that rn is in range [0, RAND_MAX]
    // So result is in range [min, max]
    // And it is uniform for RAND_MAX numbers
    // That all having same probability of occuring
    // And each having difference of step.
    return (double)min + (double) rn * step;
}

void* philFunction(void* p)
{
    struct threadArg* pointer = (struct threadArg*) p;


    int stTime = pointer->stTime;
    int philNumber = pointer->philNo;

    // Chopstick numbers
    int chopstickL = philNumber;
    int chopstickR ;
    if(philNumber == NUMBER_OF_PHILOSOPHERS - 1)
        chopstickR = 0;
    else
        chopstickR = philNumber + 1;

    // stTime is same for all threads
    // It depends on time on main thread
    // However philNumber is different for each thread
    // So it is guaranteed that seed is different for each thread
    // Also it is guaranteed that it will be undeterministic
    //Since seed depends on time
    srand(stTime + philNumber);

    while(1)
    {
        // Philosopher is hungry
        pthread_mutex_lock(&mutex);

        while(chopStates[chopstickL] == 1 || chopStates[chopstickR] == 1)
        {
            if(chopStates[chopstickL] == 1)
                pthread_cond_wait(&conds[chopstickL], &mutex);
            if(chopStates[chopstickR] == 1)
                pthread_cond_wait(&conds[chopstickR], &mutex);
        }


        chopStates[chopstickL] = 1;
        chopStates[chopstickR] = 1;

        pthread_mutex_unlock(&mutex);

        printf("philosopher %d started eating now.\n", philNumber);
        sleep(randomNumber(1, 5));
        printf("philosopher %d finished eating now.\n", philNumber);


        chopStates[chopstickR] = 0;
        pthread_cond_signal(&conds[chopstickR]);

        chopStates[chopstickL] = 0;
        pthread_cond_signal(&conds[chopstickL]);

        //Thinking phase
        sleep(randomNumber(1, 10));
    }
    pthread_exit(&test);
}
