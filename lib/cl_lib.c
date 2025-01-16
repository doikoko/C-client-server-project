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
#include <sys/stat.h>
#include <dirent.h>

#include "cl_lib.h"

#define MAX_MESSAGE 1024
#define KEY_ESC 27

void error(const char *messge){
    perror(messge);
    exit(1);
}

int handle_user_input(int sockfd, int max_y, int max_x, Footer footer, char *header, char index, char *error, func function){
    char *name = (char *)malloc(50);
    strncpy(name, get_name(max_y, max_x, header), 50);
    
    if(strcmp(name, "ESCAPE") == 0){
        wclear(stdscr);
        print_footer(footer, max_y, max_x);
        wprintw(stdscr, "canseled\n");
        divide(max_x);
        refresh();
        return 0;
    }
    print_footer(footer, max_y, max_x);
    
    Command command;
    command.index[0] = index;
    command.index[1] = '\0';
    strcpy(command.value, name);

    send_command(sockfd, command);
    wclear(stdscr);
    if(!function(sockfd, name)){
        print_footer(footer, max_y, max_x);
        wprintw(stdscr, "%s\n", error);
        divide(max_x);
        refresh();
        return 0;
    } else {
        divide(max_x);
        print_footer(footer, max_y, max_x);
        free(name);
    }
}
void print_footer(Footer footer, int max_y, int max_x){
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    move(max_y - 1, max_x);
    clrtoeol();
    for(int i = 0; i < 5; i++){
        move(max_y - 1, max_x / 6 * (i + 1) - 5);
    
        wprintw(stdscr, "%s ", footer.keys[i]);

        if(has_colors){
            attron(COLOR_PAIR(1));
            wprintw(stdscr, "%s", footer.values[i]);
            attroff(COLOR_PAIR(1));
        } else {
            wprintw(stdscr, "%s", footer.values[i]);
        }
    }
    move(0, 0);
    refresh();
}
char *get_name(int max_y, int max_x, char *header){
    move(max_y - 1, 0);
    clrtoeol();
    if(has_colors) attron(COLOR_PAIR(1));
    
    move(max_y - 1, max_x / 2 - 10);
    wprintw(stdscr, "%s (BACK: ESC)\t", header);
    
    nodelay(stdscr, FALSE);

    char *message = (char *)malloc(50);
    bzero(message, 50);
    int input;
    int index = 0;

    while(1){
        input = getch();
        if(input == KEY_ESC){
            free(message);
            move(max_y - 1, max_x / 2 - 10);
            clrtoeol();
            if(has_colors) attroff(COLOR_PAIR(1));
            move(0, 0);
            refresh();
            
            return "ESCAPE";
        } else if(input == KEY_ENTER || input == '\n'){
            if(has_colors) attroff(COLOR_PAIR(1));
            
            message[index] = '\0';
            
            move(max_y - 1, max_x / 2 - 10);
            clrtoeol();
            if(has_colors) attroff(COLOR_PAIR(1));
            move(0, 0);
            refresh();
            
            if(index == 0) return "ESCAPE";
            return message;
        } else if((input == KEY_BACKSPACE || input == 127) && index > 0){
            index--;
            message[index] = '\0';
            move(max_y - 1, max_x / 2 - 10);
            clrtoeol();
            wprintw(stdscr, "%s (BACK: ESC)\t%s", header, message);
            refresh();

            continue; 
        }
        wprintw(stdscr, "%c", input);
        refresh();
        
        message[index] = (char)input;
        index++;
    }
}
void divide(int max_x){
    for(int i = 0; i < max_x; i++){
        wprintw(stdscr, ".");
    }
    wprintw(stdscr, "\n");
    refresh();
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
    wclear(stdscr);
    return status;
}


void send_command(int sockfd, Command command) {
    send(sockfd, &command.index[0], 1, 0);
    send(sockfd, command.value, strlen(command.value), 0);
}

