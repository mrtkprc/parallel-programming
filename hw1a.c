#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <time.h>

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
    return 0;
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
    int N=0;
    double *matrix;
    pid_t f;
    pid_t *child_creation_list; //shared memory area used among all process
    int shm_children_list_id; //shared memory children list id
    double *each_child_row; // each process has own row
    double val;

    if(argc != 2)
    {
        printf("Please, use following format\n");
        printf("<./exe_name> <source dat file>\n");
        return EXIT_FAILURE;
    }

    start = clock();
    //Open the the data file supplied with command line
    openFile(&file,"r",argv[1]);
    
    if(!file)
    {
		printf("Input file cannot be opened\n");
		exit(1);
    }
    //read N number and input file
    readFile(&file,&N,&matrix);
    fclose(file);
    // Each element of Matrix Divided by 255
    divideMatrix(&matrix,&N,255.0);

    //To get request for shared memory area
    shm_children_list_id = shmget(SHMCHILDLIST,sizeof(pid_t)*N,0700|IPC_CREAT);
    //Shared memory attached 
    child_creation_list =(pid_t *)shmat(shm_children_list_id,0,0);
    //Dynamic memory area which each process has posses
    each_child_row = (double *)malloc(sizeof(double)*N);

    //Children are being created
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
        wait(0); // until every child terminate itself, wait 
        //All children processes completed own execution
        //and read related files and write to file result dat file
        file_writing = fopen("result_multi_proc.dat","w");
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
            line = NULL;
            len = 0;
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
        end = clock();
        printf("The Elapsed Time: %f\n",(((double) (end - start)) / CLOCKS_PER_SEC));
        
        shmdt(child_creation_list);
        shmctl(shm_children_list_id,IPC_RMID,0);
    }
    else
    {
        //Each process handle own part of matrix based creation order
        for (i = 0; i < N; i++)
            if (child_creation_list[i] == getpid())
                my_order = i;
        
        for (i = 0; i < N; i++)
            *(each_child_row + i) = pow(matrix[my_order*N+i],0.5);

        snprintf(tmpLine,250,"%d",my_order+1);
        strcat(tmpLine,".dat");
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