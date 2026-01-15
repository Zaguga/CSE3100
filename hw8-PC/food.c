#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "linked-list.h"

#define MAX 10

typedef struct {
    node    *head, *tail;
    pthread_mutex_t mutex;
} list_t;

typedef struct {
	int size;
        int buf[MAX][3];
        int remain;
        int counts[3];            //current indexes
        pthread_mutex_t mutex;
        pthread_cond_t produce_cond;
        pthread_cond_t consume_cond;
}two_d_buffer;

void add_to_buffer(int item, int col, two_d_buffer *p)
{
	//TODO
	//fill in code below
	pthread_mutex_lock(&p->mutex);
	while (p->counts[col] >= p->size){ //producer waits if the specific column for this item is full
		pthread_cond_wait(&p->produce_cond, &p->mutex);
	}

	p->buf[p->counts[col]][col] = item;
	p->counts[col]++; //increment count (the new index after addition)
	p->remain = p->remain - 1;
	//need to signal consumer
	pthread_cond_signal(&p->consume_cond);

	pthread_mutex_unlock(&p->mutex);
}



void remove_from_buffer(int *a, int *b, int *c, two_d_buffer *p)
{
	//TODO
	//fill in code below
	pthread_mutex_lock(&p->mutex);
	while ((p->counts[0] == 0 || p-> counts[1] == 0 || p->counts[2] == 0)){
		pthread_cond_wait(&p->consume_cond, &p->mutex);
	}
	//not removing abc but removing the first of each colomn and PUTTING IT IN ABC
	*a = p->buf[0][0]; //bottom of 1
	*b = p->buf[0][1]; //bottom of 2
	*c = p->buf[0][2]; //bottom of 3

	//slide ll changes??? -- a la 2d array just hardcode shift up with double for loop
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < p->counts[i] - 1; j++){
			p->buf[j][i] = p->buf[j+1][i]; //j is however big each queue is
		}
		p->counts[i]--; //i is 0, 1, 2
	}
	pthread_cond_broadcast(&p->produce_cond); //BORADCAST bc we are waking up all producers (its giving rd/wr lock logic here)
	pthread_mutex_unlock(&p->mutex); //SLAY
	
}

void prepare(int item)
{
	usleep((item + 1)*100);
}

struct thread_data
{
	int id;
    	list_t *p;                  
    	two_d_buffer *q;
	int total;			//total items produced by a producer
	pthread_barrier_t *p_barrier;
};

void* thread_consume(void* threadarg)
{
    	struct thread_data* my_data = (struct thread_data*) threadarg;
	int id = my_data->id;
	list_t *p = my_data->p;

	node *n1 = create_node(0);
	node *n2 = create_node(1);
	node *n3 = create_node(2);
	
	//TODO
	//fill in code below to add n1, n2, and n3 to the linked-list pointed by p
	pthread_mutex_lock(&p->mutex);
	//pass head and tail bc that the params for function (idrk where they are coming from but trust the process)
	add_last(&p->head, &p->tail, n1); 
	add_last(&p->head, &p->tail, n2);
	add_last(&p->head, &p->tail, n3);
	pthread_mutex_unlock(&p->mutex);

	pthread_barrier_t *p_barrier = my_data->p_barrier;
	pthread_barrier_wait(p_barrier);

	two_d_buffer *q = my_data->q;
	int a, b, c;
	remove_from_buffer(&a, &b, &c, q);
	printf("consumer %04d (%d %d %d)\n", id, a, b, c);		
	pthread_exit(NULL);
}

