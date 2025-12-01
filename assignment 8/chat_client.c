#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 2048

int sock = 0;
char name[50];

// Thread to receive messages from server
void *receive_handler(void *arg) {
    char message[BUFFER_SIZE];
    
    while (1) {
        memset(message, 0, BUFFER_SIZE);
        int receive = recv(sock, message, BUFFER_SIZE, 0);
        
        if (receive > 0) {
            printf("%s", message);
            fflush(stdout);
        } else if (receive == 0) {
            printf("\n[Server disconnected]\n");
            break;
        } else {
            perror("Receive error");
            break;
        }
    }
    
    return NULL;
}

// Thread to send messages to server
void *send_handler(void *arg) {
    char message[BUFFER_SIZE];
    
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        
        if (strcmp(message, "exit\n") == 0) {
            send(sock, message, strlen(message), 0);
            printf("Disconnecting...\n");
            exit(0);
        }
        
        send(sock, message, strlen(message), 0);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr;
    pthread_t recv_thread, send_thread;
    
    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return -1;
    }
    
    // Get username
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║          Chat Client - Assignment 8      ║\n");
    printf("╚═══════════════════════════════════════════╝\n\n");
    
    printf("Enter your name: ");
    fgets(name, 50, stdin);
    name[strcspn(name, "\n")] = 0;  // Remove newline
    
    if (strlen(name) == 0) {
        strcpy(name, "Anonymous");
    }
    
    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Convert IP address
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }
    
    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }
    
    printf("Connected to server!\n\n");
    
    // Send name to server
    send(sock, name, strlen(name), 0);
    
    // Create threads for sending and receiving
    pthread_create(&recv_thread, NULL, receive_handler, NULL);
    pthread_create(&send_thread, NULL, send_handler, NULL);
    
    // Wait for threads
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);
    
    close(sock);
    return 0;
}
