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
#define MAX_FRUITS 10
#define MAX_CUSTOMERS 50

typedef struct {
    char name[50];
    int quantity;
    time_t last_sold;
} Fruit;

typedef struct {
    char ip[20];
    int port;
} Customer;

Fruit fruits[MAX_FRUITS];
Customer customers[MAX_CUSTOMERS];
int fruit_count = 0;
int customer_count = 0;

void init_fruits() {
    strcpy(fruits[0].name, "Apple");
    fruits[0].quantity = 100;
    fruits[0].last_sold = time(NULL);
    
    strcpy(fruits[1].name, "Banana");
    fruits[1].quantity = 150;
    fruits[1].last_sold = time(NULL);
    
    strcpy(fruits[2].name, "Orange");
    fruits[2].quantity = 80;
    fruits[2].last_sold = time(NULL);
    
    strcpy(fruits[3].name, "Mango");
    fruits[3].quantity = 60;
    fruits[3].last_sold = time(NULL);
    
    fruit_count = 4;
}

int find_customer(char *ip, int port) {
    for (int i = 0; i < customer_count; i++) {
        if (strcmp(customers[i].ip, ip) == 0 && customers[i].port == port) {
            return 1;
        }
    }
    return 0;
}

void add_customer(char *ip, int port) {
    if (!find_customer(ip, port)) {
        strcpy(customers[customer_count].ip, ip);
        customers[customer_count].port = port;
        customer_count++;
    }
}

void display_customers() {
    printf("\n=== Customer List ===\n");
    for (int i = 0; i < customer_count; i++) {
        printf("%d. IP: %s, Port: %d\n", i+1, customers[i].ip, customers[i].port);
    }
    printf("Total unique customers: %d\n\n", customer_count);
}

void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    char fruit_name[50];
    int requested_qty;
    
    char *client_ip = inet_ntoa(client_addr.sin_addr);
    int client_port = ntohs(client_addr.sin_port);
    
    printf("Client connected: %s:%d\n", client_ip, client_port);
    add_customer(client_ip, client_port);
    display_customers();
    
    // Send stock info
    sprintf(response, "=== Fruit Stock ===\n");
    for (int i = 0; i < fruit_count; i++) {
        sprintf(response + strlen(response), "%d. %s: %d units\n", 
                i+1, fruits[i].name, fruits[i].quantity);
    }
    sprintf(response + strlen(response), "\nTotal unique customers: %d\n", customer_count);
    send(client_socket, response, strlen(response), 0);
    
    // Receive purchase request
    memset(buffer, 0, BUFFER_SIZE);
    int valread = read(client_socket, buffer, BUFFER_SIZE);
    if (valread <= 0) {
        close(client_socket);
        return;
    }
    
    sscanf(buffer, "%s %d", fruit_name, &requested_qty);
    printf("Request: %s - %d units\n", fruit_name, requested_qty);
    
    // Process purchase
    int found = 0;
    for (int i = 0; i < fruit_count; i++) {
        if (strcasecmp(fruits[i].name, fruit_name) == 0) {
            found = 1;
            if (fruits[i].quantity >= requested_qty) {
                fruits[i].quantity -= requested_qty;
                fruits[i].last_sold = time(NULL);
                sprintf(response, "SUCCESS: Purchased %d %s(s). Remaining: %d\n", 
                        requested_qty, fruit_name, fruits[i].quantity);
                printf("Transaction successful!\n");
            } else {
                sprintf(response, "REGRET: Only %d %s(s) available. Cannot fulfill order of %d units.\n",
                        fruits[i].quantity, fruit_name, requested_qty);
                printf("Transaction failed - insufficient stock\n");
            }
            break;
        }
    }
    
    if (!found) {
        sprintf(response, "REGRET: Fruit '%s' not available in store.\n", fruit_name);
    }
    
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    init_fruits();
    
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
    
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Fruit Server started on port %d\n", PORT);
    printf("Waiting for clients...\n\n");
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }
        
        handle_client(new_socket, client_addr);
    }
    
    close(server_fd);
    return 0;
}
