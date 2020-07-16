/*
    Program: hello.c
    Prompts the user for their name and greets them.
*/

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    static char buf[128];
    printf("What is your name? ");
    fgets(buf, 128, stdin);
    buf[strlen(buf) - 1] = '\0';
    printf("Hello, %s. Nice to meet you!\n", buf);
    return 0;
}