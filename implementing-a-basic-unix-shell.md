# Implementing a Basic Unix Shell

## Prerequisites
A basic understanding of the following language and concepts:
* C programming language: data types, functions, pointers and string representation
* Programs, processes, address space, zombies and reaping 
* System calls: `fork()`, `exec()`, `wait()`
* Navigating directories and executing commands with a shell (e.g bash, zsh)

If you find any of these topics completely foreign or need a refresher, please refer to the OSTEP articles linked in the *Additional Reading & Sources* section located at the end.
## Introduction
In this article, you will learn how to implement a basic shell that can be compiled and executed on Unix-based operating systems (e.g. macOS and Linux OS family). The shell will allow a user to run programs in the following fashion from the root directory (1-3), working directory (4), and absolute path (5):
1. `/bin/ls -al`
2. `/bin/ps aux`
3. `/bin/echo "Hello, World!"`
4. `program1 arg1 arg2 arg3`
5. `/path/to/the/program`

The implementation is considered basic due to the inability to run programs in a background process (i.e. using `&` operator when invoking a program), lack of signal handling for signals that may be delivered to the shell while it is running, I/O redirection and pipelining.

However, the implementation is thorough enough to grant the programmer freedom with implementing new functionality. Thus, providing a foundation upon which the aforementioned features can be added.

## Outcomes
Upon completion of the article, you will have an understanding of the fundamentals of system calls, operating system interfaces and abstractions for process control that facilitate the implementation of a Unix shell. The information presented can then be used to further your study of computer science and systems programming.

## What is a Shell?
A shell is a program that can run commands and programs on behalf of a user. Prior to the emergence of graphical user interfaces (GUIs), a user had to use a shell to run commands and programs on a computer in a text-based manner. Simply put, a shell reads in a command that a user types (e.g. `stdin`), evaluates the input by parsing and tokenizing the command to determine its validity. It then proceeds to build any necessary data structures for the program to run, creates a new process and runs that program within the context of that new process. This is all accomplished using system calls provided by the operating system for process control. It is vital to make the distinction that a shell is just an ordinary program that runs as a user level process. We will implement it as a series of steps:
1. Program set up
2. Read user input
3. Evaluate user input
4. Create a new child process
5. Execute command in the new child process
6. Repeat

## Program Set Up
The shell source code will be written in a single file called `shell.c`. The header files in the code block below must be included into the program to enable I/O and use of system calls provided by the operating system. Additionally, we need to define two symbolic constants `MAXARGS` and `MAXLINE`. The two will provide an upperbound on the number of characters for arguments a user can provide with a command and the character length of the overall command, respectively.

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

int
main() {

    return EXIT_SUCCESS;
}
```

The shell will be compiled using `gcc` in the following manner:
```
gcc shell.c -o shell
```
For generating an executable object file with debugging information, compile `shell.c` with the `-g` option (e.g. debugging with `gdb`).

## Read User Input
Our shell needs a way to prompt, read and store input from the user. For now, we assume that all of the inputs will come from `stdin`. Reading whole-line inputs from `stdin` can easily be accomplished using `fgets()`. `fgets()` reads in at most one character less than the specified maximum (i.e. accounts for the NULL character) and includes the newline character. We will need to repeat this process however many times the user wishes. Thus, it is best to write this functionality in a while-loop.

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

int
main() {
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

Since `fgets()` retains the newline character, we need to replace it with a space character to correctly parse the assumed space-delimited command string (i.e. replace last character with a space). Once the character has been replaced, we can move onto tokenizing the command string. Once again, the use of a while-loop is appropriate. 

The parsing functionaly will be written in the function `parse_cmd()`. This function is also responsible for building the `argv` array, a NULL-terminated array of pointers to character strings. By convention, the first element of `argv` is the name of the program executable file. The remaining elements, if any, are arguments or options for that program (e.g. `/bin/ls -al`). 


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

void
parse_cmd(char* buf);

int
main() {
    char buf[MAXLINE];                                      /* buffer holds user's input */
    while(1) {
        printf(">>> ");                                     /* print shell prompt */
        if (fgets(buf, MAXLINE, stdin) == NULL)             /* read and store line, check for end-of-file or error */
            exit(0);
        parse_cmd(buf);                                     /* parse the command */
    }
    return EXIT_SUCCESS;
}

void
parse_cmd(char* buf) {
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


## Create a New Child Process
## Execute Command in the New Child Process
## Reaping Child Processes and Avoiding Zombies
## Additional Reading & Sources
http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-intro.pdf

http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf

https://en.wikipedia.org/wiki/Unix_shell