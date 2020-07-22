/*
    Program: tictactoe.c
    text-based two-player tic tac toe.
*/

#include <stdio.h>
#include <stdlib.h>

#define DIM 3

static unsigned short turn_count = 0;

void init_board(char* board) {
    int i;
    for (i = 0; i < DIM * DIM; ++i)
        board[i] = ' ';
}

void print_board(char* board) {
    printf("***********\n");
    printf(" %c | %c | %c \n", board[0], board[1], board[2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[3], board[4], board[5]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[6], board[7], board[8]);
    printf("***********\n");
}

int valid_index(int index) {
    if (index >= 0 && index < DIM * DIM)
        return 1;
    return 0; 
}

int position_taken(char* board, int index) {
    if (board[index] == ' ')
        return 0;
    return 1;
}

int get_index() {
    static char buf[DIM * DIM]; 
    printf("Enter position [1-9]: ");
    fgets(buf, DIM * DIM, stdin);
    if (feof(stdin))
        exit(0);
    int pos = atoi(buf); 
    return pos - 1;
}

void set_position(char* board, int index) {
    board[index] = (turn_count % 2 == 0 ? 'X' : 'O');
}

void print_player() {
    printf("Current Player: %c\n", (turn_count % 2 == 0 ? 'X' : 'O'));
}

int game_win(char* board) {
    char player = turn_count % 2 == 0 ? 'X' : 'O';

    /* Horizontals */
    if (board[0] == player &&
        board[1] == player &&
        board[2] == player)
        return 1;
        
    if (board[3] == player &&
        board[4] == player &&
        board[5] == player)
        return 1;
        
    if (board[6] == player &&
        board[7] == player &&
        board[8] == player)
        return 1; 
    
    /* Verticals */
    if (board[0] == player &&
        board[3] == player &&
        board[6] == player)
        return 1;
        
    if (board[1] == player &&
        board[4] == player &&
        board[7] == player)
        return 1;
        
    if (board[2] == player &&
        board[5] == player &&
        board[8] == player)
        return 1;
        
    /* Diagonals */
    if (board[0] == player &&
        board[4] == player &&
        board[8] == player)
        return 1;

    if (board[2] == player &&
        board[4] == player &&
        board[6] == player)
        return 1; 
        
    return 0;
}

int game_end(char* board) {
    if (turn_count > 9) {
        printf("Stalemate. Game Over!\n");
        return 1;
    } else if (game_win(board)) {
        printf("\nPlayer %c won!\n", (turn_count % 2 == 0 ? 'X' : 'O'));
        return 1;
    } else {
        return 0;
    }
}

int main() {
    static char board[DIM * DIM];
    int index;
    init_board(board);
    printf("*** To quit type ctrl-c or ctrl-d ***\n\n");
    while (1) {
        print_player();
        print_board(board);
        index = get_index();
        
        while (!valid_index(index)) {
            printf("Invalid position. Try again.\n");
            index = get_index();
        }
            
        while (position_taken(board, index)) {
            printf("Position taken. Try again.\n");
            index = get_index();
        }
        
        set_position(board, index);
        
        if (game_end(board)) {
            print_board(board);
            printf("Good bye!\n");
            break;
        }
        ++turn_count;
    }
    return 0;
}