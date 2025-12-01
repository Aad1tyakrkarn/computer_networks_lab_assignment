#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void send_file(int client_socket, char *filename) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    long file_size = 0;
    clock_t start, end;
    double time_taken;
    
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error: File '%s' not found\n", filename);
        strcpy(buffer, "FILE_NOT_FOUND");
        send(client_socket, buffer, strlen(buffer), 0);
        return;
    }
    
    // Get file size
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    
    // Send file size first
    sprintf(buffer, "%ld", file_size);
    send(client_socket, buffer, strlen(buffer), 0);
    
    // Wait for acknowledgment
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    
    printf("Sending file '%s' (%ld bytes)...\n", filename, file_size);
    
    // Start timer
    start = clock();
    
    // Send file data
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            perror("Send failed");
            break;
        }
    }
    
    // End timer
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    fclose(fp);
    
    printf("✓ File sent successfully!\n");
    printf("  Transfer time: %.6f seconds\n", time_taken);
    printf("  Transfer speed: %.2f KB/s\n\n", (file_size / 1024.0) / time_taken);
}

void receive_file(int client_socket, char *filename) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    long file_size;
    long received = 0;
    int bytes_read;
    clock_t start, end;
    double time_taken;
    
    // Receive file size
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    file_size = atol(buffer);
    
    if (file_size == 0) {
        printf("Error: Invalid file size\n");
        return;
    }
    
    // Send acknowledgment
    strcpy(buffer, "READY");
    send(client_socket, buffer, strlen(buffer), 0);
    
    printf("Receiving file '%s' (%ld bytes)...\n", filename, file_size);
    
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("File open error");
        return;
    }
    
    // Start timer
    start = clock();
    
    // Receive file data
    while (received < file_size) {
        bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) break;
        
        fwrite(buffer, 1, bytes_read, fp);
        received += bytes_read;
    }
    
    // End timer
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    fclose(fp);
    
    printf("✓ File received successfully!\n");
    printf("  Transfer time: %.6f seconds\n", time_taken);
    printf("  Transfer speed: %.2f KB/s\n\n", (file_size / 1024.0) / time_taken);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    char command[10];
    char filename[256];
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║    File Transfer Server - Assignment 9   ║\n");
    printf("╠═══════════════════════════════════════════╣\n");
    printf("║  Server started on port %d               ║\n", PORT);
    printf("║  Waiting for client...                    ║\n");
    printf("╚═══════════════════════════════════════════╝\n\n");
    
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("Client connected!\n\n");
        
        // Receive command (DOWNLOAD or UPLOAD)
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        sscanf(buffer, "%s %s", command, filename);
        
        printf("Command: %s, File: %s\n", command, filename);
        
        if (strcmp(command, "DOWNLOAD") == 0) {
            // Client wants to download file from server
            send_file(client_socket, filename);
        } else if (strcmp(command, "UPLOAD") == 0) {
            // Client wants to upload file to server
            receive_file(client_socket, filename);
        }
        
        close(client_socket);
        printf("Client disconnected.\n");
        printf("Waiting for next client...\n\n");
    }
    
    close(server_fd);
    return 0;
}
