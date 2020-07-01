#include <stdio.h>
#include <stdlib.h>

#define MAXARGS 128
#define MAXLINE 8192

void _print_buffer_(const char* buf) {
    // Assumes null-terminated buffer
    printf("%s", buf);
}

int main(int argc, char** argv) {

    char cmdline[MAXLINE];
    
    while(1) {
        printf(">> ");
        
        fgets(cmdline, MAXLINE, stdin);
        _print_buffer_(cmdline);
        
        // Check if user typed CTRL+d to exit shell
        if(feof(stdin))
            exit(0);

    }

    return EXIT_SUCCESS;
}