#ifndef CL_LIB_H
#define CL_LIB_H

typedef struct {
    char keys[5][4];
    char values[5][11]; 
} Footer;
typedef struct {
    char index[2];
    char value[50];
} Command;

typedef int(*func)(int, char*, Footer, int, int);

void error(const char *messge);

int handle_user_input(int sockfd, int max_y, int max_x, Footer footer, char *header, char index, char *error, func function);
void print_footer(Footer footer, int max_y, int max_x);
char *get_name(int max_y, int max_x, char *header);
void divide(int max_x);
int quit_confirm(int max_y, int max_x);

void clear_screen_if_full(Footer footer, int max_y, int max_x);
void synchronize(int sockfd);

void send_command(int sockfd, Command command);

int view_file_text(int sockfd, char *name, Footer footer, int max_y, int max_x);
int view_directory(int sockfd, char *name, Footer footer, int max_y, int max_x);
int download(int sockfd, char *name, Footer footer, int max_y, int max_x);
int download_file(int sockfd, char *name, Footer footer, int max_y, int max_x);
int download_directory(int sockfd, char *name, Footer footer, int max_y, int max_x);
int change_directory(char *directory);

#endif
