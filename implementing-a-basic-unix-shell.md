
# Implementing a Basic Unix Shell

## What is a Shell?
A shell is a user level-process that runs commands and programs on behalf of a user in a text-based manner. For instance, when a system is booted and a user logs in, a shell process is automatically started. 

## Program Set-Up
The shell source code will be written in a single file called `shell.c`.  The following code block demonstrates the structure of the shell. Subsequent sections will introduce the missing prototypes and implementations.

```c
/* shell.c */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXARGS 16
#define MAXLINE 256

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
```

Two symbolic constants, `MAXARGS` and `MAXLINE`, provide an upper bound on the number of characters for arguments and character length of the overall command, respectively. 

## Read User Input
Our shell needs a way to prompt, read, and store input from the user. For now, we assume that all inputs originate from `stdin`. Reading whole-line inputs from `stdin` will need to be repeated however many times the user wishes.

```c
void read_cmd(char* buf);

void read_cmd(char* buf) {
    printf(">>> ");                                   /* print shell prompt */
    if (fgets(buf, MAXLINE, stdin) == NULL)           /* read & store line, check for EOF or error */
        exit(0);                                      /* exit on EOF or error */
}
```

## Evaluate User Input
Once the data available in `stdin` have been read into the buffer, it must be evaluated by parsing and tokenization. For simplicity, we assume the line is delimited by spaces. Parsing the buffer allows the shell to count the number of arguments available and construct the `argv` array.

```c
void parse_cmd(char* buf, char** argv, int* argc);

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
```
> Quick Tip: `argv` is a NULL-terminated array of NULL-terminated strings. It is required to run external programs.

## Built-in Commands
Built-in commands are run by the shell process rather than in the context of a child process. Therefore, the shell needs a way to determine whether the user has typed a built-in command before attempting to run an external executable.

|Built-in Commands  |Description  |
|--|--|
|  `help`|  prints useful information about the shell.|
|  `exit`|	terminates the shell process.|
|  `whoami`| prints the value of USER environment variable.|


The `builtin_cmd()` function is utilized in the next section. 
```c
int builtin_cmd(char** buf);

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
```

## Run a Program in a Child Process
By this point, the shell is capable of distinguishing built-in commands from external programs. If the user has elected to run a program (e.g. `/bin/ls`), we will need to create a child process that will have its address space overwritten with that of the selected program. 

Unix operating systems provide three systems calls that facilitate this form of process control. 
| System Call | Description  |
|---------|--|
| `fork()` |creates a new child process. On success, it returns the process identification (PID) of the child to the parent and returns 0 to the child.
| `execv()` | overwrites the calling process' address space with that of a new program. On success, it does not return. |
| `wait()` | suspends execution of the calling process and waits for a child process to terminate. If a child process has already terminated at the time it is called, it returns the PID of the child. |

Error-checking wrapper functions are used for `fork()` and `execv()`.
```c
pid_t Fork(void);
void Execv(const char* path, char *const argv[]);
void  exec_cmd(char** argv,  int argc);

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
```

> Quick Tip: the operating system scheduler decides when either the parent or child process runs. Therefore, the parent can run first, and then the child or vice-versa.  However, using `wait()` halts the parent process until a child process has terminated.

> Quick Tip: more information on the systems calls can be obtained by referring to their respective man-page entries (e.g. `man execv` or `man fork`).

## Piecing Together the Shell
```c
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
```

> Quick Tip:  compile the shell using gcc (e.g. `gcc shell.c -o shell`).

> Quick Tip: debugging information can be generated by passing the `-g` option to `gcc` for use with `gdb`. 

## Reaping Child Processes and Avoiding Zombies
When a child process terminates, `wait()` signals the operating system that the process has completed. This permits the operating system to *reap* the process. In turn freeing system resources used to maintain the process (e.g. process control block).

If `wait()` is not called the child process still runs until it terminates. However, it will continue to consume system resources because it was not reaped. This is referred to as a *zombie* state. For processes that have long-running times and spawn multiple children throughout their execution, they must reap their children to free resources.

## Additional Reading & Sources
[http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-intro.pdf](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-intro.pdf)

[http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf)

[http://pages.cs.wisc.edu/~remzi/OSTEP/vm-intro.pdf](http://pages.cs.wisc.edu/~remzi/OSTEP/vm-intro.pdf)

[https://en.wikipedia.org/wiki/Unix_shell](https://en.wikipedia.org/wiki/Unix_shell)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMjExODg1ODk2OSwtMTAzOTM0MzE0NiwtMz
gwMDAwNjg1LC0xOTYwOTQ2NzkzLC0xMTk0NDUyNDQzLC0xMjc0
Nzc2MzQ1LDE3NTA0NDE0MzIsLTEyMjk5Nzg3NzMsMTQ4NDg1Mj
gwNCwtNTQyMjYzODQ4LDE0NjQ5NzQ3ODIsLTkxMjU1MDUzMywt
NTUxNzQ3NiwyMjk0Nzg5NzYsMTA0MDQ5MTMyMiwtNDgyODM0Ny
wtNTcxMTMwNjUxLC04NjMyNTY1NiwtMTUzNjk3Mjg4NywtMjA4
NDY1Mjg3NV19
-->