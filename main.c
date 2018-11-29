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

//
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
    FILE *file;
    int N=0;
    double *matrix;
    file = fopen(OPENING_FILE,"r");

    if(!file)
    {
		printf("Input file cannot be opened\n");
		exit(1);
    }
    readFile(&file,&N,&matrix);
    writeMatrix(matrix,&N);
}