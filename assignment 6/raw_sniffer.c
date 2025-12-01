#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <sys/socket.h>
#include <time.h>

int main(){
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(sock < 0){
        perror("socket");
        return 1;
    }

    printf("Sniffer started...\n");

    unsigned char buffer[65536];

    while(1){
        int data_size = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);

        if(data_size < 0){
            perror("recvfrom");
            continue;
        }

        struct ethhdr *eth = (struct ethhdr *)buffer;

        if(ntohs(eth->h_proto) == ETH_P_IP){
            struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));

            char src_ip[20], dst_ip[20];
            inet_ntop(AF_INET, &ip->saddr, src_ip, sizeof(src_ip));
            inet_ntop(AF_INET, &ip->daddr, dst_ip, sizeof(dst_ip));

            if(ip->protocol == IPPROTO_TCP){
                struct tcphdr *tcp =
                    (struct tcphdr *)(buffer + sizeof(struct ethhdr) + ip->ihl * 4);

                printf("\n=============================\n");
                printf("TCP PACKET\n");

                printf("SRC MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    eth->h_source[0], eth->h_source[1], eth->h_source[2],
                    eth->h_source[3], eth->h_source[4], eth->h_source[5]);

                printf("DST MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
                    eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);

                printf("SRC IP: %s\n", src_ip);
                printf("DST IP: %s\n", dst_ip);

                printf("SRC PORT: %u\n", ntohs(tcp->source));
                printf("DST PORT: %u\n", ntohs(tcp->dest));

                printf("=============================\n");
            }
        }
    }

    close(sock);
    return 0;
}
