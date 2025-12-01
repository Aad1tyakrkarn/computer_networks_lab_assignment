#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 2048

// Client structure
typedef struct {
    int socket;
    int id;
    char ip[20];
    int port;
    char name[50];
} Client;

// Global variables
Client *clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *log_file;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to get current timestamp
void get_timestamp(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 80, "[%Y-%m-%d %H:%M:%S]", t);
}

// Function to write to log file
void write_log(const char *message) {
    pthread_mutex_lock(&log_mutex);
    
    char timestamp[80];
    get_timestamp(timestamp);
    
    fprintf(log_file, "%s %s\n", timestamp, message);
    fflush(log_file);
    
    pthread_mutex_unlock(&log_mutex);
}

// Function to send message to all clients except sender
void broadcast_message(char *message, int sender_id) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < client_count; i++) {
        if (clients[i]->id != sender_id) {
            if (send(clients[i]->socket, message, strlen(message), 0) < 0) {
                perror("Send failed");
            }
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Function to send message to all clients including sender
void broadcast_to_all(char *message) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < client_count; i++) {
        if (send(clients[i]->socket, message, strlen(message), 0) < 0) {
            perror("Send failed");
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Function to remove client from array
void remove_client(int id) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < client_count; i++) {
        if (clients[i]->id == id) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Client handler thread function
void *client_handler(void *arg) {
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 100];
    int leave_flag = 0;
    
    Client *client = (Client *)arg;
    
    // Receive client name
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(client->socket, buffer, BUFFER_SIZE, 0) <= 0) {
        leave_flag = 1;
    } else {
        strcpy(client->name, buffer);
        
        // Announce new user
        sprintf(message, ">>> %s has joined the chat!\n", client->name);
        printf("%s", message);
        write_log(message);
        broadcast_to_all(message);
        
        // Send welcome message to new client
        sprintf(buffer, "Welcome to the chat, %s! Type your messages below.\n", client->name);
        send(client->socket, buffer, strlen(buffer), 0);
    }
    
    // Main message loop
    while (!leave_flag) {
        memset(buffer, 0, BUFFER_SIZE);
        int receive = recv(client->socket, buffer, BUFFER_SIZE, 0);
        
        if (receive > 0) {
            if (strlen(buffer) > 0) {
                // Format message with timestamp and username
                char timestamp[80];
                get_timestamp(timestamp);
                
                sprintf(message, "%s %s: %s", timestamp, client->name, buffer);
                
                // Display on server
                printf("%s", message);
                
                // Write to log
                write_log(message);
                
                // Broadcast to all other clients
                broadcast_message(message, client->id);
            }
        } else if (receive == 0 || strcmp(buffer, "exit\n") == 0) {
            leave_flag = 1;
        } else {
            perror("Receive error");
            leave_flag = 1;
        }
    }
    
    // Client disconnected
    sprintf(message, "<<< %s has left the chat.\n", client->name);
    printf("%s", message);
    write_log(message);
    broadcast_to_all(message);
    
    // Cleanup
    close(client->socket);
    remove_client(client->id);
    free(client);
    pthread_detach(pthread_self());
    
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t tid;
    
    // Open log file
    log_file = fopen("log.txt", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return EXIT_FAILURE;
    }
    
    char log_msg[200];
    sprintf(log_msg, "\n=== Chat Server Started ===");
    write_log(log_msg);
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    // Bind socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║       Multi-threaded Chat Server         ║\n");
    printf("╠═══════════════════════════════════════════╣\n");
    printf("║  Server started on port %d               ║\n", PORT);
    printf("║  Logging to: log.txt                      ║\n");
    printf("║  Waiting for clients...                   ║\n");
    printf("╚═══════════════════════════════════════════╝\n\n");
    
    // Accept clients
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }
        
        // Check if max clients reached
        if (client_count >= MAX_CLIENTS) {
            printf("Maximum clients reached. Connection rejected.\n");
            close(new_socket);
            continue;
        }
        
        // Create new client
        Client *client = (Client *)malloc(sizeof(Client));
        client->socket = new_socket;
        client->id = client_count;
        strcpy(client->ip, inet_ntoa(client_addr.sin_addr));
        client->port = ntohs(client_addr.sin_port);
        
        // Add client to array
        pthread_mutex_lock(&clients_mutex);
        clients[client_count++] = client;
        pthread_mutex_unlock(&clients_mutex);
        
        printf("New connection from %s:%d [Client ID: %d]\n", 
               client->ip, client->port, client->id);
        
        // Create thread for client
        if (pthread_create(&tid, NULL, client_handler, (void *)client) != 0) {
            perror("Thread creation failed");
            free(client);
            client_count--;
        }
    }
    
    fclose(log_file);
    close(server_fd);
    return 0;
}
