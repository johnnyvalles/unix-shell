/* shell.c */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXARGS 16
#define MAXLINE 256

void parse_cmd(char* buf, char** argv, unsigned int* argc);
int builtin_cmd(char** buf);
void exec_cmd(char** argv, int argc);
void read_cmd(char* buf);

int main() {
    char buf[MAXLINE];                                /* input buffer */       
    char* argv[MAXARGS];                              /* argument vector */
    unsigned int argc = 0;                            /* argument count */
                            
    while(1) {
        read_cmd(buf);
        parse_cmd(buf, argv, &argc);
        exec_cmd(argv, argc);
    }

    return EXIT_SUCCESS;
}

void read_cmd(char* buf) {
    printf(">>> ");                                         /* print shell prompt */
    if (fgets(buf, MAXLINE, stdin) == NULL)                 /* read & store line, check for EOF or error */
        exit(0);                                            /* shell exits on EOF or error */
}

void parse_cmd(char* buf, char** argv, unsigned int* argc) {
    char* del;                                              /* points to delimiter */
    *argc = 0;                                              /* set argument count to 0 */
    buf[strlen(buf) - 1] = ' ';                             /* replace '\n' with a space */
    while (buf && (*buf == ' '))                            /* ignore leading spaces */
        ++buf;
    while ((del = strchr(buf, ' '))) {                      /* construct argv */
        *del = '\0';                                        /* replace space with null character */
        argv[(*argc)++] = buf; 
        buf = del + 1;
        
        while (buf && (*buf == ' '))                        /* ignore leading spaces */
            ++buf;
    }

    argv[*argc] = NULL;                                      /* NULL-terminate argv */
}

int builtin_cmd(char** argv) {
    if (!strcmp(*argv, "exit"))                             /* check if typed exit */
        exit(0);
    
    if (!strcmp(*argv, "help")) {                           /* check if typed help */
        char *help = 
        "************************** Shell "
        "**************************\n"
        "Built-in commands:\n"
        "\thelp\n"
        "\texit\n"
        "\nExternal programs:\n"
        "\tTyping in the name of a program (e.g."
        " prog1 \n\tor ./prog1) assumes the working directory\n"
        "\tto be where the shell executable is located.\n"
        "\tOtherwise an absolute path must be provided.\n"
        "***************************"
        "********************************\n";
        printf("%s", help);
        return 1;
    }

    return 0;
}

void exec_cmd(char** argv, int argc) {
    if (argc == 0 || builtin_cmd(argv))
        return;

    pid_t pid = fork();

    if (pid < 0) {                                          /* check if fork failed */
        fprintf(stderr, "fork error.\n");                   /* terminate shell if fork failed */ 
        exit(0);
    } else if (pid == 0) {                                  /* create a new process, check if child process */
        if (execv(*argv, argv) < 0) {                       /* only runs in child process attempts to run program */
            fprintf(stderr,                                 /* terminate child process if exec fails */
                    "%s: unknown command.\n", 
                    *argv);
            exit(0);
        }
    } else {
        wait(NULL);                                         /* otherwise parent waits for child process to finish */
    }
}