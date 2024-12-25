#ifndef SE_LIB_H
#define SE_LIB_H

typedef struct {
    char index[1];
    char value[50];
} Command;

void error(const char *messge);

void s_view_file();
void s_view_directory();
void s_download();
void s_change_directory();

void wait_command(int sockfd);
Command recieve_command(int sockfd);

int send_file(int sockfd, char *filename);


#endif