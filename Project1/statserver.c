#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <mqueue.h>
#include <time.h>
#include <sys/time.h>

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

void tokenize(char* command, char* tokens[4])
{
    for(int i = 0; i < 4; i ++)
    {
        tokens[i] = malloc(999);
        strcpy(tokens[i],"");
    }

    char string[999];

    strcpy(string, command);
    char * token = strtok(string, " ");
    int i = 0;
    while( token != NULL )
    {
        if(i >= 4)
            break;

        strcpy(tokens[i], token);
        token = strtok(NULL, " ");
        i ++;
    }
    //printf("%s", command);
}

// Validates a command and determines its type
// count: 1
// avg: 2
// max: 3
// count <start> <end>: 4
// avg <start> <end>: 5
// range <start> <end> <K>: 6
// invalid command: -1
int validateCommand(char* tokens[4])
{
    int start, end, k;

    start = atoi(tokens[1]);
    end = atoi(tokens[2]);
    k = atoi(tokens[3]);

    if(strcmp(tokens[0], "count") == 0 && strcmp(tokens[1], "") == 0)
        return 1;
    if(strcmp(tokens[0], "avg") == 0 && strcmp(tokens[1], "") == 0)
        return 2;
    if(strcmp(tokens[0], "max") == 0 && strcmp(tokens[1], "") == 0)
        return 3;

    start = atoi(tokens[1]);
    end = atoi(tokens[2]);
    k = atoi(tokens[3]);

    if(start == 0 && tokens[1][0] != '0')
        return -1;
    if(end == 0 && tokens[2][0] != '0')
        return -1;
    if(start > end)
        return -1;

    if(strcmp(tokens[0], "count") == 0)
    {
        if(strcmp(tokens[3], "") != 0)
            return -1;
        return 4;
    }
    else if(strcmp(tokens[0], "avg") == 0)
    {
        if(strcmp(tokens[3], "") != 0)
            return -1;
        return 5;
    }
    else if(strcmp(tokens[0], "range") == 0)
    {
        if(k <= 0)
            return -1;
        return 6;
    }
    else
        return -1;
    return -1;
}

