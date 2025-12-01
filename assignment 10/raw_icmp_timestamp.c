#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

// ICMP checksum calculation
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main() {
    int sockfd;
    char source_ip[] = "10.0.0.1";      // Sender
    char dest_ip[] = "10.0.0.2";        // Receiver
    
    struct sockaddr_in dest_addr;
    char packet[1024];
    
    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // IP_HDRINCL to create custom header
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt error");
        exit(1);
    }
    
    memset(packet, 0, sizeof(packet));
    
    struct iphdr *iph = (struct iphdr *)packet;
    struct icmphdr *icmph = (struct icmphdr *)(packet + sizeof(struct iphdr));
    
    // IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
    iph->id = htons(rand() % 65535);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_ICMP;
    iph->check = 0;
    iph->saddr = inet_addr(source_ip);
    iph->daddr = inet_addr(dest_ip);
    iph->check = checksum((unsigned short *)packet, sizeof(struct iphdr));
    
    // ICMP Timestamp Request Header
    icmph->type = ICMP_TIMESTAMP;       // Type 13 - Timestamp Request
    icmph->code = 0;
    icmph->un.echo.id = htons(getpid() & 0xFFFF);
    icmph->un.echo.sequence = htons(1);
    icmph->checksum = 0;
    icmph->checksum = checksum((unsigned short *)icmph, sizeof(struct icmphdr));
    
    // Fill in destination address
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
    
    // Send packet
    if (sendto(sockfd, packet, iph->tot_len, 0,
               (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
        exit(1);
    }
    
    printf("✓ ICMP Timestamp Request sent!\n");
    printf("  Source: %s\n", source_ip);
    printf("  Destination: %s\n", dest_ip);
    printf("  Type: 13 (Timestamp Request)\n");
    printf("  Packet size: %d bytes\n", iph->tot_len);
    
    printf("\nCapture this packet in Wireshark on receiver (h2)!\n");
    printf("Filter: icmp.type == 13\n");
    
    close(sockfd);
    return 0;
}