void* thread_produce(void* threadarg)
{   
	struct thread_data* my_data = (struct thread_data*) threadarg;
        list_t *p = my_data->p;
        pthread_barrier_t *p_barrier = my_data->p_barrier;
        pthread_barrier_wait(p_barrier);
	two_d_buffer *q = my_data->q;

	int done = 0;
	while(!done)
	{
		//TODO
		//fill in code below
		pthread_mutex_lock(&q->mutex);
		//if we made all the orders
		if(q->remain == 0 && (q->counts[0] == 0 && q->counts[1] == 0 && q->counts[2] == 0)){
			pthread_mutex_unlock(&q->mutex);
			done = 1;
			continue; //checks loop again and leaves since !done is now false
		}
		pthread_mutex_unlock(&q->mutex); //only have mutex locked when we are checking mutex things

		//
		//for (something){
		//	add_to_buffer(something, i?, two_d_buffer *q);
		//}

		//remove item from ll queue
		pthread_mutex_lock(&p->mutex);
        node *item_node = remove_first(&p->head, &p->tail);
        pthread_mutex_unlock(&p->mutex);

		if (item_node == NULL) {
            if (q->remain == 0) {
                 done = 1;
            } else {
                 continue;
            }
        }
        if (done) { 
            if(item_node != NULL) free(item_node);
            continue;
        }
        int item = item_node->v;
        int col = item;
        free(item_node);
        prepare(item);
		add_to_buffer(item, col, q);

        my_data->total++;
		
	}
	 
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
	if(argc < 4) {
		printf("Usage: %s n_consumer n_producer buffer_size\n", argv[0]);
		return -1;
	}
	int n_consumer = atoi(argv[1]);
	assert(n_consumer <= 3000);
	int n_producer = atoi(argv[2]);
	assert(n_producer <= 3000);
	int size = atoi(argv[3]);
	assert(size <= MAX);
	//initilize the list
	list_t *p = (list_t *)malloc(sizeof(list_t));
	if(p==NULL)
	{
		perror("Cannot allocate memeory.\n");
		return -1;
	}
	p->head = NULL;
	p->tail = NULL;
	pthread_mutex_init(&p->mutex, NULL);
  
	//initilize the 2d buffer
	two_d_buffer *q = malloc(sizeof(two_d_buffer));
        q->size = size;
        q->remain = 3*n_consumer;
        q->counts[0] = 0; q->counts[1] = 0; q->counts[2] = 0;
	pthread_mutex_init(&q->mutex, NULL);
    	pthread_cond_init (&q->produce_cond, NULL);
    	pthread_cond_init (&q->consume_cond, NULL);

	pthread_barrier_t barrier;
	pthread_barrier_init(&barrier, NULL, n_consumer + n_producer); 
    	pthread_t threads[n_consumer + n_producer];
    	struct thread_data thread_data_array[n_consumer + n_producer];
    	int rc, t;

	for(t=0; t<n_consumer; t++ ) {
        	thread_data_array[t].id = t;
		thread_data_array[t].p = p;
		thread_data_array[t].q = q;
		thread_data_array[t].total = 0;
		thread_data_array[t].p_barrier = &barrier;
		//TODO
		//complete the following line of code
		//just needed to add thread function


		rc = pthread_create(&threads[t], NULL, thread_consume , &thread_data_array[t]);
        	if (rc) {
            		printf("ERROR; return code from pthread_create() is %d\n", rc);
            		exit(-1);
        	}
    	}

        for(t=0; t<n_producer; t++ ) {
                thread_data_array[n_consumer + t].id = t;
                thread_data_array[n_consumer + t].p = p;
                thread_data_array[n_consumer + t].q = q;
		thread_data_array[n_consumer + t].total = 0;
		thread_data_array[n_consumer + t].p_barrier = &barrier;
		//TODO
		//complete the follow line of code
		//just needed to add thread function

                rc = pthread_create(&threads[n_consumer + t], NULL, thread_produce , &thread_data_array[n_consumer + t]);
                if (rc) {
                        printf("ERROR; return code from pthread_create() is %d\n", rc);
                        exit(-1);
                }
        }

    	for(t=0; t<n_consumer + n_producer; t++ ) 
    	{
        	rc = pthread_join( threads[t], NULL );
        	if( rc ){
            	printf("ERROR; return code from pthread_join() is %d\n", rc);
            	exit(-1);
        	}
    	}

	int total = 0;
	//TODO
	//fill in code below
	for (t = 0; t < n_producer; t++){
		total += thread_data_array[n_consumer + t].total;
	}
	

	printf("total = %d\n", total);
 
    	pthread_mutex_destroy(&p->mutex);
    	free(p);

	pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->consume_cond);
	pthread_cond_destroy(&q->produce_cond);
	free(q);

	pthread_barrier_destroy(&barrier);	
    	return 0;
}
