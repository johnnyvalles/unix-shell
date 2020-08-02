/* shell.c */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXARGS 16
#define MAXLINE 256

/* error-checking system call wrappers */
pid_t Fork(void);
void Execv(const char* path, char *const argv[]);

/* shell utilities */
void parse_cmd(char* buf, char** argv, int* argc);
int builtin_cmd(char** buf);
void exec_cmd(char** argv, int argc);
void read_cmd(char* buf);

int main(void) {
    char buf[MAXLINE];                                /* buffer for stdin line */       
    char* argv[MAXARGS];                              /* argument vector */
    int argc = 0;                                     /* argument count */
                            
    for (;;) {
        read_cmd(buf);                                /* read line from stdin */
        parse_cmd(buf, argv, &argc);                  /* parse, tokenize, build argv & argc */
        exec_cmd(argv, argc);                         /* execute parsed line */
    }

    return EXIT_SUCCESS;
}

pid_t Fork(void){
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "fork error... terminating shell.\n");
        exit(0);
    }
    
    return pid;
}

void Execv(const char* path, char *const argv[]) {
    if (execv(path, argv) < 0) {
        fprintf(stderr, "%s: unknown command.\n", *argv);
        exit(0);
    }
}

void read_cmd(char* buf) {
    printf(">>> ");                                   /* print shell prompt */
    if (fgets(buf, MAXLINE, stdin) == NULL)           /* read & store line, check for EOF or error */
        exit(0);                                      /* exit on EOF or error */
}

void parse_cmd(char* buf, char** argv, int* argc) {
    char* del;                                        /* points to space delimiter */
    *argc = 0;                                        /* set argument count to 0 */
    buf[strlen(buf) - 1] = ' ';                       /* replace trailing '\n' with space */
    while (buf && (*buf == ' '))                      /* ignore leading spaces */
        ++buf;
    while ((del = strchr(buf, ' '))) {                /* build argv */
        *del = '\0';                                  /* replace space delimiter with NULL character */
        argv[(*argc)++] = buf;                        /* set argv[argc] to string before NULL character */
        buf = del + 1;                                /* set buf after NULL character of last string */
        while (buf && (*buf == ' '))                  /* ignore leading spaces */
            ++buf;
    }
    argv[*argc] = NULL;                               /* NULL-terminate argv */
}

int builtin_cmd(char** argv) {
    if (!strcmp(*argv, "exit"))                       /* check if user typed exit */
        exit(0);

    if (!strcmp(*argv, "whoami")) {                   /* check if user typed whoami for USER env var*/
        printf("%s\n", getenv("USER"));
        return 1;
    }
    
    if (!strcmp(*argv, "help")) {                     /* check if user typed help */
        char *help = 
        "************************** Shell "
        "**************************\n"
        "Built-in commands:\n"
        "\thelp\n"
        "\texit\n"
        "\twhoami\n"
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
    if (argc == 0 || builtin_cmd(argv))               /* check if no command or built-in */
        return;

    pid_t pid = Fork();                               /* spawn child process using fork wrapper */

    if (pid == 0) {                                   /* check if child process */
        Execv(*argv, argv);                           /* replace child process address space using execv wrapper */
    } else {
        wait(NULL);                                   /* parent waits for child process to finish */
    }
}