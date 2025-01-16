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
#include <dirent.h>

#include "se_lib.h"

#define MAX_MESSAGE 1024

const int max_value_length = 50;

void error(const char *messge){
    perror(messge);
    exit(1);
}

int command_handler(int sockfd, int token, int connections){
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
            case '5':
                change_directory();
                break;
            }
        }
        printf("server: CONNECTION %d TERMINATED\n", token);
        
        close(sockfd);
        exit (1);
    }
    return 0;
}

void send_file_text(int sockfd, const char *filename, int token){
    FILE *f = fopen(filename, "r");

    if(f == NULL){
        char error[MAX_MESSAGE];
        memset(error, 0, MAX_MESSAGE);
        send(sockfd, error, MAX_MESSAGE, 0);
        printf("client %d: UNKOWN FILE: recieved file: %s\n", token, filename);
        return;
    }
    printf("client %d: EXISTING FILE: recieved file: %s\n", token, filename);

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if(size == 0){
        fclose(f);
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
    FILE *f = fopen(name, "r");
    DIR *dir = opendir(name);
    if(f != NULL){
        send(sockfd, "f", 1, 0);
        send_file_text(sockfd, name, token);  
        fclose(f);      
    } else if(dir != NULL){
        send(sockfd, "d", 1, 0);
        printf("client %d: EXISTING DIRECTORY: recieved directory: %s\n", token, name);
        download_directory(sockfd, name, token);
        closedir(dir);
    } else {
        send(sockfd, "e", 1, 0);
        printf("client %d: UNKOWN OBJECT: recieved object: %s\n", token, name);
    }
}
void download_directory(int sockfd, char *dirname, int token){
    char error[3] = "e:\0";
    char end_of_dir[4] = "EOD\0";
    
    DIR *dir = opendir(dirname);
    
    if(dir == NULL){
        printf("client %d: UNKOWN OBJECT: recieved object: %s\n", token, dirname);
        send(sockfd, error, 1, 0);
        return;
    } else {
        printf("client %d: EXISTING DIRECTORY: recieved directory: %s\n", token, dirname);
    }
    struct dirent *entry;
    char path[MAX_MESSAGE];
    bzero(path, MAX_MESSAGE);

    while((entry = readdir(dir)) != NULL){
        if(entry->d_type == DT_DIR){
            strcpy(path, "d:");
            strcat(path, entry->d_name);
            send(sockfd, path, strlen(path), 0);
            
            char d_name[100];
            bzero(d_name, 100);
            char *pos = strchr(path, ':'); 
            strcpy(d_name, pos + 1);
            strcat(d_name, "/");
            download_directory(sockfd, d_name, token);
        }
        if(entry->d_type == DT_REG){
            strcpy(path, "f:");
            strcat(path, entry->d_name);
            send(sockfd, path, strlen(path), 0);
            char f_name[100];
            bzero(f_name, 100);
            char *pos = strchr(path, ':'); 
            strcpy(f_name, pos + 1);

            send_file_text(sockfd, f_name, token);
        }
    }

    send(sockfd, end_of_dir, strlen(end_of_dir), 0);

}

void change_directory(){
    return;
}
Command recieve_command(int sockfd){
    Command error;
    error.index[0] = '1';
    error.index[1] = '\0';
    strcpy(error.value, "KILL");
    
    Command command;
    memset(&command, 0, sizeof(command));

    if(recv(sockfd, &command.index[0], 1, 0) <= 0) return error;
    if(recv(sockfd, command.value, max_value_length - 1, 0) <= 0) return error;
    command.value[max_value_length] = '\0';
    command.index[1] = '\0';

    return command;
}