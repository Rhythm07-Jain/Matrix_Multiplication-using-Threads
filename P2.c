#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>

// #define a 50
// #define n 20
// #define b 50
#define N 5

int a,n,b;
char *in1,*in2,*out;
int bufId;
int (*bufferPtr)[2000];
FILE *output;

void createSHM()
{
  bufId = shmget((key_t)1234, sizeof(long long int)*40000000, 0666|IPC_CREAT);

  if(bufId == -1 )
  {  
    perror("shmget");
    exit(1);
  }
  else
  {  
    // printf("Shared memory space created \n");
    bufferPtr = shmat(bufId,0,0);
    if(bufferPtr == (void*) -1 )
    {  
      perror("shmat");
      exit(1);
    }
  }  
}

int step = 0;
long long int ans[1000][1000];

void* mul(void* arg){
    int *numTemp = (int*)arg;
    int num = *numTemp;
    int i, j, k;
    for(i=0; i<a; i++){
        for(j=0; j<b;j++){
            ans[i][j]=0;
            for(k=0; k<n; k++){     
                ans[i][j]+=bufferPtr[i][k]*bufferPtr[a+j][k];
            }
        }
    }
}



int main(int argc, char *argv[])
{
    // printf("HEREEEEEEE-----> %d\n", getppid());
    a=atoi(argv[0]);n=atoi(argv[1]);b=atoi(argv[2]);
    in1=argv[3];in2=argv[4];out=argv[5];
    createSHM();
    pthread_t tid[N];
    usleep(150000);    
    for(int i=0; i<N; i++){
        int *p;
        pthread_create(&tid[i], NULL, mul, (void *)(&i));
        pthread_join(tid[i], NULL);
    }
    
    // printf("Multiplication Result: \n");
    // for(int i=0; i<a; i++){
    //     for(int j=0; j<b; j++){
    //         printf("%lld ", ans[i][j]);
    //     }
    //     printf("\n");
    // }
    output = fopen(out,"a");
    output = fopen(out,"w");
    for(int i=0; i<a; i++){
        for(int j=0; j<b; j++){
            fprintf(output, "%lld ", ans[i][j]);
        }
        fprintf(output, "\n");
    }
    shmctl(bufId, IPC_RMID, NULL);
    kill(getppid(), SIGUSR2);
   
    return 0;
}