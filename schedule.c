/*
CS342 Project 2
Berdan Akyurek 21600904
Samir Suleymanli 21701377
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct Burst{
    int no;
    int remainingTime;
    int arrivalTime;
};
// USE THESE FUNCTIONS TO IMPLEMENT READY QUEUE
// Priority queue functions use array implementation of min heap

// mode = 0 -> add by arrival time
// mode = 1 -> add by remaining time
// mode = 2 -> add to the end of the queue
// adds a Burst item to priority queue with specified mode
void pq_add(struct Burst arr[], int n, struct Burst item, int mode);

// mode = 0 -> remove by arrival time
// mode = 1 -> remove by remaining time
// mode = 2 -> remove the first element
// removes and returns most prior item from priority queue
struct Burst pq_pop(struct Burst arr[], int n, int mode);

// b1 < b2 -> return -1
// b1 == b2 -> return 0
// b1 > b2 -> return 1
// mode = 0 -> compare by arrival time
// mode = 1 -> compare by remaining time
// compares two Burst
int compare_bursts(struct Burst b1, struct Burst b2, int mode);

// prints an array
void arr_print(struct Burst arr[], int n);

// The function used for FCFS and SJF
int non_preemptive(struct Burst inputArr[], int n, int mode);

int FCFS(struct Burst inputArr[], int n);
int SJF(struct Burst inputArr[], int n);
int SRTF(struct Burst inputArr[], int n);
int RR(struct Burst inputArr[], int quantum, int n);

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        printf("Invalid number of arguments!\n");
        return 1;
    }

    char* inputFile = argv[1];
    int quantum = atoi(argv[2]);


    if(quantum < 10 || quantum > 300)
    {
        printf("Error! Invalid Quantum.\n");
        return 1;
    }

    FILE* fptr;

    if ((fptr = fopen(inputFile,"r")) == NULL){
        printf("Error! File could not opened\n");
        exit(1);
    }

    struct Burst arr[1000];

    int totalNumberOfBursts = 0;
    int n, a, r;
    while( fscanf(fptr,"%d %d %d", &n, &a, &r) != EOF && totalNumberOfBursts < 1000)
    {
        arr[totalNumberOfBursts].no = n;
        arr[totalNumberOfBursts].arrivalTime = a;
        arr[totalNumberOfBursts].remainingTime = r;

        totalNumberOfBursts ++;
    }

    fclose(fptr);

    printf("FCFS %d\n", FCFS(arr, totalNumberOfBursts));
    printf("SJF  %d\n", SJF(arr, totalNumberOfBursts));
    printf("SRTF %d\n", SRTF(arr, totalNumberOfBursts));
    printf("RR   %d\n", RR(arr, totalNumberOfBursts, quantum));

    return 0;
}

void pq_add(struct Burst arr[], int n, struct Burst item, int mode)
{
    arr[n] = item;

    if(mode == 2) return;

    int current = n;

    while(current > 0)
    {
        int parent = (current - 1) / 2;

        if(compare_bursts(arr[current], arr[parent], mode) == -1)
        {
            struct Burst temp = arr[current];
            arr[current] = arr[parent];
            arr[parent] = temp;
        }
        else
            break;

        current = (current - 1) / 2;
    }
}

struct Burst pq_pop(struct Burst arr[], int n, int mode)
{
    struct Burst toRet = arr[0];

    if(n == 1)
        return toRet;

    if(mode == 2) {
        for(int i = 0; i < n - 1; i++) {
            arr[i] = arr[i + 1];
        }

        return toRet;
    }

    arr[0] = arr[n-1];

    n -= 1;
    int current = 0;
    while((current + 1) * 2 - 1 < n)
    {
        int left = (current + 1) * 2 - 1;
        int right = left + 1;

        int most = current;

        if(compare_bursts(arr[left], arr[current], mode) == -1)
            most = left;

        if(right < n && compare_bursts(arr[right], arr[most], mode) == -1)
            most = right;

        if(most == current)
            break;

        struct Burst temp = arr[most];
        arr[most] = arr[current];
        arr[current] = temp;

        current = most;
    }
    return toRet;
}

int compare_bursts(struct Burst b1, struct Burst b2, int mode)
{
    if(mode == 0)
    {
        if(b1.arrivalTime < b2.arrivalTime)
            return -1;
        else if(b1.arrivalTime > b2.arrivalTime)
            return 1;
        else
        {
            if(b1.remainingTime < b2.remainingTime)
                return -1;
            else if(b1.remainingTime > b2.remainingTime)
                return 1;
            else
            {
                if(b1.no < b2.no)
                    return -1;
                else if(b1.no > b2.no)
                    return 1;
                else
                    return 0;
            }
        }
    }
    else
    {
        if(b1.remainingTime < b2.remainingTime)
            return -1;
        else if(b1.remainingTime > b2.remainingTime)
            return 1;
        else
        {
            if(b1.arrivalTime < b2.arrivalTime)
                return -1;
            else if(b1.arrivalTime > b2.arrivalTime)
                return 1;
            else
            {
                if(b1.no < b2.no)
                    return -1;
                else if(b1.no > b2.no)
                    return 1;
                else
                    return 0;
            }
        }

    }
}

void arr_print(struct Burst arr[], int n)
{
    for(int i = 0; i < n; i ++)
        printf("%d %d %d\n", arr[i].no, arr[i].arrivalTime, arr[i].remainingTime);
    printf("\n");
}

int non_preemptive(struct Burst inputArr[], int n, int mode)
{
    struct Burst queue[n];

    int readyCount = 0;
    int time = 0;

    int noNew = 0;
    int isProcessing = 0;
    struct Burst processing;

    int turnaroundTime = 0;
    int finished = 0;

    int flag;

    while(finished < n)
    {
        flag = 0;

        // add new arrived Bursts to ready queue
        if(noNew == 0)
            for(int i = 0; i < n; i ++)
            {

                if(inputArr[i].arrivalTime >= time)
                    flag = 1;

                if(inputArr[i].arrivalTime == time)
                {
                    pq_add(queue, readyCount, inputArr[i], mode);
                    readyCount ++;
                }
            }

        if(flag == 0)
            noNew = 1;
        // update current process
        if(isProcessing)
        {
            processing.remainingTime --;
            if(processing.remainingTime == 0)
            {
                //printf("Process %d finished at time %d\n", processing.no, time);
                isProcessing = 0;
                turnaroundTime += time - processing.arrivalTime;
                //printf("turnaroundTime += %d\n", time - processing.arrivalTime);
                finished ++;
            }

        }

        // start to process new burst if possible
        if(!isProcessing && readyCount > 0)
        {

            isProcessing = 1;
            processing = pq_pop(queue, readyCount, mode);
            //printf("Process %d started at time %d\n", processing.no, time);
            readyCount--;
        }

        time ++;
    }

    //printf("turnaroundTime: %d\n", turnaroundTime);
    return round((float)turnaroundTime / (float)n);

}

int FCFS(struct Burst inputArr[], int n)
{
    return non_preemptive(inputArr, n, 0);
}

int SJF(struct Burst inputArr[], int n)
{
    return non_preemptive(inputArr, n, 1);
}

int SRTF(struct Burst inputArr[], int n)
{
    struct Burst queue[n];

    int readyCount = 0;
    int time = 0;

    int isProcessing = 0;
    struct Burst processing;

    int turnaroundTime = 0;
    int finished = 0;

    int flag;

    while(finished < n)
    {
        flag = 0;

        for(int i = 0; i < n; i++)
        {
            if(inputArr[i].arrivalTime == time)
            {
                flag = 1;
                pq_add(queue, readyCount, inputArr[i], 1);
                readyCount++;
            }
        }

        if(flag) {
            if(isProcessing) {
                //printf("Process %d paused processing at time %d, remaining time: %d\n", processing.no, time, processing.remainingTime);
                pq_add(queue, readyCount, processing, 1);
                readyCount++;
            }

            processing = pq_pop(queue, readyCount, 1);
            isProcessing = 1;
            readyCount--;

            //printf("Process %d started processing at time %d\n", processing.no, time);
        }

        // update current process
        if(isProcessing)
        {
            processing.remainingTime--;

            if(processing.remainingTime == 0)
            {
                //printf("Process %d finished at time %d\n", processing.no, time + 1);
                isProcessing = 0;
                turnaroundTime += time + 1 - processing.arrivalTime;
                //printf("turnaroundTime: %d\n", turnaroundTime);
                finished++;
            }
        }

        // start to process new burst if possible
        if(!isProcessing && readyCount > 0)
        {
            isProcessing = 1;
            processing = pq_pop(queue, readyCount, 1);

            //printf("Process %d started at time %d\n", processing.no, time + 1);
            readyCount--;
        }

        time++;
    }

    //printf("turnaroundTime: %d\n", turnaroundTime);
    return round((float) turnaroundTime / (float) n);
}

int RR(struct Burst inputArr[], int n, int quantum)
{
    struct Burst queue[n];

    int readyCount = 0;
    int time = 0;

    int isProcessing = 0;
    struct Burst processing;

    int turnaroundTime = 0;
    int finished = 0;

    int remainingQuantum;

    while(finished < n)
    {
        for(int i = 0; i < n; i++)
        {
            if(inputArr[i].arrivalTime == time)
            {
                pq_add(queue, readyCount, inputArr[i], 2);
                readyCount++;
            }
        }

        // start to process new burst if possible
        if(!isProcessing && readyCount > 0)
        {
            isProcessing = 1;
            processing = pq_pop(queue, readyCount, 2);
            remainingQuantum = quantum;

            //printf("Process %d started at time %d\n", processing.no, time);
            readyCount--;
        }

        // update current process
        if(isProcessing)
        {
            processing.remainingTime--;
            remainingQuantum--;

            if(processing.remainingTime == 0)
            {
                //printf("Process %d finished at time %d\n", processing.no, time + 1);
                isProcessing = 0;
                turnaroundTime += time + 1 - processing.arrivalTime;
                //printf("turnaroundTime: %d\n", turnaroundTime);
                finished++;
            }
            else if(remainingQuantum == 0) {
                //printf("Process %d paused at time %d\n", processing.no, time + 1);
                isProcessing = 0;
                pq_add(queue, readyCount, processing, 2);
                readyCount++;
            }
        }

        time ++;
    }

    //printf("turnaroundTime: %d\n", turnaroundTime);
    return round((float) turnaroundTime / (float) n);
}
