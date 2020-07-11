# Implementing a Basic Unix Shell

## Prerequisites
A basic understanding of the following language and concepts:
* C programming language: data types, functions, pointers and string representation
* Programs, processes, address space, zombies and reaping 
* System calls: `fork()`, `exec()`, `wait()`

## Introduction
In this article, you will learn how to implement a basic shell that can be compiled and executed on Unix-based operating systems (e.g. macOS and Linux OS family). The shell will allow a user to run programs in the following fashion from the root directory (1-3), working directory (4), and absolute path (5):
1. `/bin/ls -al`
2. `/bin/ps aux`
3. `/bin/echo "Hello, World!"`
4. `program1 arg1 arg2 arg3`
5. `/path/to/the/program`

The implementation is considered basic due to the inability to run programs in a background process (i.e. using `&` operator when invoking a program), lack of signal handling for signals that may be delivered to the shell while it is running, I/O redirection and pipelining. However, the implementation is thorough enough to grant the programmer freedom with implementing new functionality. Thus, providing a foundation upon which the aforementioned features can be added.

## Outcomes
Upon completion of the article, you will have a basic understanding of the fundamentals of system calls, operating system interfaces and abstractions for process control that facilitate the implementation of a Unix shell. The information presented can then be used to further your study of computer science and systems programming.

## What is a shell?
A shell is a user program that can run commands and programs on behalf of a user. Prior to the emergence of graphical user interfaces (GUIs), a user had to use a shell to run commands and programs on a computer in a text-based manner. Simply put, a shell reads in a command that a user types (e.g. `stdin`), evaluates the input by parsing and tokenizing the command to determine its validity, builds any necessary data structures for the program to run, creates a new process and runs that program within the context of that new process. This is all accomplished by using system calls provided by the operating system for process control. It is vital to make the distinction that a shell is just an ordinary program that runs as a user level process. We will implement it in a series of steps:
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
```

The shell will be compiled using `gcc` in the following manner:
```
gcc shell.c -o shell
```
For generating an executable object file with debugging information, compile `shell.c` with the `-g` option (e.g. debugging with `gdb`).

## Read User Input
## Evaluate User Input
## Create a New Child Process
## Execute Command in the New Child Process
## Reaping Child Proceses and Avoiding Zombies
## Additional Reading & Sources
http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-intro.pdf

http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf

https://en.wikipedia.org/wiki/Unix_shell