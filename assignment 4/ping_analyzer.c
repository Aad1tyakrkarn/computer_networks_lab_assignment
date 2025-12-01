#include <stdio.h>
#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <time.h>

void process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    static int count = 1;
    struct ether_header *eth_header;
    struct ip *ip_header;
    struct icmphdr *icmp_header;
    
    printf("\n=== Packet %d ===\n", count++);
    printf("Timestamp: %ld.%06ld seconds\n", header->ts.tv_sec, header->ts.tv_usec);
    printf("Packet Length: %d bytes\n", header->len);
    
    // L2 - Ethernet Header
    eth_header = (struct ether_header *)packet;
    printf("\n--- Layer 2 (Ethernet) ---\n");
    printf("Source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth_header->ether_shost[0], eth_header->ether_shost[1],
           eth_header->ether_shost[2], eth_header->ether_shost[3],
           eth_header->ether_shost[4], eth_header->ether_shost[5]);
    printf("Dest MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth_header->ether_dhost[0], eth_header->ether_dhost[1],
           eth_header->ether_dhost[2], eth_header->ether_dhost[3],
           eth_header->ether_dhost[4], eth_header->ether_dhost[5]);
    printf("EtherType: 0x%04x\n", ntohs(eth_header->ether_type));
    
    // Check if IP packet
    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        // L3 - IP Header
        ip_header = (struct ip *)(packet + sizeof(struct ether_header));
        printf("\n--- Layer 3 (IP) ---\n");
        printf("Version: %d\n", ip_header->ip_v);
        printf("Header Length: %d bytes\n", ip_header->ip_hl * 4);
        printf("Type of Service: 0x%02x\n", ip_header->ip_tos);
        printf("Total Length: %d bytes\n", ntohs(ip_header->ip_len));
        printf("Identification: %d\n", ntohs(ip_header->ip_id));
        printf("TTL: %d\n", ip_header->ip_ttl);
        printf("Protocol: %d", ip_header->ip_p);
        
        if (ip_header->ip_p == IPPROTO_ICMP) {
            printf(" (ICMP)\n");
        } else if (ip_header->ip_p == IPPROTO_TCP) {
            printf(" (TCP)\n");
        } else if (ip_header->ip_p == IPPROTO_UDP) {
            printf(" (UDP)\n");
        } else {
            printf("\n");
        }
        
        printf("Source IP: %s\n", inet_ntoa(ip_header->ip_src));
        printf("Dest IP: %s\n", inet_ntoa(ip_header->ip_dst));
        
        // ICMP Header
        if (ip_header->ip_p == IPPROTO_ICMP) {
            icmp_header = (struct icmphdr *)(packet + sizeof(struct ether_header) + (ip_header->ip_hl * 4));
            printf("\n--- ICMP ---\n");
            printf("Type: %d ", icmp_header->type);
            
            if (icmp_header->type == ICMP_ECHO) {
                printf("(Echo Request)\n");
            } else if (icmp_header->type == ICMP_ECHOREPLY) {
                printf("(Echo Reply)\n");
            } else {
                printf("\n");
            }
            
            printf("Code: %d\n", icmp_header->code);
            printf("Checksum: 0x%04x\n", ntohs(icmp_header->checksum));
        }
    }
    
    printf("-----------------------------------\n");
}

int main(int argc, char *argv[]) {
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    
    if (argc != 2) {
        printf("Usage: %s <pcap_file>\n", argv[0]);
        return 1;
    }
    
    // Open pcap file
    handle = pcap_open_offline(argv[1], errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Error opening file: %s\n", errbuf);
        return 1;
    }
    
    printf("===========================================\n");
    printf("      PING PACKET ANALYSIS\n");
    printf("===========================================\n");
    
    // Process all packets
    pcap_loop(handle, 0, process_packet, NULL);
    
    // Summary
    printf("\n\n=== PROTOCOL SUMMARY ===\n");
    printf("L2 Protocols: Ethernet\n");
    printf("L3 Protocols: IP, ICMP\n");
    printf("L4 Protocols: None (ICMP is L3)\n");
    
    printf("\n=== TIME DIAGRAM ===\n");
    printf("Host A                    Host B\n");
    printf("  |                          |\n");
    printf("  |  ICMP Echo Request  ---> |\n");
    printf("  |                          |\n");
    printf("  | <--- ICMP Echo Reply     |\n");
    printf("  |                          |\n");
    printf("  |  ICMP Echo Request  ---> |\n");
    printf("  |                          |\n");
    printf("  | <--- ICMP Echo Reply     |\n");
    printf("  |                          |\n");
    
    pcap_close(handle);
    return 0;
}

/* Compile with: gcc ping_analyzer.c -o ping_analyzer -lpcap */