int view_file_text(int sockfd, char *name){
    int bytes_recieved;
    char buffer[MAX_MESSAGE];
    char error[MAX_MESSAGE];
    bzero(buffer, MAX_MESSAGE);
    bzero(error, MAX_MESSAGE);
    
    if((bytes_recieved = recv(sockfd, buffer, MAX_MESSAGE, 0)) <= 0)
        return 0;
    if(strcmp(error, buffer) == 0) return 0;
    
    buffer[bytes_recieved] = '\0';

    wprintw(stdscr, "%s\n", buffer);
    refresh();

    return 1;
}
int view_directory(int sockfd, char *name){
    int bytes_recieved;
    char buffer[MAX_MESSAGE];
    char error[MAX_MESSAGE];
    bzero(buffer, MAX_MESSAGE);
    bzero(error, MAX_MESSAGE);

    int x = 0, y = 0, max_row = 0, max_y = getmaxy(stdscr);
    
    if((bytes_recieved = recv(sockfd, buffer, MAX_MESSAGE, 0)) <= 0)
        return 0;
    if(strcmp(error, buffer) == 0) return 0;
    buffer[bytes_recieved] = '\0';

    for(int i = 0; i < strlen(buffer); i++){
        start_color();
        
        if(buffer[i + 1] == ':'){
            move(y, x);
            y++;
        }
        if(buffer[i] == 'd' && buffer[i + 1] == ':'){
            init_pair(2, COLOR_BLUE, COLOR_BLACK);
            attron(COLOR_PAIR(2));  
        }
        if(buffer[i] == 'f' && buffer[i + 1] == ':'){
            attroff(COLOR_PAIR(2));  
        }

        if((max_y - y) <= 6){
            max_row = max_y - y;
            x += 25;
            y = 0;
        }
        wprintw(stdscr, "%c", buffer[i]);
        refresh();
    }
    if(max_row != 0){
        move((max_y - 5), 0);
    }
    else{
        move(y, 0);
    }
    attroff(COLOR_PAIR(2));  
    wprintw(stdscr, "\n");
    refresh();
    return 1;
}
int download(int sockfd, char *name){
    char error[3] = "e:\0";
    char type;

    char new_name[25];
    char *pos;
    pos = strrchr(name, '/');
    if(pos != NULL){
        strcpy(new_name, pos + 1);
    } 
    
    if(recv(sockfd, &type, 1, 0) <= 0) return 0;
    if(type == 'e'){
        wprintw(stdscr, "no such file or directory");
        refresh();
        return 0;
    } 
    else if(type == 'f'){
        if(!download_file(sockfd, new_name)) return 0;
    }
    else if(type == 'd'){
        if(!download_directory(sockfd, new_name)) return 0;
    }
    return 1;
}
int download_file(int sockfd, char *name){
    FILE *f = fopen(name, "w");

    char buf[MAX_MESSAGE];
    char error[MAX_MESSAGE];
    bzero(buf, MAX_MESSAGE);
    bzero(error, MAX_MESSAGE);
    
    if(recv(sockfd, buf, MAX_MESSAGE, 0) <= 0 ) return 0;
    if(strcmp(buf, error) == 0) return 0;
    fprintf(f, "%s", buf);
    fclose(f);

    wprintw(stdscr, "success, downloaded file: %s\n", name);
    refresh();
    return 1;
}
int download_directory(int sockfd, char *name){
    char error[3] = "e:\0";
    char end_of_dir[4] = "EOD\0";

    if(mkdir(name, 0777) != 0) return 0;
    DIR *dir = opendir(name);

    char buf[50];
    char path[100];
    char child_name[50];
    bzero(path, 100);
    bzero(child_name, 50);
    strcpy(path, name);    

    while(strcmp(buf, end_of_dir) != 0){
        bzero(buf, 50);
        bzero(child_name, 50);
        
        if(recv(sockfd, buf, 50, 0) <= 0) return 0;
        

        if(buf[0] == 'd' && buf[1] == ':'){
            char *pos = strchr(buf, ':'); 
            strcpy(child_name, pos + 1);
            strcat(path, "/");
            strcat(path, child_name);
            download_directory(sockfd, path);
        }
        if(buf[0] == 'f' && buf[1] == ':'){
            char *pos = strchr(buf, ':'); 
            strcpy(child_name, pos + 1);
            char file_name[100];
            strcpy(file_name, path);
            strcat(file_name, name);

            if(!download_file(sockfd, file_name)) return 0;
        }
    }

    return 1;
    
}
