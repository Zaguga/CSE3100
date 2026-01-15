#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

// Search TODO to find the functions you need to work on

#define MAX_NUM_PROGRAMS 10
#define FD_STDIN    0
#define FD_STDOUT   1
#define PIPEFD_READ     0
#define PIPEFD_WRITE    1

void die(char *s)
{
    fprintf(stderr, "Error: %s\n", s);
    if (errno)
        perror("errno");
    exit(EXIT_FAILURE);
} 

void check_pointer(void *p)
{
    if (p == NULL)
       die("Pointer is NULL.");
}

// structure for storing program arguments
typedef struct program_tag {
    char**   argv;      // array of pointers to arguments
    int      argc;      // number of arguments
    int	     pid;	    // process ID of the program
    int      fd_in;     // FD for stdin
    int      fd_out;    // FD for stdout
} Program;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* start a program. This function does not wait for the child process to 
 * finish.
 * This function needs to fork, 


 * 2. redirect stdin/stdout, 
 * 3. and then upgrade the child process into the program specified.
 *
 * If pipes are not created earlier, create pipes.
 * Close file descriptors that are not needed, in both parent and child processes.
 * If a child process is created successfully, save its pid in the
 * corresponding Program structure. 
 *
 * If any system call fails, call die() to exit.  
 * For example, die("fork() failed."); 
 * It will be messy if some programs are already started.
 *
 * Parameters:
 *      programs: an array of Program structures.
 *      num_programs: number of programs in the array.
 *      cur: the index of the program that needs to be started in this call.
 *
 * Return value: 
 *      This function does not return a value.  
 *
 * The child process never returns from this function.
 */
