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

#include "se_lib.h"

#define MAX_BUFFER 1024

void error(const char *messge){
    perror(messge);
    exit(1);
}

void send_file_text(int sockfd, const char *filename){
    printf("signal");
    // FILE *f = fopen(filename, "r");
    // int bytes_sending;
    // char buffer[MAX_BUFFER];

    // if(f == NULL){
    //     printf("now such file or directory\n");
    //     return;
    // }

    // while((bytes_sending = fread(buffer, 1, 1, f)) != EOF); 

    // printf("%s", buffer);
    // fclose(f);
}
void view_directory(){
    return;
}
void download(){
    return;
}
void change_directory(){
    return;
}

void wait_command(int sockfd){
    pid_t pid = fork();
    if(pid == 0){
        while(1){
            Command command = recieve_command(sockfd);
            switch (command.index[0]){
            case '2':
                send_file_text(sockfd, command.value);
                break;
            case '3':
                view_directory();
                break;
            case '4':
                download();
                break;
            case '5':
                change_directory();
                break;
            default:
                break;
            }
        }
        exit(0);
    }
    return;
}
Command recieve_command(int sockfd){
    Command command;

    int bytes_recieved = 0;
    recv(sockfd, command.index, 1, 0);
    recv(sockfd, command.value, sizeof(command.value), 0);
    
    return command;
}