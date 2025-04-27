#define _GNU_SOURCE

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
#include <sys/stat.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ftw.h> 

#include "cl_lib.h"

#define MAX_MESSAGE 1024
#define KEY_ESC 27

char current_directory[50];

void error(const char *messge){
    perror(messge);
    exit(1);
}

int nftw_remove(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    if(typeflag == FTW_DP){
        return rmdir(fpath);
    } else {
        return remove(fpath);
    }
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
    if(!function(sockfd, name, footer, max_y, max_x)){
        print_footer(footer, max_y, max_x);
        wprintw(stdscr, "%s\n", error);
        divide(max_x);
        refresh();
        free(name);
        return 0;
    } else {
        int x = 0, y = 0;
        getyx(stdscr, y, x);
        if(y > (max_y - 3)) move(max_y - 3, 0);
        divide(max_x);
        print_footer(footer, max_y, max_x);
        free(name);
    }
    return 1;
}
void print_footer(Footer footer, int max_y, int max_x){
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    move(max_y - 1, 0);
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
void clear_screen_if_full(Footer footer, int max_y, int max_x){
    int y, x;
    getyx(stdscr, y, x);
    
    if(y > (max_y - 4)) {
        for(int i = 0; i < max_y - 3; i++){
            move(i, 0);
            clrtoeol();
        }
        move(0, 0);
    }
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

int view_file_text(int sockfd, char *name, Footer footer, int max_y, int max_x){
    int bytes_recieved;
    char buffer[MAX_MESSAGE];
    char err[MAX_MESSAGE];
    char empty[MAX_MESSAGE];
    bzero(buffer, MAX_MESSAGE);
    bzero(err, MAX_MESSAGE);
    bzero(empty, MAX_MESSAGE);
    empty[0] = '\n';

    if((bytes_recieved = recv(sockfd, buffer, MAX_MESSAGE, 0)) <= 0)
        return 0;
    if(strcmp(err, buffer) == 0) return 0;
    if(strcmp(empty, buffer) == 0){
        clear_screen_if_full(footer, max_y, max_x);
        wprintw(stdscr, "file is empty\n");
        refresh();
        return 1;
    }

    buffer[bytes_recieved] = '\0';

    wprintw(stdscr, "%s\n", buffer);
    refresh();

    return 1;
}
void synchronize(int sockfd){
    char done[3] = "*!\0";
    char check_done[3];
    bzero(check_done, 3);
    send(sockfd, done, 3, 0);
    recv(sockfd, check_done, 3, 0);
}
int view_directory(int sockfd, char *name, Footer footer, int max_y, int max_x){
    int bytes_recieved;
    char buffer[MAX_MESSAGE];
    char error[MAX_MESSAGE];
    bzero(buffer, MAX_MESSAGE);
    bzero(error, MAX_MESSAGE);

    int x = 0, y = 0, max_row = 0;
    
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
    clear_screen_if_full(footer, max_y, max_x);
    wprintw(stdscr, "\n");
    refresh();
    return 1;
}
int download(int sockfd, char *name, Footer footer, int max_y, int max_x){
    char error[3] = "e:\0";
    char type;
    
    char new_name[50];
    char *pos;
    pos = strrchr(name, '/');
    
    strcpy(new_name, current_directory);
    if(strlen(new_name) != 0){
        if(new_name[strlen(new_name) - 1] != '/')
            strcat(new_name, "/");
    }
    if(pos != NULL){
        strcat(new_name, pos + 1);
    } else {
        strcat(new_name, name);
    }
    if(recv(sockfd, &type, 1, 0) <= 0) return 0;
    if(type == 'e'){
        clear_screen_if_full(footer, max_y, max_x);
        wprintw(stdscr, "no such file or directory");
        refresh();
        return 0;
    }
    else if(type == 'f'){
        if(!download_file(sockfd, new_name, footer, max_y, max_x)) return 0;
    }
    else if(type == 'd'){
        if(!download_directory(sockfd, new_name, footer, max_y, max_x)) return 0;
    }
    return 1;
}

int download_file(int sockfd, char *name, Footer footer, int max_y, int max_x){
    FILE *f = fopen(name, "w");
    char buf[MAX_MESSAGE];
    char error[MAX_MESSAGE];
    bzero(buf, MAX_MESSAGE);
    bzero(error, MAX_MESSAGE);
    
    if(recv(sockfd, buf, MAX_MESSAGE, 0) <= 0 ) return 0;
    if(strcmp(buf, error) == 0) return 0;
    fprintf(f, "%s", buf);

    clear_screen_if_full(footer, max_y, max_x);
    wprintw(stdscr, "success, downloaded file: %s\n", name);
    refresh();
    fclose(f);
    return 1;
}
int download_directory(int sockfd, char *name, Footer footer, int max_y, int max_x){
    char error[3] = "e:\0";
    char end_of_dir[4] = "EOD\0";

    DIR *dir = opendir(name);
    if(dir != NULL){
        nftw(name, nftw_remove, 100, FTW_DEPTH | FTW_PHYS);
    }
    if(mkdir(name, S_IRWXU | S_IROTH | S_IWOTH | S_IXOTH) != 0) return 0;
    dir = opendir(name);

    char buf[50];
    bzero(buf, 50);
    char new_name[50];
   while(1){
        bzero(buf, 50);

        synchronize(sockfd);
        if(recv(sockfd, buf, 50, 0) <= 0) return 0;
        synchronize(sockfd);
        
        if(strncmp(buf, end_of_dir, (size_t)4) == 0) return 1;
        if(buf[0] == 'd' && buf[1] == ':'){
            if(strlen(current_directory) != 0){
                strcpy(new_name, current_directory);
                if(new_name[strlen(new_name) - 1] != '/') strcat(new_name, "/");
                strcat(new_name, (buf + 2));
                if(mkdir(new_name, S_IRWXU | S_IROTH | S_IWOTH | S_IXOTH) != 0) return 0;
            }
            else if(mkdir((buf + 2), S_IRWXU | S_IROTH | S_IWOTH | S_IXOTH) != 0) return 0;
            clear_screen_if_full(footer, max_y, max_x);
            wprintw(stdscr, "success, creating directory %s\n", (buf + 2));
            refresh();
        }
        if(buf[0] == 'f' && buf[1] == ':'){
            if(strlen(current_directory) != 0){
                strcpy(new_name, current_directory);
                if(new_name[strlen(new_name) - 1] != '/') strcat(new_name, "/");
                strcat(new_name, (buf + 2));
                if(!download_file(sockfd, new_name, footer, max_y, max_x)) return 0;
            }
            else if(!download_file(sockfd, (buf + 2), footer, max_y, max_x)) return 0;
        }
   }
   closedir(dir);
} 
int change_directory(char *directory){
    char check_directory[50];
    if(strlen(current_directory) == 0){
        strcpy(check_directory, directory);
    }
    else{
        strcpy(check_directory, current_directory);
        if(check_directory[strlen(check_directory) - 1] != '/')
            strcat(check_directory, "/");
        strcat(check_directory, directory);
    }
    DIR *d = opendir(check_directory);
    if(d == NULL) {
        closedir(d);
        return 0;
    }
    closedir(d);

    strcpy(current_directory, check_directory);

    return 1;
}
