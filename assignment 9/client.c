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

void download_file(int sock, char *filename) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    long file_size;
    long received = 0;
    int bytes_read;
    clock_t start, end;
    double time_taken;
    
    // Send download command
    sprintf(buffer, "DOWNLOAD %s", filename);
    send(sock, buffer, strlen(buffer), 0);
    
    // Receive file size
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    
    if (strcmp(buffer, "FILE_NOT_FOUND") == 0) {
        printf("Error: File not found on server\n");
        return;
    }
    
    file_size = atol(buffer);
    
    // Send acknowledgment
    strcpy(buffer, "READY");
    send(sock, buffer, strlen(buffer), 0);
    
    printf("\n--- Downloading File ---\n");
    printf("File: %s\n", filename);
    printf("Size: %ld bytes\n", file_size);
    
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("File open error");
        return;
    }
    
    // Start timer
    start = clock();
    
    // Receive file data
    while (received < file_size) {
        bytes_read = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) break;
        
        fwrite(buffer, 1, bytes_read, fp);
        received += bytes_read;
    }
    
    // End timer
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    fclose(fp);
    
    printf("✓ Download complete!\n");
    printf("  Received: %ld bytes\n", received);
    printf("  Transfer time: %.6f seconds\n", time_taken);
    printf("  Transfer speed: %.2f KB/s\n", (file_size / 1024.0) / time_taken);
}

void upload_file(int sock, char *filename) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    long file_size = 0;
    clock_t start, end;
    double time_taken;
    
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error: File '%s' not found\n", filename);
        return;
    }
    
    // Get file size
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    
    // Send upload command
    sprintf(buffer, "UPLOAD %s", filename);
    send(sock, buffer, strlen(buffer), 0);
    
    // Send file size
    sprintf(buffer, "%ld", file_size);
    send(sock, buffer, strlen(buffer), 0);
    
    // Wait for acknowledgment
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    
    printf("\n--- Uploading File ---\n");
    printf("File: %s\n", filename);
    printf("Size: %ld bytes\n", file_size);
    
    // Start timer
    start = clock();
    
    // Send file data
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        if (send(sock, buffer, bytes_read, 0) < 0) {
            perror("Send failed");
            break;
        }
    }
    
    // End timer
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    fclose(fp);
    
    printf("✓ Upload complete!\n");
    printf("  Sent: %ld bytes\n", file_size);
    printf("  Transfer time: %.6f seconds\n", time_taken);
    printf("  Transfer speed: %.2f KB/s\n", (file_size / 1024.0) / time_taken);
}

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return -1;
    }
    
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║    File Transfer Client - Assignment 9   ║\n");
    printf("╚═══════════════════════════════════════════╝\n");
    
    // Download operation
    printf("\n=== OPERATION 1: DOWNLOAD FILE FROM SERVER ===\n");
    
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
    
    download_file(sock, "server_file.txt");
    close(sock);
    
    sleep(2);  // Wait between operations
    
    // Upload operation
    printf("\n=== OPERATION 2: UPLOAD FILE TO SERVER ===\n");
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }
    
    upload_file(sock, "client_file.txt");
    close(sock);
    
    printf("\n╔═══════════════════════════════════════════╗\n");
    printf("║        File Transfer Complete!            ║\n");
    printf("╚═══════════════════════════════════════════╝\n");
    
    return 0;
}
