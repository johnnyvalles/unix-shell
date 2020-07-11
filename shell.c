/* shell.c */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXARGS 128
#define MAXLINE 8192

int
main() {
    char buf[MAXLINE];                                      /* buffer holds user's input */
    while(1) {
        printf(">>> ");                                     /* print the shell prompt */
        fgets(buf, MAXLINE, stdin);                         /* read a line and store in buffer */
        if (fgets(buf, MAXLINE, stdin) == NULL)             /* check for end-of-file or error */
            exit(0);
        printf("%s\n", buf);
    }
    return EXIT_SUCCESS;
}