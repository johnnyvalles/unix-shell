
# Implementing a Basic Unix Shell

## Prerequisites
A basic understanding of the following language and concepts:
* C programming language: data types, functions, pointers and string representation
* Programs, processes, address space, zombies and reaping 
* System calls: `fork()`, `execv()`, `wait()` or alternatively `execve()` and `waitpid()`
* Navigating directories and executing commands with a shell (e.g bash, zsh)

If you find any of these topics completely foreign or need a refresher, please refer to the OSTEP articles linked in the *Additional Reading & Sources* section located at the end.

## Introduction
In this article, you will learn how to implement a basic shell that can be compiled and executed on Unix-based operating systems (e.g. macOS and Linux distributions). The shell will allow a user to run programs in the following fashion using absolute (1-3) and relative paths (4-5). 
1. `/bin/ls -al`
2. `/bin/ps aux`
3. `/bin/echo "Hello, World!"`
4. `program1 arg1 arg2 arg3`
5. `./program2 arg1`

The implementation is considered basic due to the inability to run programs in a background process (i.e. using `&` operator when invoking a program), lack of signal handling for signals that may be delivered to the shell while it is running, I/O redirection and pipelining.

However, the implementation is thorough enough to grant the programmer freedom with implementing new functionality. Thus, providing a foundation upon which the aforementioned features can be added.

Before proceeding, it is highly recommended to have some form of documentation available for the functions and system calls used throughout the article. The quickest way to obtain documentation for a function is to use its manual page, otherwise known as a *man page*. This page can easily be brought up in the following manner right from a terminal:
* `man echo`
* `man strcmp`
* `man exevc`
* `man fork`

Searching for a particular entry in a man page can be done using `grep` and optionally passing its output to a program like `echo` or `less` via a pipe (i.e. `|`).

## Outcomes
Upon completion of the article, you will have an understanding of the fundamentals of system calls, operating system interfaces, and abstractions for process control that facilitate the implementation of a Unix shell. The information presented can then be used to further your study of computer science and systems programming.

## What is a Shell?
A shell is a program that can run commands and programs on behalf of a user. Prior to the emergence of graphical user interfaces (GUIs), a user had to use a shell to run commands and programs on a computer in a text-based manner. Simply put, a shell reads in a command that a user types (e.g. `stdin`), evaluates the input by parsing and tokenizing the command to determine its validity. It then proceeds to build any necessary data structures for the program to run, creates a new process, and runs that program within the context of that new process. This is all accomplished using system calls provided by the operating system for process control. It is vital to make the distinction that a shell is just an ordinary program that runs as a user-level process. We will implement it as a series of steps:
1. Program set up
2. Read user input
3. Evaluate user input
4. Built-in commands 
5. Run a program in a child process

## Program Set Up
The shell source code will be written in a single file called `shell.c`. The header files in the code block below must be included in the program to enable I/O and the use of system calls provided by the operating system. Additionally, we need to define two symbolic constants `MAXARGS` and `MAXLINE`. The two will provide an upper bound on the number of characters for arguments a user can provide with a command and the character length of the overall command, respectively.

```c
/* shell.c */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXARGS 128
#define MAXLINE 8192

int main() {

    return EXIT_SUCCESS;
}
```

The shell will be compiled using `gcc` in the following manner:
```
gcc shell.c -o shell
```
For generating an executable object file with debugging information, compile `shell.c` with the `-g` option (e.g. debugging with `gdb`).

## Read User Input
Our shell needs a way to prompt, read, and store input from the user. For now, we assume that all of the inputs will come from `stdin`. Reading whole-line inputs from `stdin` can easily be accomplished using `fgets()`. `fgets()` reads in at most one character less than the specified maximum (i.e. accounts for the NULL character) and includes the newline character. We will need to repeat this process however many times the user wishes. Thus, it is best to write this functionality in a while-loop.

```c
/* shell.c */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXARGS 128
#define MAXLINE 8192

int main() {
    char buf[MAXLINE];                                      /* buffer holds user's input */
    while(1) {
        printf(">>> ");                                     /* print shell prompt */
        if (fgets(buf, MAXLINE, stdin) == NULL)             /* read and store line, check for end-of-file or error */
            exit(0);
    }
    return EXIT_SUCCESS;
}
```

## Evaluate User Input
Once the data available in `stdin` have been read into the buffer, it must be evaluated by parsing and tokenization. For simplicity, we assume the line is delimited by spaces. Parsing the command allows the shell to determine whether the line pertains to a built-in command (e.g. pwd) or a program name. Otherwise, an error message is printed to `stderr` informing the user that the command was invalid. Our shell has two built-in commands: `exit` and `help`. The former terminates the shell process and the latter prints out helpful information for using our shell.

Since `fgets()` retains the newline character, we need to replace it with a space character to correctly parse the assumed space-delimited command string (i.e. replace the last character with space). Once the character has been replaced, we can move onto tokenizing the command string. Once again, the use of a while-loop is appropriate. 

The parsing functionality will be written in the function `parse_cmd()`. This function is also responsible for building the `argv` array, a NULL-terminated array of pointers to NULL-terminated character strings. By convention, the first element of `argv` is the name of the program executable file. The remaining elements, if any, are arguments or options for that program (e.g. `/bin/ls -al`). 


