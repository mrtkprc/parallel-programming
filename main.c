#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define OPENING_FILE "qr_basic_3.dat"

int readFile(FILE **file,int *N,double **matrix);
void writeMatrix(const double *matrix, const int *N);
void divideMatrix(double **matrix,const int *N,double divider);

//
void divideMatrix(double **matrix,const int *N,double divider)
{
    int i,k;
    double val;
    for(i = 0; i < *N ;i++)
        for(k = 0; k < *N ;k++)
            *((*matrix)+((i+1)*k)) = (*((*matrix)+((i+1)*k))) / divider;
}

void writeMatrix(const double *matrix, const int *N)
{
    int i,k;
    double val;
    for(i = 0; i < *N ;i++)
    {
        for(k = 0; k < *N ;k++)
        {
            val = *(matrix+((i+1)*k));
            printf("%.2f ",val);
        }
        printf("\n");
    }
}

int readFile(FILE **file,int *N,double **matrix)
{
    char *line,tmpLine[256];
    int lineOrder = 0;
	int order = 0;
	size_t len = 0;
	int read; 
    char *foundNewLine;
    double line_val;
    int N_number;

    while((read = getline(&line,&len,*file)) != -1)
	{
		if(lineOrder == 0) 
		{
            N_number = atoi(line);
			lineOrder++;
            *matrix = (double *)malloc(sizeof(double) * N_number * N_number);
            memset(*matrix,0,sizeof(double)*N_number*N_number);
            *N = N_number;
		}
        else
        {
            foundNewLine = strstr(line,"\n");
            if(foundNewLine != NULL)
				strncpy(foundNewLine,"",1); // new line character is cleared
            strcpy(tmpLine,(const char*)line);
            line_val = atof(tmpLine);

            *(*matrix+(lineOrder-1)) = line_val;
            lineOrder++;
        }
    }
    return 0;
}


int main (int argc, char *argv[])
{
    int i;
    FILE *file;
    int N=0;
    double *matrix;
    pid_t f;
    pid_t *child_creation_list;

    file = fopen(OPENING_FILE,"r");
    if(!file)
    {
		printf("Input file cannot be opened\n");
		exit(1);
    }
    readFile(&file,&N,&matrix);
    //writeMatrix(matrix,&N);
    divideMatrix(&matrix,&N,255.0);

    child_creation_list = (pid_t*)malloc(sizeof(pid_t) * N);
    
    for (i = 0; i < N; i++)
	{
		f = fork();
		if(f<0)
		{
			printf("Process Creation Error\n");
			sleep(2);
			exit(1);
		}
		else if (f>0)
            child_creation_list[i] = f; //Add child pid in order
		else
            break; //So that other children exit
	}

    if(f!=0) // Parent Proces
	{
        wait(0);
        printf("Parent Process ID: %u\n",getpid());
    }
    else
    {
        printf("My Process ID: %u and Parent ID : %u\n",getpid(),getppid());
        sleep(1);
    }



}