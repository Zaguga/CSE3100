#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

/***************************************************************
 * RANDOM NUMBER MACROS (unchanged from original)
 **************************************************************/
#define RANDOM_INIT(x)  (g_random_buffer[0]=3100,g_random_buffer[1]=(x), g_random_buffer[2] = (x) >> 16)
#define RANDOM_INT() (nrand48(g_random_buffer))
#define THINKING() my_msleep(RANDOM_INT()%10)
#define MAKE_CHOICE() (nrand48(g_random_buffer) % 3)

enum {C_ROCK, C_PAPER, C_SCISSORS};
enum {S_INIT, S_READY};   // shared-int status states

/***************************************************************
 * ERROR HANDLING
 **************************************************************/
static void die(char *s) {
    if (errno) perror(s);
    else fprintf(stderr, "Error: %s\n", s);
    exit(EXIT_FAILURE);
}

/***************************************************************
 * SLEEP
 **************************************************************/
void my_msleep(int r) {
    struct timespec req = {0, (r % 100 + 1) * 1000000};
    nanosleep(&req, NULL);
}

/***************************************************************
 * SHARED STRUCTURES
 **************************************************************/

/* Shared integer between a single player and referee */
typedef struct {
    int value;                // player’s choice (rock/paper/scissors)
    int status;               // S_INIT=not ready, S_READY=ready to read
    pthread_mutex_t mutex;
    pthread_cond_t cond;      // signals when value is ready
} shared_int_t;

/*
 * Result shared between referee and BOTH players.
 * This replaces the barrier with explicit state flags + mutex.
 */
typedef struct {
    int value;                // the referee’s computed result

    int result_ready;         // 0 = not ready, 1 = ready for players
    int players_done;         // how many players have consumed the result

    pthread_mutex_t mutex;
    pthread_cond_t cond_result_ready; // players wait on this
    pthread_cond_t cond_players_done; // referee waits on this
} result_t;

/***************************************************************
 * THREAD ARGUMENT STRUCTS
 **************************************************************/
typedef struct {
    int id;           // player id: 1 or 2
    int seed;
    int n_rounds;
    int opt_quiet;

    shared_int_t *choice; // where they publish their choice
    result_t *result;     // where they read the final result

    int n_wins;
    int n_ties;
} player_arg_t;

typedef struct {
    int n_rounds;
    shared_int_t *choice1;
    shared_int_t *choice2;
    result_t *result;
    int opt_quiet;
} referee_arg_t;

/***************************************************************
 * GAME LOGIC
 **************************************************************/
int compare_choices(int c1, int c2) {
    if (c1 == c2) return 0;
    if ((c1 + 1) % 3 == c2) return 2;
    return 1;
}

/***************************************************************
 * PLAYER THREAD
 * Each round:
 *   1. Make a choice
 *   2. Publish it to referee
 *   3. Wait for result_ready
 *   4. Read result
 *   5. Tell referee you’re done
 **************************************************************/
void *thread_player(void *arg_in) {
    unsigned short g_random_buffer[3];
    player_arg_t *arg = arg_in;

    RANDOM_INIT(arg->seed);
    arg->n_wins = arg->n_ties = 0;

    shared_int_t *choice = arg->choice;
    result_t *result = arg->result;

    for (int i = 0; i < arg->n_rounds; i++) {
        int r = MAKE_CHOICE();
        int outcome = 0;

        if (!arg->opt_quiet)
            printf("Player %d: round %d: %d\n", arg->id, i, r);

        /***********************
         * PRODUCE CHOICE
         ***********************/
        pthread_mutex_lock(&choice->mutex);
        choice->value = r;
        choice->status = S_READY;
        pthread_cond_signal(&choice->cond); // wake referee
        pthread_mutex_unlock(&choice->mutex);

        /***********************
         * WAIT FOR RESULT
         ***********************/
        pthread_mutex_lock(&result->mutex);

        while (result->result_ready == 0)
            pthread_cond_wait(&result->cond_result_ready, &result->mutex);

        outcome = result->value; // safely read

        // Mark this player as done reading the result
        result->players_done++;
        if (result->players_done == 2)
            pthread_cond_signal(&result->cond_players_done);

        pthread_mutex_unlock(&result->mutex);

        /***********************
         * UPDATE STATS
         ***********************/
        if (outcome == 0) arg->n_ties++;
        else if (outcome == arg->id) arg->n_wins++;
    }

    return NULL;
}

