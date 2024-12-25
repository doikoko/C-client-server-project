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

#define MAX_MESSAGE 255

void error(const char *messge){
    perror(messge);
    exit(1);
}

void s_send_file_text(const char *filename){
    return;
}
void s_view_directory(){
    return;
}
void s_download(){
    return;
}
void s_change_directory(){
    return;
}

void wait_command(int sockfd){
    pid_t pid = fork();
    if(pid == 0){
        Command command = recieve_command(sockfd);
        while(1){
            if(command.index != 0){
                switch (atoi(command.index)){
                case 2:
                    s_send_file_text("filename");
                    break;
                case 3:
                    s_view_directory();
                    break;
                case 4:
                    s_download();
                    break;
                case 5:
                    s_change_directory();
                    break;
                default:
                    break;
                }
            }
        }
        exit(0);
    }
    return;
}
Command recieve_command(int sockfd){
    Command command;

    int bytes_recieved;
    while((bytes_recieved = recv(sockfd, command.value, sizeof(command.value), 0)) > 0){
        if(bytes_recieved == 1){
            command.index[0] = command.value[0];
            bzero(command.value, sizeof(command.value)); 
        }
    }
    return command;
}

int send_file(int sockfd, char *filename){
    FILE *f = fopen(filename, "r");
    int bytes_sending;
    char buffer[MAX_MESSAGE];

    if(f == NULL){
        return -1;
    }

    while(bytes_sending = fread(buffer, sizeof(char), sizeof(buffer), f)){
        send(sockfd, buffer, bytes_sending, 0);
    } 

    fclose(f);
    return 0;
}
