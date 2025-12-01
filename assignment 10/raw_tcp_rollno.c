#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

// Pseudo header for TCP checksum calculation
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

// Checksum calculation function
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main() {
    int sockfd;
    char source_ip[] = "10.0.0.1";      // Source IP
    char dest_ip[] = "10.0.0.2";        // Destination IP
    char roll_number[] = "CSM24019";  // Replace with your roll number
    
    struct sockaddr_in dest_addr;
    char packet[4096];
    
    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Set IP_HDRINCL option
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }
    
    // Zero out the packet buffer
    memset(packet, 0, 4096);
    
    // IP Header
    struct iphdr *iph = (struct iphdr *)packet;
    
    // TCP Header
    struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));
    
    // Data (Roll Number)
    char *data = packet + sizeof(struct iphdr) + sizeof(struct tcphdr);
    strcpy(data, roll_number);
    int data_len = strlen(data);
    
    // Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + data_len;
    iph->id = htons(54321);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;
    iph->saddr = inet_addr(source_ip);
    iph->daddr = inet_addr(dest_ip);
    
    // Calculate IP checksum
    iph->check = checksum((unsigned short *)packet, iph->tot_len);
    
    // Fill in the TCP Header
    tcph->source = htons(12345);        // Source port
    tcph->dest = htons(80);             // Destination port
    tcph->seq = htonl(1000);
    tcph->ack_seq = 0;
    tcph->doff = 5;                     // TCP header size
    tcph->fin = 0;
    tcph->syn = 1;                      // SYN flag set
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(5840);
    tcph->check = 0;
    tcph->urg_ptr = 0;
    
    // Calculate TCP checksum with pseudo header
    struct pseudo_header psh;
    psh.source_address = inet_addr(source_ip);
    psh.dest_address = inet_addr(dest_ip);
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + data_len);
    
    int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + data_len;
    char *pseudogram = malloc(psize);
    
    memcpy(pseudogram, (char *)&psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr) + data_len);
    
    tcph->check = checksum((unsigned short *)pseudogram, psize);
    
    // Destination address
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(80);
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
    
    // Send the packet
    if (sendto(sockfd, packet, iph->tot_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    } else {
        printf("✓ TCP packet sent successfully!\n");
        printf("  Source: %s:12345\n", source_ip);
        printf("  Destination: %s:80\n", dest_ip);
        printf("  Payload: %s\n", roll_number);
        printf("  Packet size: %d bytes\n", iph->tot_len);
    }
    
    free(pseudogram);
    close(sockfd);
    
    printf("\nCapture this packet in Wireshark on the receiver to verify payload!\n");
    
    return 0;
}