void start_program(Program *programs, int num_programs, int cur) 
{
    // TODO
    pid_t pid = fork();
    if (pid < 0){
        die("fork() failed.");
    }
    else if (pid == 0){
        //child -- check if overcomplication works without error bc idk if results value will carry and mess up the program
        int result;
        //redirect stdin
        if (programs[cur].fd_in != STDIN_FILENO){
            result = dup2(programs[cur].fd_in, STDIN_FILENO); //“Hey, standard input (fd 0) should now come from whatever programs[cur].fd_in is.”
            if (result == -1){
                die("dup2() failed for stdout.");
            }
        }
        //redirect stdout
        if (programs[cur].fd_out != STDOUT_FILENO){
            result = dup2(programs[cur].fd_out, STDOUT_FILENO); //“Hey, standard input (fd 0) should now come from whatever programs[cur].fd_in is.”
            if (result == -1){
                die("dup2() failed for stdout.");
            }
        }
        //closes the correct end to close unused ends (i+1 only reads from i so no need to have i+1's write to i open)
        for (int i = 0; i < num_programs; i++) {
            if (programs[i].fd_in != STDIN_FILENO)
                close(programs[i].fd_in);
            if (programs[i].fd_out != STDOUT_FILENO)
                close(programs[i].fd_out);
        }


        execvp(programs[cur].argv[0], programs[cur].argv); //diverts to run the actual program, but the rewiring stays
        //will only read this next code if it failed, so make it a kill statement
        die("execvp() failed.");
    }
    else{
        //parent
        programs[cur].pid = pid;
        //dont recieve or send from the pipes that arent the first and last that read from the actual std in and std out (i thinks)
        if (programs[cur].fd_in != STDIN_FILENO){
            close(programs[cur].fd_in);
        }
        if (programs[cur].fd_out != STDOUT_FILENO){
            close(programs[cur].fd_out); //its open to the parent and we want to close it?
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Wait on a program. 
 *
 * Parameter:
 *  prog:   A pointer to a Program, which has the pid of the program,
 *          which is set when the child process is created.
 *
 * Return value:
 *  -1:     prog->pid is less than 0, or waitpid() returns error.
 *  0-255:  exit value of the program (after WEXITSTATUS()).
 *
 * */
int wait_on_program(Program *prog)
{
    int exitStatus;

    if (prog->pid < 0)
        return -1;

    // TODO
    if (waitpid(prog->pid, &exitStatus, 0) == -1){ //standard conventino of how the waitpid() function works, the 0 is where you can put ur super secret settings (minecraft reference hehe)
        return -1;
    }
    return WEXITSTATUS(exitStatus); //takes the data the waitpid() dumps ints error into, kinda confusing but just accept this as standard and move on with ur life (idk girl im tired)
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* This function creates pipes to be used for connecting two pipeline stages.
 * You can create all pipes here,
 * or you can create pipes when they are needed in start_program().
 * If you decide to create pipes in start_program(),
 * leave this function empty.
 *
 * Paramters:
 *  programs: an array of Program structures.
 *  num_programs: number of programs in the array.
 *
 * Return value:
 *  This function does not return a value. If pipe() fails, 
 *  call the following function to exit from the program.
 *       die("pipe() failed.");
 */
void prepare_pipes(Program *programs, int num_programs)
{
    // TODO
    for (int i = 0; i < num_programs - 1; i++){
        //standard stuff
        int fds[2];
        if (pipe(fds) == -1){
            die("pipe() failed.");
        }
        
        programs[i].fd_out = fds[1]; //this bitch goes into the write end -- the writer, the output
        programs[i+1].fd_in = fds[0]; //this bitch goes into the read end -- the reader, the input
    }
    programs[0].fd_in = STDIN_FILENO; //the first one is the standard input (user) -- start of pipe sequence
    programs[num_programs -1].fd_out = STDOUT_FILENO; // the last one goes out to the stdout (terminal) -- the end of the pipe sequence
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*********************************************************/
/* Do not change code below                              */
/*********************************************************/
/* initialize a Program structure that allows argc arguments */
void init_program(Program *prog, int argc)
{
    // allocate memory for array of arguments
    prog->argv = malloc((argc + 1) * sizeof(char *));
    check_pointer(prog->argv);
    prog->argc= 0;
    prog->argv[0] = NULL;
    prog->pid = prog->fd_in = prog->fd_out = -1;
}

/* free the memory used by prog */
void free_programs(Program *programs, int num_programs)
{
    for (int i = 0; i <  num_programs; i ++) {
        free(programs[i].argv);
        programs[i].argv = NULL;
    }
}

// parse the command line arguments
// split them into programs
// return the number of programs found
int parse_command_line(Program *progs, int max_num_progs, int argc, char **argv)
{
    int     cur_arg = 1;
    int     prog_count = 0;

    while (cur_arg < argc) {
        // get arguments for a new program

        if (prog_count == max_num_progs) 
            die("Too many programs.");

        // allocate space for argv for progs[prog_count]
        // Could have scanned the arguments and find out how many arguments
        // the current program has. 
        init_program(&progs[prog_count], argc);

        int     ia = 0;  // index of arguments for progs[prog_count]
        while (cur_arg < argc && strcmp(argv[cur_arg], "--")) {
            progs[prog_count].argv[ia++] = argv[cur_arg++];
        }
        progs[prog_count].argv[ia] = NULL;
        progs[prog_count].argc = ia;
        prog_count ++;
        if (ia == 0) 
            die("Empty program.");

        cur_arg ++; // skip "--"
        if (cur_arg == argc) 
            die("Last program is empty."); 
    }

    return prog_count;
}

int main(int argc, char **argv)
{
    Program programs[MAX_NUM_PROGRAMS];
    int     num_programs;

    if (argc <= 1) 
        die ("Specify at least one program to run. Multiple programs are separated by --");

    // Prepare programs and their arguments
    num_programs = parse_command_line(programs, MAX_NUM_PROGRAMS, argc, argv); 

    // Prepare pipes
    prepare_pipes(programs, num_programs);

    // spawn children
    for (int i = 0; i < num_programs; i ++) {
        fprintf(stderr, "Starting program %d:%s\n", i, programs[i].argv[0]);
        start_program(programs, num_programs, i);
    }

    // wait for children
    for (int i = 0; i < num_programs; i ++) {
        fprintf(stderr, "Waiting for program %d:%s\n", i, programs[i].argv[0]);
        int status = wait_on_program(&programs[i]);
        fprintf(stderr, "Program %d:%s exited with %d\n", i, programs[i].argv[0], status);
    }

    // free memory
    free_programs(programs, num_programs);

    // You should see the message only once
    fprintf(stderr, "Parent: Everything is good.\n");
    return 0;
}
