#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 2

int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[BUFFER_SIZE];
    char expression[BUFFER_SIZE];
    struct sockaddr_in servaddr;
    struct timeval tv;
    int sent_packets = 0;
    int received_packets = 0;
    
    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return -1;
    }
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Set timeout for socket
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    memset(&servaddr, 0, sizeof(servaddr));
    
    // Server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    
    printf("╔═══════════════════════════════════════╗\n");
    printf("║  Scientific Calculator Client (UDP)   ║\n");
    printf("╠═══════════════════════════════════════╣\n");
    printf("║  Connected to: %-22s ║\n", argv[1]);
    printf("╚═══════════════════════════════════════╝\n\n");
    
    printf("Supported operations:\n");
    printf("  - Trigonometric: sin(30), cos(45), tan(60)\n");
    printf("  - Logarithmic: log(100), ln(2.718), exp(2)\n");
    printf("  - Arithmetic: 5+3, 10-4, 6*7, 20/4\n");
    printf("  - Power: pow(2,3), 2^8\n");
    printf("  - Others: sqrt(16), inv(4), abs(-5)\n");
    printf("  - Type 'exit' to quit\n\n");
    
    while (1) {
        printf("Enter expression: ");
        fgets(expression, BUFFER_SIZE, stdin);
        expression[strcspn(expression, "\n")] = 0;  // Remove newline
        
        if (strlen(expression) == 0) continue;
        
        // Send expression to server
        sendto(sockfd, expression, strlen(expression), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        sent_packets++;
        
        if (strcmp(expression, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }
        
        // Receive result from server
        memset(buffer, 0, BUFFER_SIZE);
        socklen_t len = sizeof(servaddr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&servaddr, &len);
        
        if (n < 0) {
            printf("⚠️  PACKET LOSS DETECTED - No response from server (timeout)\n");
            printf("   Possible packet loss or server unavailable\n\n");
        } else {
            buffer[n] = '\0';
            received_packets++;
            printf("✓ Result: %s\n\n", buffer);
        }
    }
    
    // Statistics
    printf("\n╔═══════════════════════════════════════╗\n");
    printf("║         Connection Statistics         ║\n");
    printf("╠═══════════════════════════════════════╣\n");
    printf("║ Packets Sent: %-23d ║\n", sent_packets);
    printf("║ Packets Received: %-19d ║\n", received_packets);
    printf("║ Packets Lost: %-23d ║\n", sent_packets - received_packets);
    
    if (sent_packets > 0) {
        double loss_rate = ((double)(sent_packets - received_packets) / sent_packets) * 100;
        printf("║ Packet Loss Rate: %-18.2f%% ║\n", loss_rate);
    }
    
    printf("╚═══════════════════════════════════════╝\n");
    
    close(sockfd);
    return 0;
}
