//echo_server.c
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "util.h"

#define SERV_PORT   6000
#define MAXLINE     512

void echo(int fd) {
    while(1) {
        char line[MAXLINE];
        int n = readline(fd, line, MAXLINE);
        CHKBOOLQUIT(n >= 0, "readline error");

        if(n == 0)  //connection terminated
            return;
        else
            CHKBOOLQUIT(writen(fd, line, n) == n, "writen error");
    }
}

int main(int argc, char **argv) {
    int sfd;
    struct sockaddr_in saddr;

    CHKBOOLQUIT( (sfd = socket(AF_INET, SOCK_STREAM, 0)) >= 0, "socket failed" );
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(SERV_PORT);
    CHKBOOLQUIT( bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr)) >= 0, "bind failed");
    CHKBOOLQUIT( listen(sfd, 1024) >= 0, "listen failed" );

    while(1) {
        struct sockaddr_in caddr;
        int cfd, clen = sizeof(caddr);
        CHKBOOLQUIT( (cfd = accept(sfd, (struct sockaddr*) &caddr, &clen)) >= 0,
                                                                       "accept failed");
        if(fork() == 0) {   //child
            close(sfd);
            printf("pid: %d, client: %s:%d\n", getpid(),
                                            inet_ntoa(caddr.sin_addr), caddr.sin_port);
            echo(cfd);
            printf("pid: %d done\n", getpid());
            exit(0);
        }
        else 
            close(cfd);
    }
    return 0;
}

