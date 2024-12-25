#ifndef CL_LIB_H
#define CL_LIB_H

typedef struct {
    char keys[5][4];
    char values[5][11]; 
} Footer;

void error(const char *messge);


void print_footer(Footer footer, int max_y, int max_x);
int quit_confirm(int max_y, int max_x);

void send_command(int sockfd, char *command);

int recieve_file(int sockfd, char *filename);
int view_file(int sockfd);

#endif