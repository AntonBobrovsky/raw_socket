#define __USE_BSD
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define DATALEN 4096
#define IP_HEADER_LEN sizeof(struct ipheader)
#define UDP_HEADER_LEN sizeof(struct udpheader)

#define LOCALHOST "127.0.0.1"
#define PORT 5555

struct ipheader {
    unsigned char ip_hl:4, ip_v:4;
    unsigned char ip_tos;
    unsigned short int ip_len;
    unsigned short int ip_id;
    unsigned short int ip_off;
    unsigned char ip_ttl;
    unsigned char ip_p;
    unsigned short int ip_sum;
    unsigned int ip_src;
    unsigned int ip_dst;
}__attribute__((__packed__ ));

struct udpheader {
    unsigned short int sport;
    unsigned short int dport;
    unsigned short int len;
    unsigned short int check;
}__attribute__((__packed__ ));

uint16_t iphdr_checksum (uint16_t *addr, int len);

int main(int argc, char const *argv[]) {
    int sock;
    char datagram[DATALEN];
    struct ipheader *iphdr = (struct ipheader *) datagram;
    struct udpheader *udphdr = (struct udpheader *) (datagram + sizeof (struct ipheader));
    struct sockaddr_in addr;

    /*data for send datagram*/
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = PORT;
    addr.sin_addr.s_addr = inet_addr(LOCALHOST);

    bzero(&datagram, sizeof(datagram));
    strcpy(datagram + IP_HEADER_LEN + UDP_HEADER_LEN, "Hello World !");

    /*fill ip/tcp header*/
    iphdr->ip_hl = 5;
    iphdr->ip_v = 4;
    iphdr->ip_tos = 0;
    iphdr->ip_len = IP_HEADER_LEN + UDP_HEADER_LEN + DATALEN;
    iphdr->ip_id = 0;
    iphdr->ip_off = 0;
    iphdr->ip_ttl = 255;
    iphdr->ip_p = IPPROTO_UDP;
    iphdr->ip_src = inet_addr(LOCALHOST);
    iphdr->ip_dst = inet_addr(LOCALHOST);
    iphdr->ip_sum = 0;
    iphdr->ip_sum = iphdr_checksum((uint16_t *) &iphdr, IP_HEADER_LEN);

    udphdr->sport = htons(1234);
    udphdr->dport = htons(PORT);
    udphdr->len = UDP_HEADER_LEN + DATALEN;
    udphdr->check = 0;

    /*create raw socket*/
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("socket()");
        exit(-1);
    }

    for (int i = 0; i < 5; i++) {
        if (sendto(sock, datagram, iphdr->ip_len, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("sendto()");
            exit(-1);
        }
    }

    return 0;
}

// Computing the internet checksum (RFC 1071).
uint16_t iphdr_checksum (uint16_t *addr, int len) {
    int count = len;
    register uint32_t sum = 0;
    uint16_t answer = 0;

    while (count > 1) {
        sum += *(addr++);
        count -= 2;
    }

    if (count > 0) {
        sum += *(uint8_t *) addr;
    }

    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    answer = ~sum;
    return answer;
}
