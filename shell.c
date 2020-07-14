/* shell.c */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXARGS 128
#define MAXLINE 8192

void parse_cmd(char* buf);
int builtin_cmd(char** buf);
void exec_cmd(char** argv);

int main() {
    char buf[MAXLINE];                                      /* buffer holds user's input */
    while(1) {
        printf(">>> ");                                     /* print shell prompt */
        if (fgets(buf, MAXLINE, stdin) == NULL)             /* read and store line, check for end-of-file or error */
            exit(0);
        parse_cmd(buf);                                     /* parse the command */
    }
    return EXIT_SUCCESS;
}

void parse_cmd(char* buf) {
    char* argv[MAXARGS];                                    /* argument vector */
    char* del;                                              /* delimiter pointer */
    int argc;                                               /* number of arguments parsed */
    
    buf[strlen(buf) - 1] = ' ';                             /* replace '\n' with a space */
    
    while (buf && (*buf == ' '))                            /* ignore leading spaces */
        ++buf;
    
    argc = 0;
    while ((del = strchr(buf, ' '))) {                      /* construct argv */
        *del = '\0';
        argv[argc++] = buf; 
        buf = del + 1;
        
        while (buf && (*buf == ' '))                        /* ignore leading spaces */
            ++buf;
    }

    argv[argc] = NULL;                                      /* NULL-terminate argv */
    
    if (argc == 0)                                          /* empty line */
        return;

    if (builtin_cmd(argv))                                  /* check if built-in command */
        return;

    exec_cmd(argv);
}

int builtin_cmd(char** argv) {
    if (!strcmp(*argv, "exit"))
        exit(0);
    
    if (!strcmp(*argv, "help")) {
        printf("Help command.\n");
        return 1;
    }

    return 0;
}

void exec_cmd(char** argv) {
    if (fork() == 0) {
        if (execv(*argv, argv) < 0) {
            printf("%s: unknown command.\n", *argv);
            exit(0);
        }
    } else {
        wait(NULL);
    }
}