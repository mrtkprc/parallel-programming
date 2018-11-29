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

#define OPENING_FILE "qr_basic_3.dat"

#define SHMCHILDLIST 1

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
    printf("File %s opened\n",file_name);
    return 0;
}

int main (int argc, char *argv[])
{
    int i,k;
    char *line;
    int read;
    size_t len;
    char tmpLine[250];
    int my_order = -1;
    FILE *file;
    FILE *file_writing;
    int N=0;
    double *matrix;
    pid_t f;
    pid_t *child_creation_list;
    int shm_children_list_id; //shared memory children list id
    double *each_child_row;
    double val;

    openFile(&file,"r",OPENING_FILE);
    
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

    shm_children_list_id = shmget(SHMCHILDLIST,sizeof(pid_t)*N,0700|IPC_CREAT);
    child_creation_list =(pid_t *)shmat(shm_children_list_id,0,0);

    each_child_row = (double *)malloc(sizeof(double)*N);

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
        file_writing = fopen("result.dat","w");
        if (file_writing == NULL) 
        {
            perror("fopen");
            exit(1);
        }
        for (i = 0; i < N; i++)
	    {
            snprintf(tmpLine,250,"%d",i+1);
            strcat(tmpLine,".dat");
            openFile(&file,"r",tmpLine);
           
            while((read = getline(&line,&len,file)) != -1)
	        {
                if(!strcmp(line,"\n"))
                    break;
                fprintf(file_writing,"%s",line);
                //printf("Each file :%s");
                fflush(file_writing);
            }
            fclose(file);
        }
        fclose(file_writing);
        
        printf("The End\n");
        shmdt(child_creation_list);
        shmctl(shm_children_list_id,IPC_RMID,0);
    }
    else
    {
        printf("My Process ID: %u and Parent ID : %u\n",getpid(),getppid());
        for (i = 0; i < N; i++)
            if (child_creation_list[i] == getpid())
                my_order = i;
        
        for (i = 0; i < N; i++)
            *(each_child_row + i) = pow(matrix[my_order*N+i],0.5);

        snprintf(tmpLine,250,"%d",my_order+1);
        strcat(tmpLine,".dat");
        printf("file name: %s\n",tmpLine);
        file_writing = fopen((const char*)tmpLine,"w");
        if (file_writing == NULL) 
        {
            perror("fopen");
            exit(1);
        }
        
        for (i = 0; i < N; i++)
            fprintf(file_writing,"%f\n",*(each_child_row + i));
        
        fflush(file_writing);
        fclose(file_writing);

        sleep(1);
        wait(0);
    }
}