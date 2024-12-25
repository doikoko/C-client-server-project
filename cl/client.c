#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ncurses.h>
#include <signal.h>

#include "../lib/cl_lib.h"

#define MAX_MESSAGE 255

int main(int argc, char **argv){
    if(argc != 3){
        printf("------ARGUMENTS-LIST------\n1)IP\n2)PORT\n--------------------------\n");
        return 1;
    }
    char *IP = argv[1];
    char *PORT = argv[2];

    int sockfd;
    char buffer[MAX_MESSAGE];
    struct sockaddr_in addr;
    socklen_t socklen;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        error("socket error");
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(PORT));
    if((inet_pton(AF_INET, IP, &addr.sin_addr)) < 0){
        error("IP is not available");
    }
    
    socklen = sizeof(addr);
    if((connect(sockfd, (struct sockaddr*)&addr, socklen)) < 0){
        error("connection error");
    } else {
        printf("connected\n");
    }
    
    initscr();
    noecho();

    int max_x, max_y;

    Footer footer = {{"q/Q", "f/F", "v/V", "d/D", "c/C"}, {"QUIT", "VIEW File", "VIEW Dir", "DOWNLOAD", "ch direct"}};
 
    getmaxyx(stdscr, max_y, max_x);
    print_footer(footer, max_y, max_x);
    refresh();
    
    nodelay(stdscr, FALSE);
    char input;
    while(input = getch()){
        if(input == 'q' || input == 'Q'){
            if(quit_confirm(max_y, max_x)) break;
            else print_footer(footer, max_y, max_x);
        }
        if(input == 'f' || input == 'F'){
            view_file(sockfd);
        }
    }

    endwin();
    return 0;
}