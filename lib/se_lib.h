#ifndef SE_LIB_H
#define SE_LIB_H

typedef struct {
    char index[2];
    char value[50];
} Command;

void error(const char *messge);

void send_file_text(int sockfd, const char *filename);
void view_directory();
void download();
void change_directory();

int wait_signal(int sockfd, char response);
void wait_command(int sockfd);
Command recieve_command(int sockfd);


#endif