The prototype and definition for `parse_cmd()`are presented below.

```c
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
    int i;                                                  /* holds number of arguments parsed (argc) */
    
    buf[strlen(buf) - 1] = ' ';                             /* replace '\n' with a space */
    
    while (buf && (*buf == ' '))                            /* ignore leading spaces */
        ++buf;
    
    i = 0;
    while ((del = strchr(buf, ' '))) {                      /* construct argv */
        *del = '\0';
        argv[i++] = buf; 
        buf = del + 1;
        
        while (buf && (*buf == ' '))                        /* ignore leading spaces */
            ++buf;
    }

    argv[i] = NULL;                                         /* NULL-terminate argv */
    
    if (i == 0)
        return;
}
```

## Built-in Commands
The shell has two built-in commands: `exit` and `help`. These commands are run by the shell process rather than in the context of a child process. `exit` allows a user to terminate the shell if they wish to do so. `help` displays useful information about the shell, such as the version number or a list of built-in commands. 

Thus, the shell needs a way to determine whether the user has typed a built-in command or the name of a program. This functionality will be written in the `builtin_cmd()` function, utilizing the `strcmp()` function. If the input is a built-in command, it runs the command and proceeds to return 1, 0 otherwise. The prototype and definition are presented below. The use of the function will be showcased in the next section.

```c
int builtin_cmd(char** buf);

int builtin_cmd(char** argv) {
    if (!strcmp(*argv, "exit"))
        exit(0);
    
    if (!strcmp(*argv, "help")) {
        printf("Help command.\n");
        return 1;
    }

    return 0;
}
```

## Run a Program in a Child Process
By this point, the shell is capable of distinguishing built-in commands from external programs. In the event that the user has elected to run a program (e.g. `/bin/ls`), we will need to create a child process that will have its address space overwritten with that of the selected program. Our shell is not capable of running background processes, so instead, it will create a child process and halt until the child process has terminated. 

Unix operating systems provide three systems calls that facilitate this form of process control. A brief description of the three follows.

1. `fork()`: creates a new child process. This system call is unique in the sense that it returns twice. On success, it returns the process identification (PID) of the child to the parent and returns 0 to the child. When invoked, it creates a copy of the calling process' address space for the child. However, after forking, the two processes, although containing identical symbols, do not share them (e.g. variable names or functions). Unless the parent process is explicitly instructed to wait on the child, the order in which they run is completely up to the operating system scheduler. Thus the order can vary from system-to-system.
2.  `exec()`: overwrites the calling process' address space with that of a new program. On success, it does not return.
3.  `wait()`: suspends execution of the calling process and waits for a child process to terminate. If a child process has already terminated at the time it is called, it returns the PID of the child.

Executing the specified program in the context of a new process will be handled by the `exec_cmd()` function, which is invoked from `parse_cmd()`. This is where the shell's abilities begin to shine by utilizing the three systems calls introduced in the *Prerequisites* section. If `exec_cmd()` fails to run the program specified by the path in `argv[0]`, and an error message is printed from the child process to `stdout` and then terminates. 

Recall that the scheduler decides when either the parent or child process runs. Therefore, it is possible for the parent to run first and then the child or vice-versa. However, the parent process can be halted until a child process it spawned has terminated by using `wait()`. That way if the parent process runs first, it will immediately halt. 

```c
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
```

## Reaping Child Processes and Avoiding Zombies
As mentioned in the previous section, `wait()` suspends execution of the calling process until a child process has terminated. Once a child process has terminated, `wait()` also signals to the operating system that the process has completed and no longer has to be kept around in memory. This allows the operating system to `reap` the process. Reaping terminated processes frees up resources and process data structures used during their execution.

If `wait()` is not called then the child process will run but once terminated, will continue to consume system resources used to represent the process. This state where the process is no longer running but is still in memory is known as a `zombie` state. For processes that have long-running times and spawn multiple children throughout their execution, it is vital that they reap their children to free system resources.

In the final code block of the previous section, it is possible to remove the `wait()` call to further understand the concepts of reaping zombie children. Removing the call produces child processes that are in a `zombie` state and can be examined right from the shell. Checking for zombie processes can be accomplished by invoking the `ps` program using an absolute path to its executable file (i.e `/bin/ps`).

## Zombie Process Example (optional)
This section will make use of the `ps` program to demonstrate what happens if the shell does not call `wait()` when it is running (i.e. remove or comment out the call in `shell.c`). For more information on the usage of `ps`, please reference its man page. Moreover, the repository for this article contains example programs and a Makefile to compile all of the programs. A description of what each program does is given in their respective source file.  

## Additional Reading & Sources
http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-intro.pdf

http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf

http://pages.cs.wisc.edu/~remzi/OSTEP/vm-intro.pdf

https://en.wikipedia.org/wiki/Unix_shell
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTM2MTg1MzkyNyw2NzQxMDg4MjMsODc5MD
A1NDg5LDE1NzYyMTQxOTMsMTEzMzY4MzM0NywyMDgyOTE4NjMy
XX0=
-->