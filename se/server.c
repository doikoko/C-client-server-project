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
#include <dirent.h>

#include "../lib/se_lib.h"

int active_connections = 0;
int connection_token = 0;

void child_handler(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
    active_connections--;
    printf("server: ACTIVE CONNECTIONS: %d\n", active_connections);
}

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

    printf("server is turned on\n\n");
 
    struct sigaction sa;
    sa.sa_handler = child_handler;
    sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;

    sigaction(SIGCHLD, &sa, NULL);    

    while(1){
        if(active_connections >= MAX_CONNECTIONS) continue;
        
        if(listen(sockfd, MAX_CONNECTIONS) < 0){
            error("listen error");
        } else {
            printf("server: ...WAITING CONNECTIONS...\n");
        }
        if((cl_sockfd = accept(sockfd, (struct sockaddr*)&addr, &socklen)) < 0){
            error("accept rejected");  
        } else {
            connection_token++;
            active_connections++;
            printf("server: NEW CONNECTION, TOKEN: %d\n", connection_token);
            printf("server: ACTIVE CONNECTIONS: %d\n", active_connections);
        }  
        command_handler(cl_sockfd, connection_token, active_connections);
    }
    return 0;
}
