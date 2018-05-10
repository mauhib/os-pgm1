#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <stdlib.h>

#define SEM_S_NAME "sems"
#define SEM_N_NAME "semn"
#define SHM_NAME "sharedmem"
#define SHM_SIZE 80000 //Items

#define DELAY_OF_OPERATIONS 0
#define MAX_ITEMS 20000

#define WAIT_WAY 1
#define DEBUG 0

int32_t *ptr;
sem_t *s;
sem_t *n;

/*sem_wait(&mutex);
sem_post(&mutex);*/

void print_double_array(double *arr, size_t len, char* filename){
  FILE *fp;
  fp = fopen(filename, "w+");
  if(!fp){
    printf("Could not open file.\n");
    return;
  }
  int i;
  for(i=0;i<len;i++){
    fprintf(fp, "%.20f\n",arr[i]);
  }
  fclose(fp);
}

void print_int_array(int32_t *arr, size_t len, char* filename){
  FILE *fp;
  fp = fopen(filename, "w+");
  int i;
  for(i=0;i<len;i++){
    fprintf(fp, "%d\n",arr[i]);
  }
  fclose(fp);
}

void producer(char* file_time, char* file_n){
  printf("Producer Started\n");
  //Allocate storage for results
  double *time_storage = calloc(MAX_ITEMS,sizeof(double));
  if(!time_storage){
    printf("Couldn't allocate time_storage\n");
    return;
  }
  int32_t *number_storage = calloc(MAX_ITEMS,sizeof(int32_t));
  if(!number_storage){
    printf("Couldn't allocate number_storage\n");
    return;
  }
  //Done allocating
  struct timespec start,end;
  int32_t produced_value = 0;
  srand(time(0));
  for(ptr[0]=MAX_ITEMS;ptr[0]>0;ptr[0]--){
    clock_gettime(CLOCK_MONOTONIC, &start); //start clock
    produced_value = rand();
#if DEBUG
    printf("Producer: produce() => %d \n", produced_value);
#endif
#if WAIT_WAY == 0
    sem_wait(s);
#else
    while(sem_trywait(s) < 0){
    }
#endif
    sem_getvalue(n,&(number_storage[MAX_ITEMS - ptr[0]]));
    if(DELAY_OF_OPERATIONS) sleep(DELAY_OF_OPERATIONS);
#if DEBUG
    printf("Producer: append(); n = %d\n",number_storage[MAX_ITEMS - ptr[0]]);
#endif
    ptr[number_storage[MAX_ITEMS - ptr[0]]+1] = produced_value;
    sem_post(s);
    sem_post(n);
    clock_gettime(CLOCK_MONOTONIC, &end); //end clock
    time_storage[MAX_ITEMS - ptr[0]] = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/10e9);
  }
  //Output results
  printf("Producer: Outputting Time\n");
  print_double_array(time_storage, MAX_ITEMS, file_time);
  printf("Producer: Outputting Items in Array\n");
  print_int_array(number_storage, MAX_ITEMS, file_n);
  //Done Outputting
  //Clean allocation
  free(time_storage);
  free(number_storage);
  //Clean Done
  printf("Producer Exitting\n");
}

void consumer(char* file_time, char* file_n){
  printf("Consumer Started\n");
  //Allocate storage for results
  double *time_storage = calloc(MAX_ITEMS,sizeof(double));
  if(!time_storage){
    printf("Couldn't allocate time_storage\n");
    return;
  }
  int32_t *number_storage = calloc(MAX_ITEMS,sizeof(int32_t));
  if(!number_storage){
    printf("Couldn't allocate number_storage\n");
    return;
  }
  //Done allocating
  struct timespec start,end;
  int32_t taken_value;
  int count;
  for(count = 0; ; count++){
    clock_gettime(CLOCK_MONOTONIC, &start); //start clock
#if WAIT_WAY == 0
    sem_wait(n);
    sem_wait(s);
#else
    while(sem_trywait(n) < 0);
    while(sem_trywait(s) < 0);
#endif
    sem_getvalue(n,&(number_storage[count]));
    if(DELAY_OF_OPERATIONS) sleep(DELAY_OF_OPERATIONS);
#if DEBUG
    printf("\t\tConsumer: take(); n = %d\n",number_storage[count]);
#endif
    //Take
    taken_value = ptr[number_storage[count]+1];
    //End Take
    sem_post(s);
    //Consume
#if DEBUG
    printf("\t\tConsumer: consume() => %d\n", taken_value);
#endif
      //Doing a NOP here just so the time isn't zero
      taken_value = taken_value / 5;
    //End Consume
    clock_gettime(CLOCK_MONOTONIC, &end); //end clock
    time_storage[MAX_ITEMS - ptr[0]] = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/10e9);
    if(!(number_storage[count] || ptr[0])) break;
  }
  //Output results
  printf("Consumer: Outputting Time\n");
  print_double_array(time_storage, MAX_ITEMS, file_time);
  printf("Consumer: Outputting Items in Array\n");
  print_int_array(number_storage, MAX_ITEMS, file_n);
  //Done Outputting
  //Clean Allocation
  free(time_storage);
  free(number_storage);
  //Clean Done
  printf("Consumer Exitting\n");
}

int main(int argc, char **argv)
{
  if(argc < 5){
    printf("Usage: ./pgm1 file1 file2 file3 file4\n");
    printf("file1: Producer Time Output\n");
    printf("file2: Producer Items in Array Output\n");
    printf("file3: Consumer Time Output\n");
    printf("file4: Consumer Items in Array Output\n");
    return -2;
  }
  /*Clean Start*/
  sem_unlink(SEM_S_NAME);
  sem_unlink(SEM_N_NAME);
  shm_unlink(SHM_NAME);
  /*Clean end*/

  //Initialize Semaphores and output starting values
  int temp = 0;
  s = sem_open(SEM_S_NAME, O_CREAT | O_EXCL, 0644, 1);
  sem_getvalue(s,&temp);
  printf("Main: value of s = %d\n",temp);
  if(s == SEM_FAILED){
    printf("Can't create sem s\n");
    return -1;
  }
  n = sem_open(SEM_N_NAME, O_CREAT | O_EXCL, 0644, 0);
  sem_getvalue(n,&temp);
  printf("Main: value of n = %d\n",temp);

  if(n == SEM_FAILED){
    printf("Can't create sem n\n");
    return -1;
  }
  //Done Initializing Semaphores

  //Creating a new shared memory segment.
	int shm_fd;
	shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
	/* configure the size of the shared memory segment */
	ftruncate(shm_fd,SHM_SIZE*sizeof(int32_t));
	/* now map the shared memory segment in the address space of the process */
	ptr = mmap(0,SHM_SIZE*sizeof(int32_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Shared Memory Map failed\n");
		return -1;
	}
  // End Creating a new shared memory segment.

  //Fork Process and start consumer/producer
  int pid = fork();
  if(pid < 0) {
    printf("Can't fork\n");
    return -1;
  }
  else if(pid > 0) {
    consumer(argv[3],argv[4]);
    //Cleanup
    printf("Now Cleaning\n");
    sem_unlink(SEM_S_NAME);
    sem_unlink(SEM_N_NAME);
    shm_unlink(SHM_NAME);
    //End Cleanup
  }
  else {
    producer(argv[1],argv[2]);
  }
  //Done forking

  return 0;
}
