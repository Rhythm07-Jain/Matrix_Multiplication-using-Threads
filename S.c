#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <wait.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/time.h>
#include <signal.h>

pid_t pids[2];
int statusP1=1;
int statusP2=1;
float tatP1 = 0;
float tatP2 = 0;
float timeForOnlyP1 = 0;
float waitTimeP1 = 0;
float waitTimeP2 = 0;

long long int act_time(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

void signalHandler(int signalNum){
	if (signalNum == SIGUSR1) {
    printf("Caught SIGUSR1 signal.\n");
    statusP1 = 0;
  }
  if (signalNum == SIGUSR2) {
    printf("Caught SIGUSR2 signal.\n");
    statusP2 = 0;
  }
}
	long long TotalTime = 0;
	  int counter =0;

void* ThreadFuncRR() {
	// int *p2 = (int*) z;
	// int p = *p2;
  long long int timeQuantum = 1000;
  int lastProcess = 0;
  long long int startTime = act_time()/1000.0;
  long long int lastSwitched = act_time();
  

  while(statusP1 || statusP2) {
  	// printf("Hello\n");
  	if(statusP1){
	    while(act_time() - lastSwitched < timeQuantum);
	    struct timespec t0, t1;
    
	  	if (timespec_get (&t0, TIME_UTC) != TIME_UTC){
	      printf ("Error in calling timespec_get\n");
	      exit (EXIT_FAILURE);
	  	}
    
	    tatP1 = tatP1 + timeQuantum/1000;
	    if(lastProcess==0){
	    	timeForOnlyP1 = timeForOnlyP1 + timeQuantum/1000;
	    }
	    if(statusP2){
	    	tatP2 = tatP2 + timeQuantum/1000;
	    }
	    long long int tmp = act_time();
	    kill(pids[lastProcess], SIGSTOP);
	    kill(pids[!lastProcess], SIGCONT);
	    lastProcess = !lastProcess;
	    lastSwitched = tmp;
	    if (timespec_get (&t1, TIME_UTC) != TIME_UTC)
	    {
	      printf ("Error in calling timespec_get\n");
	      exit (EXIT_FAILURE);
	    }
    
    	long long  diff = (long long )(t1.tv_sec - t0.tv_sec)*1000000000 + ((long long )(t1.tv_nsec - t0.tv_nsec));
   		// printf("Elapsed time: %lld seconds\n", diff);
   		
	   	TotalTime += diff;
	   counter++;
	   
	   
	   
	}
	else{
		kill(pids[1], SIGCONT);
	}
	// if(lastProcess)
	// 	printf("Process 2\n");
	// else
	// 	printf("Process 1\n");
    }
	long long int endTime = act_time()/1000.0;

    float onlyP2 = (float)(act_time() - lastSwitched)/1000.0;
    float temp = tatP2;
    tatP2 = tatP2 + onlyP2;
    waitTimeP1 = tatP1/2;
    waitTimeP2 = timeForOnlyP1;

    printf("waitTimeP1: %f\n", waitTimeP1);
    printf("waitTimeP2: %f\n", waitTimeP2);
}

int z;

void RR(char *argv[]) {
    pid_t p = fork();
	
    if(p == 0) {
        pids[0] = getpid();                     // 1st child process
		// printf("p1 = %d\n", pids[0]);		
		p = pids[0];
		execl("proc1", argv[1],argv[2],argv[3],argv[4],argv[5],argv[6], NULL); 
    } 
	else {
        p = fork();
        if(p == 0) {
            pids[1] = getpid();
			// printf("p2 = %d\n", pids[1]);
			p = pids[1];
			z = pids[1];
			execl("proc2", argv[1],argv[2],argv[3],argv[4],argv[5],argv[6], NULL);                  // 2nd child process
        }
		else{
            p = getpid();
			pthread_t timer;
			pthread_create(&timer, NULL, ThreadFuncRR, NULL);
			pthread_join(timer, NULL);
			wait(NULL);
			wait(NULL);       
        }
    }
}

int main(int argc, char *argv[]) {

	signal(SIGUSR1, signalHandler);
	signal(SIGUSR2, signalHandler);
	// printf("HEREEEEEEE SCHEDULER-----> %d\n", getpid());
    RR(argv);
    printf("tat1: %f\n", tatP1);
    printf("tat2: %f\n", tatP2);
    long long int average = TotalTime/counter ;
    printf("Average switching time : %lld nanoseconds\n", average);
    return 0;
}
