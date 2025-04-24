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
#include <unistd.h>

#include "../lib/cl_lib.h"

#define MAX_MESSAGE 255
#define KEY_ESC 27

int sockfd;

void sigterm(){
    Command command;
    command.index[0] = '1';
    command.index[1] = '\0';
    strcpy(command.value, "KILL");

    send_command(sockfd, command);
    endwin();
    exit(0);
}
   
int main(int argc, char **argv){
    if(argc != 3){
        printf("------ARGUMENTS-LIST------\n1)IP\n2)PORT\n--------------------------\n");
        return 1;
    }
    char *IP = argv[1];
    char *PORT = argv[2];

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

    signal(SIGTERM, sigterm);
    signal(SIGINT, sigterm);
    
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
            if(quit_confirm(max_y, max_x)){ 
                sigterm();
                break;
            }
            else {
                wclear(stdscr);
                print_footer(footer, max_y, max_x);
                wprintw(stdscr, "canseled\n");
                divide(max_x);
                refresh();
            }
        }
        if(input == 'f' || input == 'F'){
            input = '0';
            if(!handle_user_input(sockfd, max_y, max_x, footer, "ENTER FILENAME", '2', "no such file", view_file_text))
                continue;
        }
        if(input == 'v' || input == 'V'){
            input = '0';
            if(!handle_user_input(sockfd, max_y, max_x, footer, "ENTER DIRNAME", '3', "no such dir", view_directory))
                continue;
        }
        if(input == 'd' || input == 'D'){
            input = '0';
            if(!handle_user_input(sockfd, max_y, max_x, footer, "ENTER NAME TO DOWNLOAD", '4', "no such file or directory", download))
                continue;
        }
        if(input == 'c' || input == 'C'){
            input = '0';
            // if(!handle_user_input(sockfd, max_y, max_x, footer, "ETNER DIRNAME", '5', "no such dir", change_directory));
        }
    }

    endwin();
    close(sockfd);
    return 0;
}