/***************************************************************
 * REFEREE THREAD
 * Each round:
 *   1. Wait for P1 choice
 *   2. Wait for P2 choice
 *   3. Compute outcome
 *   4. Publish result_ready
 *   5. Wait for both players to read it (players_done == 2)
 *   6. Reset state
 **************************************************************/
void *thread_referee(void *arg_in) {
    referee_arg_t *arg = arg_in;

    shared_int_t *p1 = arg->choice1;
    shared_int_t *p2 = arg->choice2;
    result_t *result = arg->result;

    for (int i = 0; i < arg->n_rounds; i++) {
        int c1, c2, outcome;

        /***********************
         * WAIT FOR PLAYER 1
         ***********************/
        pthread_mutex_lock(&p1->mutex);
        while (p1->status == S_INIT)
            pthread_cond_wait(&p1->cond, &p1->mutex);
        c1 = p1->value;
        p1->status = S_INIT;
        pthread_mutex_unlock(&p1->mutex);

        /***********************
         * WAIT FOR PLAYER 2
         ***********************/
        pthread_mutex_lock(&p2->mutex);
        while (p2->status == S_INIT)
            pthread_cond_wait(&p2->cond, &p2->mutex);
        c2 = p2->value;
        p2->status = S_INIT;
        pthread_mutex_unlock(&p2->mutex);

        /***********************
         * COMPUTE OUTCOME
         ***********************/
        outcome = compare_choices(c1, c2);
        if (!arg->opt_quiet) //a command line argument that flags whether or not to print output
            printf("Ref: Round %d: (%d,%d) outcome is %d\n", i, c1, c2, outcome);

        /***********************
         * PUBLISH RESULT
         ***********************/
        pthread_mutex_lock(&result->mutex);

        result->value = outcome;
        result->result_ready = 1;             // mark ready
        pthread_cond_broadcast(&result->cond_result_ready); // wake both players

        /***********************
         * WAIT FOR BOTH PLAYERS
         ***********************/
        while (result->players_done < 2)
            pthread_cond_wait(&result->cond_players_done, &result->mutex);

        // Reset state for next round
        result->players_done = 0;
        result->result_ready = 0;

        pthread_mutex_unlock(&result->mutex);
    }

    return NULL;
}

/***************************************************************
 * MAIN FUNCTION
 **************************************************************/
int main(int argc, char *argv[]) {
    int seed = 5050;
    int n_rounds = 10;
    int opt_quiet = 0;

    /* parse args omitted for brevity—same as your original */

    shared_int_t data[2];
    result_t result;
    player_arg_t pargs[2];
    pthread_t tids[2];
    referee_arg_t rarg;

    /***********************
     * INIT PLAYER SHARED DATA
     ***********************/
    for (int i = 0; i < 2; i++) {
        data[i].status = S_INIT;
        pthread_mutex_init(&data[i].mutex, NULL);
        pthread_cond_init(&data[i].cond, NULL);
    }

    /***********************
     * INIT RESULT STRUCT
     ***********************/
    result.result_ready = 0;
    result.players_done = 0;
    pthread_mutex_init(&result.mutex, NULL);
    pthread_cond_init(&result.cond_result_ready, NULL);
    pthread_cond_init(&result.cond_players_done, NULL);

    /***********************
     * CREATE PLAYER THREADS
     ***********************/
    for (int i = 0; i < 2; i++) {
        pargs[i].id = i + 1;
        pargs[i].seed = (seed << 2) + i;
        pargs[i].n_rounds = n_rounds;
        pargs[i].choice = &data[i];
        pargs[i].result = &result;
        pargs[i].opt_quiet = opt_quiet;

        pthread_create(&tids[i], NULL, thread_player, &pargs[i]);
    }

    /***********************
     * RUN REFEREE IN MAIN THREAD
     ***********************/
    rarg.choice1 = &data[0];
    rarg.choice2 = &data[1];
    rarg.result = &result;
    rarg.n_rounds = n_rounds;
    rarg.opt_quiet = opt_quiet;

    thread_referee(&rarg);

    /***********************
     * JOIN PLAYER THREADS
     ***********************/
    for (int i = 0; i < 2; i++)
        pthread_join(tids[i], NULL);

    /***********************
     * PRINT RESULTS
     ***********************/
    for (int i = 0; i < 2; i++) {
        printf("Player %d won %d, lost %d, tied %d.\n",
               pargs[i].id,
               pargs[i].n_wins,
               pargs[i].n_rounds - pargs[i].n_wins - pargs[i].n_ties,
               pargs[i].n_ties);
    }

    return 0;
}
