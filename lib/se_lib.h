#ifndef SE_LIB_H
#define SE_LIB_H

typedef struct {
    char index[2];
    char value[50];
} Command;

void error(const char *messge);

void send_file_text(int sockfd, const char *filename, int token);
void send_directory_entries(int sockfd, char *dirname, int token);
void download(int sockfd, char *name, int token);
void download_directory(int sockfd, char *dirname, int token);


void change_directory();

int command_handler(int sockfd, int token, int connections);
Command recieve_command(int sockfd);


#endif