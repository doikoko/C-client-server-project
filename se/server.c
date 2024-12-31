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

#include "../lib/se_lib.h"

int main(int argc, char **argv){
    if(argc != 4){
        printf("------ARGUMENTS-LIST------\n1)IP\n2)PORT\n3)MAX CONNECTIONS\n--------------------------\n");
        return 1;
    }
    char *IP = argv[1];
    char *PORT = argv[2];
    int MAX_CONNECTIONS = atoi(argv[3]);

    int sockfd, cl_sockfd;
    struct sockaddr_in addr;
    socklen_t socklen;
    pid_t pid;  

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        error("socket error");
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(PORT));
    if((inet_pton(AF_INET, IP, &addr.sin_addr)) < 0){
        error("IP is not available");
    }

    socklen = sizeof(addr);

    if((bind(sockfd, (struct sockaddr*)&addr, socklen)) < 0){
        error("bind error");
    }

    if((pid = fork()) == -1){
        error("can't run server");
    } else if(pid != 0){
        printf("server is turned on");
    }
    int a;
    if(pid == 0){
        while(1){
            if(a = (listen(sockfd, MAX_CONNECTIONS)) < 0){
                error("listen error");
            } else {
                printf("...\n");
            }
            if((cl_sockfd = accept(sockfd, (struct sockaddr*)&addr, &socklen)) < 0){
                error("accept rejected");  
            } else {
                printf("\033[A");
                printf("\033[K");
                printf("connected\n.........\n%d\n", cl_sockfd);
            }  
           wait_command(cl_sockfd);
        }   
        return 0;
    } 
    if((waitpid(pid, NULL, 0)) == -1){
        error("process error");
    } else {
        printf("\n\ndone!\n");
        close(sockfd);
    }
    
    return 0;
}