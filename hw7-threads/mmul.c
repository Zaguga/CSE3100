#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "matrix.h"

// Search TODO to find the locations where code needs to be completed

#define     NUM_THREADS     2

typedef struct {
    unsigned int id;
    TMatrix *m, *n, *t;
} thread_arg_t;

static void * thread_main(void * p_arg)
{
    // TODO
    thread_arg_t *arg = (thread_arg_t *)p_arg; //tell the void * pointer to act as a thread_arg_t (struct) pointer so we can access its info

    //1st unpack the info for local use
    unsigned int id = arg->id;//use to split the work
    TMatrix *m = arg->m;
    TMatrix *n = arg->n;
    TMatrix *t = arg->t;
    //split up the work
    unsigned rows_per_thread = m->nrows/NUM_THREADS;
    unsigned start_row = id * rows_per_thread;
    unsigned end_row = (id == NUM_THREADS - 1)
                        ? m->nrows
                        : start_row + rows_per_thread;


    //the actual multiplication part

    for (unsigned int i = 0; i < end_row; i++){
        for (unsigned int j = 0; j < n->ncols; i ++){
            t->data[i][j] = 0;
            for (unsigned int k = 0; k < m->ncols; k++){
                t->data[i][j] += m->data[i][k] * n->data[k][j];
            }
        }
    }


    return NULL;
}

/* Return the sum of two matrices.
 *
 * If any pthread function fails, report error and exit. 
 * Return NULL if anything else is wrong.
 *
 * Similar to mulMatrix, but with multi-threading.
 */
TMatrix * mulMatrix_thread(TMatrix *m, TMatrix *n)
{
    if (    m == NULL || n == NULL
         || m->ncols != n->nrows )
        return NULL;

    TMatrix * t = newMatrix(m->nrows, n->ncols);
    if (t == NULL)
        return t;

    // TODO
    pthread_t threads[NUM_THREADS]; //create our threads 0,1
    thread_arg_t args[NUM_THREADS]; //create our array of structs to pass to each thread 0,1; rn is 2 empty structs, so we need to fill the attributes with args[i].attribute = x;

    //create the structs for each thread (for 0, 1)
    for(unsigned int i = 0; i < NUM_THREADS; i++){
        args[i].id = i; //0 or 1
        args[i].m = m; //from func args
        args[i].n = n; //from func args
        args[i].t = t; //created earlier

        //create the actual threads for each
        if (pthread_create(&threads[i], NULL, thread_main, &args[i]) != 0){
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        //done with setting up threads, loop is done
    }
    // "parent" stuff (joining the threads/ the waiting for them to be done, the pthread create sends them on their way, this is when they get back/if)
    
    for(unsigned int i = 0; i < NUM_THREADS; i++){ //for each thread....
        if (pthread_join(threads[i], NULL) != 0){ //wait for them to join, if one returns something other than 0 meaning it failed quit the program
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }


    return t;
}