int main(int argc, char* argv[])
{

    if(argc <= 2)
    {
        printf("You must specify at least one file.\n");
        return -1;
    }

    int docCount = atoi(argv[1]);

    if(docCount != argc - 2)
    {
        printf("Given file count and argument count does not match!\n");
        return -1;
    }
    int forkType;
    char* fileName = "";

    char *bufptr;
    int buflen;
    mqd_t mq;

    int nmq;

    struct item *itemptr;

    mq = mq_open("/mq", O_RDWR | O_CREAT, 0666, NULL);

    buflen = 99999;
    //buflen = mq_attr.mq_msgsize;

    bufptr = (char *) malloc(buflen);
    //printf("buflen %d\n", buflen);

    if (mq == -1) {
        perror("can not create msg queue\n");
        exit(1);
    }

    while(1)
    {
        nmq = mq_receive(mq, (char *) bufptr, buflen, NULL);
		if (nmq == -1) {
			perror("mq_receive failed\n");
			exit(1);
		}

		printf("mq_receive success, message size = %d\n", nmq);

        struct timeval begin, endT;
        gettimeofday(&begin, 0);

        itemptr = (struct item *) bufptr;

		printf("received item->id = %d\n",itemptr->id );
        printf("received item->command = %s\n", itemptr->command);
		printf("\n");

        int idTest = itemptr->id;

        char* command = itemptr->command;

        char* tokens[4];

        tokenize(command, tokens);

        int start, end, k = 0;

        //for(int i = 0; i < 4; i ++)
        //    printf("+%s+\n", tokens[i]);

        int commandType = validateCommand(tokens);

        if(commandType >= 4)
        {
            start = atoi(tokens[1]);
            end = atoi(tokens[2]);

        }
        if(commandType == 6)
            k = atoi(tokens[3]);

        int pipes[docCount][2];
        int pipeNo = -1;

        //return 0;
        char idStr[10];
        char str1[5] = "/mqr\0";
        sprintf(idStr, "%d", idTest);

        //char* catS = " ";
        mqd_t mqReceive;
        char* catS;

        catS = strcat(str1, idStr );

        mqReceive = mq_open(catS, O_RDWR);

        forkType = 1;
        for(int i = 0; i < docCount; i ++)
            if(forkType > 0)
            {
                pipe(pipes[i]);
                forkType = fork();
                if(forkType == 0)
                {
                    fileName = argv[i+2];
                    pipeNo = i;
                }
            }


        if(forkType == 0) // children
        {
            FILE* f;
            f = fopen(fileName,"rb");
            int nextInt = 0;

            switch(commandType)
            {

            case 1:
            {
                int count = 0;
                while (fscanf(f,"%d",&nextInt) != EOF)
                    count ++;

                close(pipes[pipeNo][0]);
                write(pipes[pipeNo][1],&count, sizeof(count));
                close(pipes[pipeNo][1]);
                break;
            }

            case 2:
            {
                int count = 0;
                int sum = 0;
                while (fscanf(f,"%d",&nextInt) != EOF)
                {
                    count ++;
                    sum += nextInt;
                }

                close(pipes[pipeNo][0]);
                write(pipes[pipeNo][1],&count, sizeof(count));
                write(pipes[pipeNo][1],&sum, sizeof(sum));
                close(pipes[pipeNo][1]);
                break;
            }

            case 3:
            {
                int max = 0;
                int flag = 0;

                while (fscanf(f,"%d",&nextInt) != EOF)
                    if(flag == 0 || nextInt > max)
                    {
                        flag = 1;
                        max = nextInt;
                    }

                close(pipes[pipeNo][0]);
                write(pipes[pipeNo][1],&max, sizeof(max));
                close(pipes[pipeNo][1]);
                break;
            }

            case 4:
            {
                int count = 0;
                while (fscanf(f,"%d",&nextInt) != EOF)
                    if(nextInt >= start && nextInt <= end)
                    count ++;

                close(pipes[pipeNo][0]);
                write(pipes[pipeNo][1],&count, sizeof(count));
                close(pipes[pipeNo][1]);
                break;
            }

            case 5:
            {
                int count = 0;
                int sum = 0;
                while (fscanf(f,"%d",&nextInt) != EOF)
                {
                    if(nextInt >= start && nextInt <= end)
                    {
                        count ++;
                        sum += nextInt;
                    }
                }

                close(pipes[pipeNo][0]);
                write(pipes[pipeNo][1],&count, sizeof(count));
                write(pipes[pipeNo][1],&sum, sizeof(sum));
                close(pipes[pipeNo][1]);
                break;
            }

            case 6:
            {
                int itemCount = 0;
                int queue[k];

                int i = 0;


                while (fscanf(f,"%d",&nextInt) != EOF)
                {
                    if(itemCount < k && nextInt >= start && nextInt <= end)
                    {
                        queue[itemCount] = nextInt;
                        int j = itemCount;

                        while(itemCount > 0 && queue[j] > queue[j - 1])
                        {
                            int temp = queue[j];
                            queue[j] = queue[j - 1];
                            queue[j - 1] = temp;
                            j --;
                        }

                        itemCount ++;
                    }
                    else if(queue[k-1] < nextInt && nextInt >= start && nextInt <= end )
                    {
                        queue[k-1] = nextInt;
                        int j = k-1;

                        while(itemCount > 0 && queue[j] > queue[j - 1])
                        {
                            int temp = queue[j];
                            queue[j] = queue[j - 1];
                            queue[j - 1] = temp;
                            j --;
                        }
                    }
                    i ++;
                }

                close(pipes[pipeNo][0]);

                int terminate = -1;



                for(i = 0; i < itemCount; i ++)
                    write(pipes[pipeNo][1],&queue[i], sizeof(queue[i]));

                write(pipes[pipeNo][1],&terminate, sizeof(queue[i]));
                close(pipes[pipeNo][1]);

                break;
            }

            case -1:
                printf("Invalid command\n");
                break;
            }

            fclose(f);
            exit(0);
        }
        else if(forkType > 0) // parent
        {
            for (int i = 0; i < docCount; i++)
                waitpid(-1, NULL, 0);

            switch(commandType)
            {

            case 1: case 4:
            {
                int pipeResult;
                int count = 0;
                for(int i = 0; i < docCount; i ++)
                {
                    close(pipes[i][1]);
                    read(pipes[i][0], &pipeResult, sizeof(pipeResult));
                    close(pipes[i][0]);
                    count += pipeResult;
                }

                struct response resp;

                resp.id = -1;
                resp.number = count;
                mq_send(mqReceive, (char*) &resp, sizeof(struct response), 0);
                printf("count=%d\n", count);
                break;
            }
            case 2: case 5:
            {
                int pipeResult;
                int sum = 0;
                int count = 0;
                for(int i = 0; i < docCount; i ++)
                {
                    close(pipes[i][1]);
                    read(pipes[i][0], &pipeResult, sizeof(pipeResult));
                    count += pipeResult;

                    read(pipes[i][0], &pipeResult, sizeof(pipeResult));
                    sum += pipeResult;

                    close(pipes[i][0]);
                }

                double avg = 0;
                if(count != 0)
                    avg = (double)sum/(double)count;


                struct response resp;

                resp.id = -1;
                resp.number = avg;
                mq_send(mqReceive, (char*) &resp, sizeof(struct response), 0);

                printf("avg=%f\n", avg);
                break;
            }
            case 3:
            {
                int pipeResult;
                int max;
                for(int i = 0; i < docCount; i ++)
                {
                    close(pipes[i][1]);
                    read(pipes[i][0], &pipeResult, sizeof(pipeResult));
                    close(pipes[i][0]);

                    if(i == 0 || pipeResult > max)
                        max = pipeResult;
                }
                printf("max=%d\n", max);

                struct response resp;

                resp.id = -1;
                resp.number = max;
                mq_send(mqReceive, (char*) &resp, sizeof(struct response), 0);
                break;
            }
            case 6:
            {
                printf("%d %d %d\n", start, end, k);

                int pipeResult;

                int childResults[docCount][k];
                int cursors[docCount];

                int resultArray[k];



                for(int i = 0; i < docCount; i ++)
                {
                    close(pipes[i][1]);

                    cursors[i] = 0;

                    for(int j = 0; j < k; j ++)
                    {
                        read(pipes[i][0], &pipeResult, sizeof(pipeResult));
                        childResults[i][j] = pipeResult;

                        if(j == k - 1 || pipeResult == -1)
                        {
                            close(pipes[i][0]);
                            break;
                        }
                    }
                }

                int resultArraySize = 0;

                for(int i = 0; i < k; i ++)
                {
                    int flag = 0;
                    int max = 0;
                    int maxIndex = 0;

                    for(int j = 0; j < docCount; j ++)
                    {
                        if(cursors[j] == k || childResults[j][cursors[j]] == -1 )
                            continue;

                        if( flag == 0 || childResults[j][cursors[j]] > max)
                        {
                            maxIndex = j;
                            max = childResults[j][cursors[j]];
                            flag = 1;
                        }
                    }

                    if(flag == 0)
                    {
                        resultArraySize = i;
                        break;
                    }

                    cursors[maxIndex] ++;
                    resultArray[i] = max;
                    resultArraySize ++;
                }

                for(int i = resultArraySize - 1; i >= 0; i -- )
                {
                    printf("%d \n", resultArray[i]);
                    struct response resp;

                    if(i == 0)
                        resp.id = -1;
                    else
                        resp.id = 0;
                    resp.number = resultArray[i];
                    mq_send(mqReceive, (char*) &resp, sizeof(struct response), 0);
                }
                break;
            }
            default:
                struct response resp;

                    resp.id = -2;
                    resp.number = -2;
                    mq_send(mqReceive, (char*) &resp, sizeof(struct response), 0);
                break;
            }
        }

        for(int i = 0; i < 4; i ++)
            free(tokens[i]);

        mq_close(mqReceive);

        gettimeofday(&endT, 0);
        long seconds = endT.tv_sec - begin.tv_sec;
        long microseconds = endT.tv_usec - begin.tv_usec;
        double elapsed = seconds + microseconds*1e-6;

        printf("Time measured: %.3f seconds.\n", elapsed);

        printf("\n");

    }
}
