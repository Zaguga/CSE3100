#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N_THREADS 2
#define SAMPLE_SIZE 10000000

typedef struct thread_arg_tag {
  int id;
  int count;
  int total;
} thread_arg_t;

double rand_double(unsigned int *s) { return (rand_r(s) / (double)RAND_MAX); }

void *thread_main(void *thread_arg) {
  thread_arg_t *arg = (thread_arg_t *)thread_arg;

  int id = arg->id;
  int total = arg->total;
  int counter = arg->count;

  double x, y;

  for (int i = 0; i<total; i++){
    x = rand_double(&id);
    y = rand_double(&id);

    if (x * x + y * y <= 1){
      counter++; //if there is an error its here prob
    }
  }
  arg->count = counter;
  return NULL;

}

int main(int argc, char *argv[]) {

  int n = SAMPLE_SIZE;
  int n_threads = N_THREADS;

  int total_count = 0;

  if (argc > 1) {
    n = atoi(argv[1]);
  }
  if (argc > 2) {
    n_threads = atoi(argv[2]);
  }

  pthread_t thread_arr[n_threads];
  thread_arg_t thread_args[n_threads];

  for (unsigned int i = 0; i < n_threads; i++){
    thread_args[i].id = i;
    thread_args[i].count = 0;
    thread_args[i].total = n;

    if(pthread_create(&thread_arr[i], NULL, thread_main, &thread_args[i]) != 0){
      perror("pthread_create");
      exit(EXIT_FAILURE);
    }
  }

  for(unsigned int i = 0; i <n_threads; i++){
    if(pthread_join(thread_arr[i], NULL) != 0){
      perror("pthread_join");
      exit(EXIT_FAILURE);
    }
  }

  for(unsigned int i = 0; i <n_threads; i++){
    total_count += thread_args[i].count;
  }

  double pi = 4.0 * total_count / (double)n;

  printf("pi = %.2f\n", pi);

  return 0;
}
