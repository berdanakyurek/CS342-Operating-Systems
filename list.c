#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

struct Node {
    int value;
    struct Node* next;
};

// adds n at the end of linked list with head
void addToLinkedList(struct Node** head, int n)
{
    struct Node* new = (struct Node*)malloc(sizeof(struct Node));

    new -> next = NULL;
    new -> value = n;

    if(*head == NULL)
    {
        *head = new;
        return;
    }

    struct Node* current = *head;

    while(current->next != NULL)
        current = current->next;

    current -> next = new;
}

void printLinkedList(struct Node* head)
{
    if(head == NULL)
    {
        printf("Empty linked list.\n");
        return;
    }

    while(head != NULL)
    {
        printf("%d\n", head->value);
        head = head->next;
    }

}

int main()
{
    struct Node* head = NULL;
    srand(time(NULL));

    struct timeval beforeAll;
    gettimeofday(&beforeAll, NULL);

    for(int i = 0; i < 10000; i ++)
    {
        struct timeval beforei;
        gettimeofday(&beforei, NULL);

        addToLinkedList(&head, rand() % 101);

        struct timeval afteri;
        gettimeofday(&afteri, NULL);

        printf("Execution of adding node %d took %ld seconds and %ldmicroseconds\n", i
           , afteri.tv_sec - beforei.tv_sec
           , (((afteri.tv_sec - beforei.tv_sec) * 1000000)
              + afteri.tv_usec) - (beforei.tv_usec));
    }

    struct timeval afterAll;
    gettimeofday(&afterAll, NULL);

    //printLinkedList(head);

    printf("All execution took %ld seconds and %ld microseconds\n"
           , afterAll.tv_sec - beforeAll.tv_sec
           , (((afterAll.tv_sec - beforeAll.tv_sec) * 1000000)
              + afterAll.tv_usec) - (beforeAll.tv_usec));

    return 0;
}
