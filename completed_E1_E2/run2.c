#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char ** argv)
{
    pid_t child_p1;
    pid_t child_p2;
    int exitStatus;

    // at least, there should be 3 arguments
    // 2 for the first command, and the rest for the second command
    if (argc < 4) {
        fprintf(stderr, "Usage: %s cmd1 cmd1_arg cmd2 [cmd2_args ..]\n", argv[0]);
        return 1;
    }

    // TODO
    child_p1 = fork();
    if (child_p1 < 0){
        //fork fails
        perror("fork()");
        exit(1);
    }
    else if (child_p1 == 0){
        //child 1
        execlp(argv[1], argv[1], argv[2], NULL);
        perror("execlp()");
        exit(1);
    }
//parent wait for c1
    waitpid(child_p1, &exitStatus, 0);
    printf("exited=%d exitstatus=%d\n", WIFEXITED(exitStatus), WEXITSTATUS(exitStatus));

    child_p2 = fork();
    if (child_p2 < 0){
        //fork fails
        perror("fork()");
        exit(1);
    }
    else if(child_p2 == 0){
        //child 2
        char **argv2 = &argv[3]; //takes the elements from argv[3] onwards by setting a new pointer to it just like we did for the original argv
        execvp(argv2[0], argv2);
        perror("execvp()");
        exit(1);
    }
//parent wait for c2
    //outside child blocks so it only executes in the parent
    waitpid(child_p2, &exitStatus, 0);
    printf("exited=%d exitstatus=%d\n", WIFEXITED(exitStatus), WEXITSTATUS(exitStatus));

    return 0;
}
