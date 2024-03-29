#include <sys/types.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <time.h>

#define OPENING_FILE "qr_code_64x64_grayscale.dat"


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
    double *data_list;
    double val;

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

    for (i = 0; i < N * N; i++)
            matrix[i] = pow(matrix[i],0.5);

    file_writing = fopen("result_single_proc.dat","w");
    if (file_writing == NULL) 
    {
        perror("fopen");
        exit(1);
    }
    for (i = 0; i < N * N; i++)
        fprintf(file_writing,"%f\n",matrix[i]);

    fclose(file_writing);
    end = clock();

    printf("The Elapsed Time: %f\n",(((double) (end - start)) / CLOCKS_PER_SEC));
}
