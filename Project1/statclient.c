#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct item
{
	int id;
	char command[999];
};

struct response
{
    int id;
    int number;
};

int main()
{
    mqd_t mq;
    mq = mq_open("/mq", O_RDWR);

    mqd_t mqReceive;
    int n;
    int idTest = 0;
    char* catS;
    int buflen = 99999;
    char* bufptr = (char *) malloc(buflen);
    while(1)
    {
        char str1[5] = "/mqr";
        char idStr[10];
        sprintf(idStr, "%d", idTest);

        //char* catS = " ";
        catS = strcat(str1, idStr );

        mqReceive = mq_open(catS, O_RDWR);
        if(mqReceive == -1)
            break;

        idTest ++;
        mq_close(mqReceive);
    }

    mqReceive = mq_open(catS, O_RDWR | O_CREAT, 0666, NULL);

    if (mqReceive == -1) {
		perror("can not create msg queue\n");
		exit(1);
	}

    while(1)
    {
        char command[999];
        printf("Enter your command: ");

        scanf ("%[^\n]%*c", command);

        command[strcspn(command, "\n")] = 0;

        if(strcmp(command, "q") == 0)
            break;

        struct item item;
        item.id = idTest;
        strcpy(item.command, command);

        mq_send(mq, (char*) &item, sizeof(struct item), 0);



        printf("Response:\n");



        while(1)
        {
            n =  mq_receive(mqReceive, (char *) bufptr, buflen, NULL);
            if (n == -1) {
            	perror("mq_receive failed\n");
            	exit(1);
            }

            struct response *responseptr;

            responseptr = (struct response *) bufptr;

            if(responseptr->id == -2)
            {
                printf("Invalid command!\n");
                break;
            }
            printf("%d\n", responseptr->number);

            if(responseptr->id == -1)
                break;
        }

        printf("\n");


    }

    mq_close(mqReceive);
    free(bufptr);
    return 0;
}
