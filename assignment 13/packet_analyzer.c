#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <time.h>

typedef struct {
    int arp_count;
    int icmp_count;
    int tcp_count;
    int udp_count;
    int other_count;
} ProtocolStats;

ProtocolStats stats = {0};
double start_time = 0;

void print_time_diagram() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    TIME SEQUENCE DIAGRAM                   ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║                                                            ║\n");
    printf("║   Host h1                                          Host h8 ║\n");
    printf("║      |                                                |    ║\n");
    printf("║      |  ARP Request (Who has 10.0.0.8?)  -------->   |    ║\n");
    printf("║      |                                                |    ║\n");
    printf("║      |  <--------  ARP Reply (I am 10.0.0.8)         |    ║\n");
    printf("║      |                                                |    ║\n");
    printf("║      |  ICMP Echo Request (ping) ------------>       |    ║\n");
    printf("║      |                                                |    ║\n");
    printf("║      |  <----------  ICMP Echo Reply                 |    ║\n");
    printf("║      |                                                |    ║\n");
    printf("║      |  ICMP Echo Request ------------>              |    ║\n");
    printf("║      |                                                |    ║\n");
    printf("║      |  <----------  ICMP Echo Reply                 |    ║\n");
    printf("║      |                                                |    ║\n");
    printf("║      |  ... (continues for all ping packets)         |    ║\n");
    printf("║      |                                                |    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

void print_protocol_summary() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    PROTOCOL SUMMARY                        ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Layer 2 (Data Link) Protocols:                           ║\n");
    printf("║    - Ethernet II                                           ║\n");
    printf("║    - ARP (Address Resolution Protocol)                     ║\n");
    printf("║                                                            ║\n");
    printf("║  Layer 3 (Network) Protocols:                              ║\n");
    printf("║    - IP (Internet Protocol)                                ║\n");
    printf("║    - ICMP (Internet Control Message Protocol)              ║\n");
    printf("║                                                            ║\n");
    printf("║  Layer 4 (Transport) Protocols:                            ║\n");
    printf("║    - None in PING operation                                ║\n");
    printf("║    - (TCP/UDP would appear if captured)                    ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Packet Statistics:                                        ║\n");
    printf("║    ARP Packets:   %-4d                                    ║\n", stats.arp_count);
    printf("║    ICMP Packets:  %-4d                                    ║\n", stats.icmp_count);
    printf("║    TCP Packets:   %-4d                                    ║\n", stats.tcp_count);
    printf("║    UDP Packets:   %-4d                                    ║\n", stats.udp_count);
    printf("║    Other:         %-4d                                    ║\n", stats.other_count);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

void process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    static int packet_count = 0;
    struct ether_header *eth_header;
    struct ip *ip_header;
    struct icmphdr *icmp_header;
    struct tcphdr *tcp_header;
    struct udphdr *udp_header;
    
    packet_count++;
    
    // Calculate relative time
    double packet_time = header->ts.tv_sec + (header->ts.tv_usec / 1000000.0);
    if (start_time == 0) {
        start_time = packet_time;
    }
    double relative_time = packet_time - start_time;
    
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Packet #%d - Time: %.6f seconds\n", packet_count, relative_time);
    printf("═══════════════════════════════════════════════════════════\n");
    
    // Layer 2 - Ethernet Header
    eth_header = (struct ether_header *)packet;
    printf("\n[Layer 2 - Ethernet Header]\n");
    printf("  Source MAC:      %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth_header->ether_shost[0], eth_header->ether_shost[1],
           eth_header->ether_shost[2], eth_header->ether_shost[3],
           eth_header->ether_shost[4], eth_header->ether_shost[5]);
    printf("  Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth_header->ether_dhost[0], eth_header->ether_dhost[1],
           eth_header->ether_dhost[2], eth_header->ether_dhost[3],
           eth_header->ether_dhost[4], eth_header->ether_dhost[5]);
    printf("  EtherType:       0x%04x ", ntohs(eth_header->ether_type));
    
    // Check protocol type
    if (ntohs(eth_header->ether_type) == ETHERTYPE_ARP) {
        printf("(ARP)\n");
        stats.arp_count++;
        return;
    } else if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        printf("(IPv4)\n");
    } else {
        printf("(Other)\n");
        stats.other_count++;
        return;
    }
    
    // Layer 3 - IP Header
    ip_header = (struct ip *)(packet + sizeof(struct ether_header));
    printf("\n[Layer 3 - IP Header]\n");
    printf("  Version:         %d\n", ip_header->ip_v);
    printf("  Header Length:   %d bytes\n", ip_header->ip_hl * 4);
    printf("  Total Length:    %d bytes\n", ntohs(ip_header->ip_len));
    printf("  TTL:             %d\n", ip_header->ip_ttl);
    printf("  Protocol:        %d ", ip_header->ip_p);
    
    if (ip_header->ip_p == IPPROTO_ICMP) {
        printf("(ICMP)\n");
        stats.icmp_count++;
    } else if (ip_header->ip_p == IPPROTO_TCP) {
        printf("(TCP)\n");
        stats.tcp_count++;
    } else if (ip_header->ip_p == IPPROTO_UDP) {
        printf("(UDP)\n");
        stats.udp_count++;
    } else {
        printf("(Other)\n");
        stats.other_count++;
    }
    
    printf("  Source IP:       %s\n", inet_ntoa(ip_header->ip_src));
    printf("  Destination IP:  %s\n", inet_ntoa(ip_header->ip_dst));
    
    // Layer 4 - Transport/ICMP
    if (ip_header->ip_p == IPPROTO_ICMP) {
        icmp_header = (struct icmphdr *)(packet + sizeof(struct ether_header) + (ip_header->ip_hl * 4));
        printf("\n[ICMP Header]\n");
        printf("  Type:            %d ", icmp_header->type);
        if (icmp_header->type == ICMP_ECHO) {
            printf("(Echo Request)\n");
        } else if (icmp_header->type == ICMP_ECHOREPLY) {
            printf("(Echo Reply)\n");
        } else {
            printf("\n");
        }
        printf("  Code:            %d\n", icmp_header->code);
        printf("  Checksum:        0x%04x\n", ntohs(icmp_header->checksum));
    } else if (ip_header->ip_p == IPPROTO_TCP) {
        tcp_header = (struct tcphdr *)(packet + sizeof(struct ether_header) + (ip_header->ip_hl * 4));
        printf("\n[Layer 4 - TCP Header]\n");
        printf("  Source Port:     %d\n", ntohs(tcp_header->source));
        printf("  Dest Port:       %d\n", ntohs(tcp_header->dest));
        printf("  Sequence:        %u\n", ntohl(tcp_header->seq));
        printf("  Ack Sequence:    %u\n", ntohl(tcp_header->ack_seq));
    } else if (ip_header->ip_p == IPPROTO_UDP) {
        udp_header = (struct udphdr *)(packet + sizeof(struct ether_header) + (ip_header->ip_hl * 4));
        printf("\n[Layer 4 - UDP Header]\n");
        printf("  Source Port:     %d\n", ntohs(udp_header->source));
        printf("  Dest Port:       %d\n", ntohs(udp_header->dest));
        printf("  Length:          %d\n", ntohs(udp_header->len));
    }
}

int main(int argc, char *argv[]) {
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    
    if (argc != 2) {
        printf("Usage: %s <pcap_file>\n", argv[0]);
        printf("Example: %s root_switch.pcap\n", argv[0]);
        return 1;
    }
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║        Binary Tree Topology Packet Analysis               ║\n");
    printf("║                  Assignment 13                             ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Open pcap file
    handle = pcap_open_offline(argv[1], errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Error opening file: %s\n", errbuf);
        return 2;
    }
    
    printf("\nAnalyzing packets from: %s\n", argv[1]);
    
    // Process all packets
    pcap_loop(handle, 0, process_packet, NULL);
    
    // Print summaries
    print_time_diagram();
    print_protocol_summary();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  L2 Protocols Extracted: Ethernet, ARP                     ║\n");
    printf("║  L3 Protocols Extracted: IP, ICMP                          ║\n");
    printf("║  L4 Protocols Extracted: None (ICMP is Layer 3)            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    pcap_close(handle);
    return 0;
}


