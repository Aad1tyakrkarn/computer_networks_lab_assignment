// gcc tcp_server.c -o tcp_server
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
    int sockfd, newsock;
    struct sockaddr_in serv, cli;
    socklen_t clilen = sizeof(cli);
    char buf[1024];
    int port = (argc>1)?atoi(argv[1]):5000;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){ perror("socket"); return 1; }

    memset(&serv,0,sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(port);

    if(bind(sockfd,(struct sockaddr*)&serv,sizeof(serv))<0){ perror("bind"); return 1; }
    listen(sockfd,5);
    printf("Server listening on %d\n", port);

    newsock = accept(sockfd,(struct sockaddr*)&cli,&clilen);
    if(newsock<0){ perror("accept"); return 1; }

    recv(newsock, buf, sizeof(buf)-1, 0);
    printf("Received: %s\n", buf);

    send(newsock, "Hello", 5, 0);
    close(newsock);
    close(sockfd);
    return 0;
}
