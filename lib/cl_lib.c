#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <ncurses.h>

#include "cl_lib.h"

#define MAX_MESSAGE 255

void error(const char *messge){
    perror(messge);
    exit(1);
}

void print_footer(Footer footer, int max_y, int max_x){
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    for(int i = 0; i < 5; i++){
        move(max_y - 1, max_x / 6 * (i + 1) - 5);
    
        printw("%s ", footer.keys[i]);

        if(has_colors){
            attron(COLOR_PAIR(1));
            printw("%s", footer.values[i]);
            attroff(COLOR_PAIR(1));
        } else {
            printw("%s", footer.values[i]);
        }
    }
    move(0, 0);
}
int quit_confirm(int max_y, int max_x){
    move(max_y - 1, 0);
    clrtoeol();
    if(has_colors) attron(COLOR_PAIR(1));
    
    move(max_y - 1, max_x / 2 - 10);
    printw("CONFIRM: (y/Y) BACK: (n/N)\t");
    
    nodelay(stdscr, FALSE);

    int status;
    char input = getch();
    if(input == 'y' || input == 'Y') status = 1;
    else status = 0;
    
    if(has_colors) attroff(COLOR_PAIR(1));
    
    move(max_y - 1, 0);
    clrtoeol();

    return status;
}


void send_command(int sockfd, Command command) {   
    send(sockfd, command.index, 1, 0);
    send(sockfd, command.value, strlen(command.value), 0);
}

int recieve_file(int sockfd, char *filename){
    FILE *f = fopen(filename, "w");
    int bytes_recieved;
    char buffer[MAX_MESSAGE];

    if(f == NULL){
        return -1;
    }

    while((bytes_recieved = recv(sockfd, buffer, sizeof(buffer), 0)) > 0){
        fprintf(stdout, "%s", buffer);
    }

    fclose(f);
    return 0;
}
int view_file(int sockfd, char *filename){
    int bytes_recieved;
    char buffer[MAX_MESSAGE];

    while((bytes_recieved = recv(sockfd, buffer, sizeof(buffer), 0)) > 0){
        wprintw(stdscr, "%s", buffer);
    }

    return 0;
}