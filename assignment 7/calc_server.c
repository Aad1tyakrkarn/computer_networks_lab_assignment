#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define PI 3.14159265358979323846

double evaluate_expression(char *expr) {
    char operation[20];
    double num1, num2, result;
    
    // Try single operand functions first
    if (sscanf(expr, "sin(%lf)", &num1) == 1) {
        return sin(num1 * PI / 180.0);
    }
    else if (sscanf(expr, "cos(%lf)", &num1) == 1) {
        return cos(num1 * PI / 180.0);
    }
    else if (sscanf(expr, "tan(%lf)", &num1) == 1) {
        return tan(num1 * PI / 180.0);
    }
    else if (sscanf(expr, "sqrt(%lf)", &num1) == 1) {
        return sqrt(num1);
    }
    else if (sscanf(expr, "log(%lf)", &num1) == 1) {
        return log10(num1);
    }
    else if (sscanf(expr, "ln(%lf)", &num1) == 1) {
        return log(num1);
    }
    else if (sscanf(expr, "exp(%lf)", &num1) == 1) {
        return exp(num1);
    }
    else if (sscanf(expr, "inv(%lf)", &num1) == 1 || sscanf(expr, "1/%lf", &num1) == 1) {
        if (num1 == 0) return NAN;
        return 1.0 / num1;
    }
    else if (sscanf(expr, "abs(%lf)", &num1) == 1) {
        return fabs(num1);
    }
    // Try two operand operations
    else if (sscanf(expr, "%lf+%lf", &num1, &num2) == 2) {
        return num1 + num2;
    }
    else if (sscanf(expr, "%lf-%lf", &num1, &num2) == 2) {
        return num1 - num2;
    }
    else if (sscanf(expr, "%lf*%lf", &num1, &num2) == 2) {
        return num1 * num2;
    }
    else if (sscanf(expr, "%lf/%lf", &num1, &num2) == 2) {
        if (num2 == 0) return NAN;
        return num1 / num2;
    }
    else if (sscanf(expr, "pow(%lf,%lf)", &num1, &num2) == 2) {
        return pow(num1, num2);
    }
    else if (sscanf(expr, "%lf^%lf", &num1, &num2) == 2) {
        return pow(num1, num2);
    }
    
    return NAN;
}

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    int packet_count = 0;
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    
    // Server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    printf("╔═══════════════════════════════════════╗\n");
    printf("║  Scientific Calculator Server (UDP)   ║\n");
    printf("╠═══════════════════════════════════════╣\n");
    printf("║  Server started on port %d           ║\n", PORT);
    printf("║  Waiting for calculations...          ║\n");
    printf("╚═══════════════════════════════════════╝\n\n");
    
    printf("Supported operations:\n");
    printf("  - Trigonometric: sin(x), cos(x), tan(x)\n");
    printf("  - Logarithmic: log(x), ln(x), exp(x)\n");
    printf("  - Arithmetic: x+y, x-y, x*y, x/y\n");
    printf("  - Power: pow(x,y), x^y\n");
    printf("  - Others: sqrt(x), inv(x), abs(x)\n");
    printf("  Note: Angles in degrees\n\n");
    
    while (1) {
        socklen_t len = sizeof(cliaddr);
        memset(buffer, 0, BUFFER_SIZE);
        
        // Receive expression from client
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("Receive failed");
            continue;
        }
        
        buffer[n] = '\0';
        packet_count++;
        
        char *client_ip = inet_ntoa(cliaddr.sin_addr);
        int client_port = ntohs(cliaddr.sin_port);
        
        printf("[Packet #%d] From %s:%d\n", packet_count, client_ip, client_port);
        printf("Expression: %s\n", buffer);
        
        // Check for exit command
        if (strcmp(buffer, "exit") == 0) {
            printf("Client requested exit.\n\n");
            char *response = "Server: Goodbye!";
            sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *)&cliaddr, len);
            continue;
        }
        
        // Evaluate expression
        double result = evaluate_expression(buffer);
        char response[BUFFER_SIZE];
        
        if (isnan(result)) {
            sprintf(response, "ERROR: Invalid expression or division by zero");
            printf("Result: %s\n\n", response);
        } else {
            sprintf(response, "%.10lf", result);
            printf("Result: %s\n\n", response);
        }
        
        // Send result back to client
        sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *)&cliaddr, len);
    }
    
    close(sockfd);
    return 0;
}
