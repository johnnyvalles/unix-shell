

# Implementing a Basic Unix Shell

## What is a Shell?
A shell is a program that can run commands and programs on behalf of a user in a text-based manner. Simply put, a shell reads in a command that a user types (e.g. `stdin`), evaluates the input by parsing and tokenizing the command to determine its validity. Finally, it builds any data structures necessary for the program to run, creates a new process, and runs that program within that context.

## Program Set Up
The shell source code will be written in a single file called `shell.c`. The header files in the code block below must be included in the program to enable I/O and the use of system calls provided by the operating system. Additionally, we need to define two symbolic constants `MAXARGS` and `MAXLINE`. The two will provide an upper bound on the number of characters for arguments a user can provide and the character length of the overall command, respectively.

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

int main() {

    return EXIT_SUCCESS;
}
```

The shell will be compiled using `gcc` in the following manner:
```
gcc shell.c -o shell
```
To generate an executable object file with debugging information, compile `shell.c` with the `-g` option (e.g. debugging with `gdb`).

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

#define MAXARGS 16
#define MAXLINE 256

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

The parsing functionality is written in the function `parse_cmd()`. This function is also responsible for building the `argv` array, a NULL-terminated array of pointers to NULL-terminated character strings. By convention, the first element of `argv` is the name of the program executable file. The remaining elements, if any, are arguments or options for that program (e.g. `/bin/ls -al`). 


The prototype, definition, and usage of `parse_cmd()`are presented below.

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
}
```

## Built-in Commands
The shell has two built-in commands: `exit` and `help`. These commands are run by the shell process rather than in the context of a child process. `exit` allows a user to terminate the shell if they wish to do so. `help` displays useful information about the shell, such as the version number or a list of built-in commands. 

Thus, the shell needs a way to determine whether the user has typed a built-in command or the name of a program. This functionality is written in the `builtin_cmd()` function, utilizing the `strcmp()` function. If the input is a built-in command, it runs the command and proceeds to return 1, 0 otherwise. The prototype and definition are presented below. The use of the function will be showcased in the next section in `parse_cmd()`.

```c
int builtin_cmd(char** buf);

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
```

## Run a Program in a Child Process
By this point, the shell is capable of distinguishing built-in commands from external programs. If the user has elected to run a program (e.g. `/bin/ls`), we will need to create a child process that will have its address space overwritten with that of the selected program. Our shell creates a child process and halts until the child process has terminated. 

Unix operating systems provide three systems calls that facilitate this form of process control. A brief description of the three follows.

1. `fork()`: creates a new child process. This system call is unique in the sense that it returns twice. On success, it returns the process identification (PID) of the child to the parent and returns 0 to the child. When invoked, it creates a copy of the calling process' address space for the child. However, after forking, the two processes, although containing identical symbols, do not share them (e.g. variable names or functions). Unless the parent process is explicitly instructed to wait on the child, the order in which they run is completely up to the operating system scheduler. Thus the order can vary from system-to-system.
2.  `exec()`: overwrites the calling process' address space with that of a new program. On success, it does not return.
3.  `wait()`: suspends execution of the calling process and waits for a child process to terminate. If a child process has already terminated at the time it is called, it returns the PID of the child.

Executing the specified program in the context of a new process is handled by the `exec_cmd()` function, which is invoked from `parse_cmd()`. This is where the shell's abilities begin to shine by utilizing the three systems calls introduced in the *Prerequisites* section. If `exec_cmd()` fails to run the program specified by the path in `argv[0]`, an error message is printed from the child process to `stdout` and then terminates. 

Recall that the scheduler decides when either the parent or child process runs. Therefore, the parent can run first, and then the child or vice-versa. However, the parent process can be halted until a child process it spawned has terminated by using `wait()`. That way if the parent process runs first, it immediately halts. 

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

