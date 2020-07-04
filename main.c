#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>

#define MAXARGS 128
#define MAXLINE 8192

int main(int argc, char** argv) {

    int i;
    struct winsize terminal;
    char cmdline[MAXLINE];
    
    while(1) {
        printf("ROWS: %d, COLUMNS: %d\n", terminal.ws_row, terminal.ws_col);
        printf(">> ");
        
        fgets(cmdline, MAXLINE, stdin);
        
        // Check if user typed CTRL+d to exit shell
        if(feof(stdin))
            exit(0);

        if (strcmp(cmdline, "clear")) {
            ioctl(0, TIOCGWINSZ, &terminal);
            
            for (i = 0; i < terminal.ws_row - 2; ++i)
                printf("\n");

        }


    }

    return EXIT_SUCCESS;
}