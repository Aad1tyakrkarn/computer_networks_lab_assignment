// gcc tcp_client.c -o tcp_client
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
    int sockfd;
    struct sockaddr_in serv;
    char buf[1024];
    if(argc < 3){ printf("Usage: %s <server-ip> <port>\n", argv[0]); return 1; }
    char *server = argv[1];
    int port = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){ perror("socket"); return 1; }

    memset(&serv,0,sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, server, &serv.sin_addr);

    if(connect(sockfd,(struct sockaddr*)&serv,sizeof(serv))<0){ perror("connect"); return 1; }

    send(sockfd, "Hi", 2, 0);
    int n = recv(sockfd, buf, sizeof(buf)-1, 0);
    buf[n] = 0;
    printf("Server response: %s\n", buf);
    close(sockfd);
    return 0;
}
