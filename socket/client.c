//echo_client.c
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "util.h"

#define SERV_ADDR   "127.0.0.1"
#define SERV_PORT   6000
#define MAXLINE     512

void copy(int sfd) {
    char sline[MAXLINE], rline[MAXLINE];
    while(fgets(sline, MAXLINE, stdin) != NULL) {
        int n = strlen(sline);
        CHKBOOLQUIT( writen(sfd, sline, n) == n, "writen error" );

        CHKBOOLQUIT( (n = readline(sfd, rline, MAXLINE)) >= 0, "readline error" );
        rline[n] = 0;
        fputs(rline, stdout);
    }
    CHKBOOLQUIT( ferror(stdin) == 0, "cannot read file" );
}

int main(int argc, char **argv) {
    int sfd;
    struct sockaddr_in saddr;

    CHKBOOLQUIT( (sfd = socket(AF_INET, SOCK_STREAM, 0)) >= 0,
                 "socket failed" );

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(SERV_ADDR);
    saddr.sin_port = htons(SERV_PORT);

    CHKBOOLQUIT( connect(sfd, (struct sockaddr*)&saddr, sizeof(saddr)) >= 0,
                 "connectfailed" );
    printf("server: %s:%d\n", inet_ntoa(saddr.sin_addr), saddr.sin_port);

    copy(sfd);

    close(sfd);
    return 0;
}
