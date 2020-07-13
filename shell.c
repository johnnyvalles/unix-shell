/* shell.c */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXARGS 128
#define MAXLINE 8192

void
parse_cmd(char* buf);

int
main() {
    char buf[MAXLINE];                                      /* buffer holds user's input */
    while(1) {
        printf(">>> ");                                     /* print the shell prompt */
        fgets(buf, MAXLINE, stdin);                         /* read a line and store in buffer */
        if (fgets(buf, MAXLINE, stdin) == NULL)             /* check for end-of-file or error */
            exit(0);
        parse_cmd(buf);                                     /* parse the command */
    }
    return EXIT_SUCCESS;
}

void
parse_cmd(char* buf) {
    char* argv[MAXARGS];                                    /* argument vector */
    char* del;                                              /* delimter pointer */
    int i;                                                  /* holds number of arguments parsed */
    
    buf[strlen(buf) - 1] = ' ';                             /* Replace newline with a space */
    
    while (buf && (buf[i] == ' '))                          /* ignore leading spaces */
        ++buf;
    
    i = 0;
    while ((del = strchr(buf, ' '))) {                      /* begin constructing argv */
        *del = '\0';
        argv[i++] = buf; 
        buf = del + 1;
        
        while (buf && (buf[i] == ' '))                      /* ignore leading spaces */
            ++buf;
    }
    
    if (i == 0)
        return;
}