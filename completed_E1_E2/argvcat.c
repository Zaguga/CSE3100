#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* print out an error message and exit */
void my_error(char *s)
{
    perror(s);
    exit(1);
}

/* Concatnate two strings.
 * Dynamically allocate space for the result.
 * Return the address of the result.
 */
char *my_strcat(char *s1, char *s2)
{
    // TODO
    //get size of the 2 strings +1 for null character
    size_t size = strlen(s1) + strlen(s2) + 1;

    char *q = malloc(size);
    if (q == NULL){
        my_error("malloc");
    }

    //copy the strings over
    memcpy(q, s1, strlen(s1));
    memcpy(q + strlen(s1), s2, strlen(s2));
    q[strlen(s1) + strlen(s2)] = '\0';

    return q;

}

int main(int argc, char *argv[])
{
    char *s, *temp;

    s = my_strcat("", argv[0]);

    for (int i = 1; i < argc; i++) {
        temp = s;
        s = my_strcat(s, argv[i]);
        free(temp);
    }

    printf("%s\n", s);

    free(s);
    return 0;
}
