#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXARGS 128
#define MAXLINE 8192

extern char** environ;

void evalcmd(char* buf);
int parsecmd(char* buf, char** argv);
void builtincmd(char** argv) {
    if (!strcmp(argv[0], "quit"))
        exit(0);
}

int main(int argc, char** argv) {
    char buf[MAXLINE]; // Input buffer
    
    while(1) {
        // Print shell prompt
        printf(">> ");
        
        // Get line from stdin
        fgets(buf, MAXLINE, stdin);
        
        // Check for end-of-file
        if(feof(stdin))
            exit(0);

        // Evaluate command
        evalcmd(buf);
    }

    return EXIT_SUCCESS;
}

void evalcmd(char* buf) {
    char* argv[MAXARGS];
    char modbuf[MAXLINE];
    pid_t pid;

    strcpy(modbuf, buf);

    if (parsecmd(modbuf, argv) < 0)
        return;
    
    if ((pid = fork()) == 0) {
        if (execve(argv[0], argv, environ) < 0) {
            printf("%s: Command not found.\n", argv[0]);
            return;
        }
    }

    // Parent waits for foreground children
    if (waitpid(pid, NULL, 0) < 0)
        printf("Wait Error.\n");

}

int parsecmd(char* buf, char** argv) {
    char* del;
    int argc;

    // Remove trailing '\n'
    buf[strlen(buf) - 1] = ' ';

    // Ignore leading spaces
    while (buf && (*buf == ' '))
        ++buf;

    // Build argv
    argc = 0;
    while ((del = strchr(buf, ' '))) {
        *del = '\0';
        argv[argc++] = buf;

        buf = del + 1;

        while (buf && (*buf == ' '))
            ++buf;
    }
    argv[argc] = NULL;

    if (argc == 0)
        return -1;
    
    builtincmd(argv);

    return 0;
}