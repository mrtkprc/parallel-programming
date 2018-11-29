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
#include <math.h>
#include <pthread.h>
#include <time.h>

double *matrix;
int N=0;



int readFile(FILE **file,int *N,double **matrix);
void writeMatrix(const double *matrix, const int *N);
void divideMatrix(double **matrix,const int *N,double divider);
int openFile(FILE **file,const char *opening_mode,const char *file_name);

//
void divideMatrix(double **matrix,const int *N,double divider)
{
    int i,k;
    double val;
    for(i = 0; i < *N ;i++)
        for(k = 0; k < *N ;k++)
            *((*matrix)+(*N)*i+k) = (*((*matrix)+(*N)*i+k)) / divider;
}

void writeMatrix(const double *matrix, const int *N)
{
    int i,k;
    double val;
    for(i = 0; i < *N ;i++)
    {
        for(k = 0; k < *N ;k++)
        {
            val = *(matrix+(i*(*N)+k));
            printf("%f ",val);
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
            *((*matrix)+lineOrder-1) = line_val;
            lineOrder++;
        }
    }
    return 0;
}

int openFile(FILE **file,const char *opening_mode,const char *file_name)
{
    *file = fopen(file_name,"r");
    if(*file == NULL)
    {
        printf("File %s failed\n",file_name);
        return -1;
    }
    //printf("File %s opened\n",file_name);
    return 0;
}


void *each_child_thread(void *arg)
{
    int i = 0;
    int thread_order = (int)arg;
    char tmpLine[250];
    FILE *file_writing;
    snprintf(tmpLine,250,"%d",thread_order+1);
    strcat(tmpLine,".dat");
    file_writing = fopen((const char*)tmpLine,"w");

    for (i = thread_order * N; i < thread_order * N + N; i++)
    {
        if (file_writing == NULL) 
        {
            perror("fopen");
            
            exit(1);
        }
        fprintf(file_writing,"%f\n",pow(matrix[i],0.5));
        fflush(file_writing);
    }
    
    fclose(file_writing);
    pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
    int i,k;
    clock_t start,end;
    char *line;
    int read;
    size_t len;
    char tmpLine[250];
    int my_order = -1;
    FILE *file;
    FILE *file_writing;
    
    double *each_child_row;
    double val;
    pthread_t *thread_list;
    pthread_attr_t attr;
    int rc;
    void *status;

    if(argc != 2)
    {
        printf("Please, use following format\n");
        printf("<./exe_name> <source dat file>\n");
        return EXIT_FAILURE;
    }

    start = clock();
    
    openFile(&file,"r",argv[1]);
    
    if(!file)
    {
		printf("Input file cannot be opened\n");
		exit(1);
    }
    readFile(&file,&N,&matrix);
    fclose(file);

    //writeMatrix(matrix,&N);

    divideMatrix(&matrix,&N,255.0);
    //writeMatrix(matrix,&N);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    thread_list = (pthread_t*)malloc(sizeof(pthread_t)* N);
    
    
    for (i = 0; i < N; i++)
    {
        rc = pthread_create((thread_list+i),&attr,each_child_thread,(void *)i);
        if(rc)
        {
            printf("Error in creating thread\n");
            return EXIT_FAILURE;
        }
    }
    pthread_attr_destroy(&attr);
    for (i = 0; i < N; i++)
    {
        rc = pthread_join(*(thread_list+i),&status);
        if(rc)
        {
            printf("Error in join thread\n");
            return EXIT_FAILURE;
        }
    }
    file_writing = fopen("result_multi_thrd.dat","w");
    if (file_writing == NULL) 
    {
        perror("fopen");
        exit(1);
    }
    
    for (i = 0; i < N; i++)
	{
        snprintf(tmpLine,250,"%d",i+1);
        strcat(tmpLine,".dat");
        file = NULL;
        openFile(&file,"r",tmpLine);
        if(file == NULL)   
        {
            printf("file opening error\n");
            return EXIT_FAILURE;
        }
        line = NULL;
        len = 0;
        while((read = getline(&line,&len,file)) != -1)
	    {
            if(!strcmp(line,"\n"))
                break;
            fprintf(file_writing,"%s",line);
            fflush(file_writing);
        }
        fclose(file);
    }
    
    fclose(file_writing);
    end = clock();
    printf("The Elapsed Time: %f\n",(((double) (end - start)) / CLOCKS_PER_SEC));
    pthread_exit(NULL);

    return 0;
}