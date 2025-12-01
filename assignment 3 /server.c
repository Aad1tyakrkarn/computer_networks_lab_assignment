#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#define MAX_FRUITS 5
#define MAX_CLIENTS 100

// Structure to store fruit record
struct Fruit {
    char name[50];
    int quantity;
    char lastSold[100];
};

// Structure to store unique clients
struct Client {
    char ip[INET_ADDRSTRLEN];
    int port;
};

int main() {
    int welcomeSocket, newSocket;
    char buffer[1024];
    struct sockaddr_in serverAddr, serverStorage;
    socklen_t addr_size;

    // Initialize fruit records
    struct Fruit fruits[MAX_FRUITS] = {
        {"apple", 10, "N/A"},
        {"banana", 15, "N/A"},
        {"mango", 5, "N/A"},
        {"orange", 20, "N/A"},
        {"grape", 8, "N/A"}
    };

    struct Client clients[MAX_CLIENTS];
    int clientCount = 0;

    welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("10.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    bind(welcomeSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (listen(welcomeSocket, 5) == 0)
        printf("Server listening on port 7891...\n");
    else
        printf("Error in listen()\n");

    while (1) {
        addr_size = sizeof(serverStorage);
        newSocket = accept(welcomeSocket, (struct sockaddr *)&serverStorage, &addr_size);

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &serverStorage.sin_addr, clientIP, INET_ADDRSTRLEN);
        int clientPort = ntohs(serverStorage.sin_port);

        printf("\nConnected client: <%s, %d>\n", clientIP, clientPort);

        // Add to unique client list if not already present
        int exists = 0;
        for (int i = 0; i < clientCount; i++) {
            if (strcmp(clients[i].ip, clientIP) == 0 && clients[i].port == clientPort) {
                exists = 1;
                break;
            }
        }
        if (!exists && clientCount < MAX_CLIENTS) {
            strcpy(clients[clientCount].ip, clientIP);
            clients[clientCount].port = clientPort;
            clientCount++;
        }

        // Receive client request
        recv(newSocket, buffer, 1024, 0);
        char fruitName[50];
        int qty;
        sscanf(buffer, "%s %d", fruitName, &qty);

        printf("Client requested: %d %s\n", qty, fruitName);

        int found = 0;
        char response[1024];
        for (int i = 0; i < MAX_FRUITS; i++) {
            if (strcmp(fruits[i].name, fruitName) == 0) {
                found = 1;
                if (fruits[i].quantity >= qty) {
                    fruits[i].quantity -= qty;

                    time_t now = time(NULL);
                    strcpy(fruits[i].lastSold, ctime(&now));
                    fruits[i].lastSold[strcspn(fruits[i].lastSold, "\n")] = '\0';

                    sprintf(response, "✅ Purchase successful! Remaining %s = %d\nLast sold: %s\nUnique customers: %d\n",
                            fruits[i].name, fruits[i].quantity, fruits[i].lastSold, clientCount);
                } else {
                    sprintf(response, "❌ Not enough stock for %s. Only %d available.\nUnique customers: %d\n",
                            fruits[i].name, fruits[i].quantity, clientCount);
                }
                break;
            }
        }

        if (!found) {
            sprintf(response, "❌ Fruit '%s' not found.\nUnique customers: %d\n", fruitName, clientCount);
        }

        send(newSocket, response, strlen(response) + 1, 0);
        close(newSocket);
    }

    return 0;
}