void exec_cmd(char** argv) {
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
```

## Reaping Child Processes and Avoiding Zombies
As mentioned in the previous section, `wait()` suspends execution of the calling process until a child process has terminated. Once a child process has terminated, `wait()` also signals to the operating system that the process has completed and no longer has to be kept around in memory. This allows the operating system to *reap* the process. Reaping terminated processes frees up resources and process data structures used during their execution.

If `wait()` is not called then the child process will run but once terminated, will continue to consume system resources used to represent it. This state where the process is no longer running but is still in memory is known as a *zombie* state. For processes that have long-running times and spawn multiple children throughout their execution, they must reap their children to free system resources.

In the final code block of the previous section, it is possible to remove the `wait()` call to further understand the concepts of reaping zombie children. Removing the call produces child processes that are in a `zombie` state and can be examined right from the shell. Checking for zombie processes from our shell can be accomplished by invoking the `ps` program using an absolute path to its executable file (e.g. `/bin/ps`).

## Zombie Process Example (optional)
This section will make use of the `ps` program to demonstrate what happens if the shell does not call `wait()` when it is running (i.e. remove or comment out the call in `shell.c`). For more information on the usage of `ps`, please refer to its man page. Moreover, the repository for this article contains example programs and a Makefile to compile them all. A description of what each program does is given in their respective source file. To continue smoothly, it is recommended to clone or download the repository contents.

Begin by opening up a terminal and navigating to the directory where the shell source file is located. Ensure that the directory contains the makefile and `progs/` directory as in the repository. Finally, run make which will compile the shell and any programs located in `progs/`. 

```
johnny@dev-vm:~/Desktop/unix-shell$ make
gcc  shell.c -o shell
gcc     progs/sleepy.c   -o progs/sleepy
gcc     progs/hello.c   -o progs/hello
gcc     progs/tictactoe.c   -o progs/tictactoe
gcc     progs/args.c   -o progs/args
johnny@dev-vm:~/Desktop/unix-shell$ 

```
To remove all executables, run `make clean`.

Go ahead and clear the screen and run the compiled shell program. If everything has compiled without errors up to this point, you will be prompted with `>>>`.
```
johnny@dev-vm:~/Desktop/unix-shell$ ./shell
>>> 
```

Run the `sleepy` program from the shell. Please note that because the shell no longer calls `wait()`, the output and input prompts may not follow the program flow from the source files due to context switching. If at any point after running a program you are greeted with a blank line with a flashing cursor, hit `enter` or `return`. This causes the shell to read in an empty line, ignores it, and prompts with `>>>`.
```
johnny@dev-vm:~/Desktop/unix-shell$ ./shell
>>> progs/sleepy 3
>>> About to sleep.
Done sleeping.

>>> 
```

After `sleepy` has completed, run `ps` using an absolute path. This will produce output similar to the following:
```
>>> /bin/ps
>>>    PID TTY          TIME CMD
  2490 pts/0    00:00:00 bash
  3656 pts/0    00:00:00 shell
  3657 pts/0    00:00:00 sleepy <defunct>
  3658 pts/0    00:00:00 ps
```
Notice that the process with PID of 3657, is in a `<defunct>` state. The `sleepy` program ran and terminated, but it was never reaped by its parent process (i.e. our shell) and it is now a zombie. Thus, although it is no long-running, it is still consuming system resources. Running the `ps` program once more will also list the previous `ps` instance as defunct. This may not seem too serious for our simple Unix shell. Unfortunately, as mentioned in a previous section, long-running processes that spawn multiple child processes should always reap their terminated children to avoid unnecessary consumption of system resources.

## Additional Reading & Sources
[http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-intro.pdf](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-intro.pdf)

[http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf)

[http://pages.cs.wisc.edu/~remzi/OSTEP/vm-intro.pdf](http://pages.cs.wisc.edu/~remzi/OSTEP/vm-intro.pdf)

[https://en.wikipedia.org/wiki/Unix_shell](https://en.wikipedia.org/wiki/Unix_shell)
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTY1MTQyMzY2MSwyNDc5NTg3NDUsLTIwMz
kyMTU4NzIsLTE2MjAxODkyOSwtNzI0ODg3MDE2LDc3MDg2MzM5
MSwtOTI5NTIwMjI4LC05OTQ3NzA5NzQsLTI5ODIyMjgzNiwtMT
c1ODc5MTcxNiwtMjA0Mzc0MTk0OCwtOTM5NDU5MzUxLC0xMTk4
Mzg2NjQ3LC0xODI5OTEyMzQwLC0xNzMzNDg1ODYsLTg5NDM1Nj
I2LC00MDI5OTE2NjMsLTE1ODYzNjAzODYsLTEyNDU2NTc1NjQs
LTEwMjU2MjY0NjBdfQ==
-->