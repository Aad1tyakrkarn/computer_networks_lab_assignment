#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char fruit_name[50];
    int quantity;
    
    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return -1;
    }
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }
    
    printf("Connected to Fruit Server!\n\n");
    
    // Receive stock information
    read(sock, buffer, BUFFER_SIZE);
    printf("%s\n", buffer);
    
    // Get user input
    printf("Enter fruit name: ");
    scanf("%s", fruit_name);
    printf("Enter quantity: ");
    scanf("%d", &quantity);
    
    // Send purchase request
    sprintf(buffer, "%s %d", fruit_name, quantity);
    send(sock, buffer, strlen(buffer), 0);
    
    // Receive response
    memset(buffer, 0, BUFFER_SIZE);
    read(sock, buffer, BUFFER_SIZE);
    printf("\nServer Response:\n%s\n", buffer);
    
    close(sock);
    return 0;
}
