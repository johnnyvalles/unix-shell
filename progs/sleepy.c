/*
    Program: sleepy.c
    Suspends program execution for argv[1] seconds if given or 5 seconds
    by default.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
    printf("About to sleep.\n");
    (argc > 1) ? sleep(atoi(argv[1])) : sleep(5);
    printf("Done sleeping.\n");
    return 0;
}