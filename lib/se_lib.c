#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ncurses.h>
#include <dirent.h>

#include "se_lib.h"

#define MAX_MESSAGE 1024

const int max_value_length = 50;
void error(const char *messge){
    perror(messge);
    exit(1);
}

int command_handler(int sockfd, int token){
    pid_t pid = fork();
    if(pid < 0) return 0;
    if(pid == 0){
        while(1){
            Command command;
            memset(&command, 0, sizeof(command));
            command = recieve_command(sockfd);
            switch (command.index[0]){
            case '1':
                if(strcmp(command.value, "KILL") == 0){
                    printf("server: CONNECTION %d TERMINATED\n", token);

                    close(sockfd);
                    exit(1);
                }
                break;
            case '2':
                printf("client %d: get command 2:\n", token);
                send_file_text(sockfd, command.value, token);
                break;
            case '3':
                printf("client %d: get command 3:\n", token);
                send_directory_entries(sockfd, command.value, token);
                break;
            case '4':
                printf("client %d: get command 4:\n", token);
                download(sockfd, command.value, token);
                break;
            }
        }
        printf("server: CONNECTION %d TERMINATED\n", token);
        
        close(sockfd);
        exit (1);
    }
    return 0;
}

void synchronize(int sockfd){
    char done[3] = "*!\0";
    char check_done[3];
    bzero(check_done, 3);
    recv(sockfd, check_done, 3, 0);
    send(sockfd, done, 3, 0);
}
void send_file_text(int sockfd, const char *filename, int token){
    FILE *f = fopen(filename, "r");
    char empty[MAX_MESSAGE];
    memset(empty, 0, MAX_MESSAGE);
    empty[0] = '\n';
    if(f == NULL){
        char error[MAX_MESSAGE];
        memset(error, 0, MAX_MESSAGE);
        send(sockfd, error, MAX_MESSAGE, 0);
       
        printf("client %d: UNKOWN FILE: recieved file: %s\n", token, filename);
        fclose(f);
        return;
    }
    printf("client %d: EXISTING FILE: recieved file: %s\n", token, filename);

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if(size == 0){
        fclose(f);
        send(sockfd, empty, MAX_MESSAGE, 0);
        return;
    }

    char buffer[size];
    size_t res = fread(buffer, 1, size, f);
    if(res != size){
        printf("error reading file");
        fclose(f);
        return;
    }
    buffer[size] = '\0';
    
    send(sockfd, buffer, size, 0);
    fclose(f);
}
void send_directory_entries(int sockfd, char *dirname, int token){
    DIR *dir;

    dir = opendir(dirname);
    if(dir == NULL){
        char error[MAX_MESSAGE];
        memset(error, 0, MAX_MESSAGE);
        send(sockfd, error, MAX_MESSAGE, 0);
        printf("client %d: UNKOWN DIRECTORY: recieved directory: %s\n", token, dirname);
        return;
    }
    
    printf("client %d: EXISTING DIRECTORY: recieved directory: %s\n", token, dirname);
    
    struct dirent *entry;
    char buf[MAX_MESSAGE];
    bzero(buf, MAX_MESSAGE);

    while((entry = readdir(dir)) != NULL){
        if(entry->d_type == DT_DIR) strcat(buf, "d: ");
        if(entry->d_type == DT_REG) strcat(buf, "f: ");
        strncat(buf, entry->d_name, strlen(entry->d_name));
    }
    send(sockfd, buf, strlen(buf), 0);
    closedir(dir);
}

void download(int sockfd, char *name, int token){
    DIR *dir = opendir(name);
    if(dir != NULL){
        send(sockfd, "d", 1, 0);
        closedir(dir);
        download_directory(sockfd, name, token);
        return;
    }
    FILE *f = fopen(name, "r");
    if(f != NULL){
        send(sockfd, "f", 1, 0);
        fclose(f);
        send_file_text(sockfd, name, token);  
        return;
    } 
    printf("client %d: UNKOWN OBJECT: recieved object: %s\n", token, name);
    send(sockfd, "e", 1, 0);
    fclose(f);
}
void download_directory(int sockfd, char *dirname, int token){
    char error[3] = "e:\0";
    char end_of_dir[4] = "EOD\0";

    DIR *dir = opendir(dirname);
    if(dir != NULL) printf("client %d: EXISTING DIRECTORY: recieved directory: %s\n", token, dirname);

    struct dirent *entry;

    while((entry = readdir(dir)) != NULL){
       if(entry->d_type == DT_DIR && 
               strcmp(entry->d_name, "..") != 0 &&
               strcmp(entry->d_name, ".") != 0){
           char child_dir_name[50];

           strcpy(child_dir_name, "d:");
           strcat(child_dir_name, dirname);
           strcat(child_dir_name, "/");
           strcat(child_dir_name, entry->d_name);
           synchronize(sockfd);
           send(sockfd, child_dir_name, strlen(child_dir_name), 0);
           synchronize(sockfd);
           
           download_directory(sockfd, (child_dir_name + 2), token);
       }
        if(entry->d_type == DT_REG){
            char file_name[100];
            bzero(file_name, 100);
            strcpy(file_name, "f:");
            strcat(file_name, dirname);
            strcat(file_name, "/");
            strcat(file_name, entry->d_name);
           
            synchronize(sockfd);
            send(sockfd, file_name, strlen(file_name), 0);
            synchronize(sockfd);

            send_file_text(sockfd, (file_name + 2), token);
        }
    }

    synchronize(sockfd);
    send(sockfd, end_of_dir, strlen(end_of_dir), 0);
    synchronize(sockfd);

    closedir(dir);
}

Command recieve_command(int sockfd){
    Command error;
    error.index[0] = '1';
    error.index[1] = '\0';
    strcpy(error.value, "KILL");
    
    Command command;
    memset(&command, 0, sizeof(command));

    if(recv(sockfd, command.index, 1, 0) <= 0) return error;
    if(recv(sockfd, command.value, max_value_length - 1, 0) <= 0) return error;
    command.value[max_value_length] = '\0';
    command.index[1] = '\0';

    return command;
}
