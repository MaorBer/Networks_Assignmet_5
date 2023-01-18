#include <errno.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* IP Header */
struct ipheader
{
    unsigned char iph_ihl : 4,       // IP header length
        iph_ver : 4;                 // IP version
    unsigned char iph_tos;           // Type of service
    unsigned short int iph_len;      // IP Packet length (data + header)
    unsigned short int iph_ident;    // Identification
    unsigned short int iph_flag : 3, // Fragmentation flags
        iph_offset : 13;             // Flags offset
    unsigned char iph_ttl;           // Time to Live
    unsigned char iph_protocol;      // Protocol type
    unsigned short int iph_chksum;   // IP datagram checksum
    struct in_addr iph_sourceip;     // Source IP address
    struct in_addr iph_destip;       // Destination IP address
};

/* ICMP Header  */
struct icmpheader
{
    unsigned char icmp_type;        // ICMP message type
    unsigned char icmp_code;        // Error code
    unsigned short int icmp_chksum; // Checksum for ICMP Header and data
    unsigned short int icmp_id;     // Used for identifying request
    unsigned short int icmp_seq;    // Sequence number
};

/* UDP Header */
struct udpheader
{
    u_int16_t udp_sport; /* source port */
    u_int16_t udp_dport; /* destination port */
    u_int16_t udp_ulen;  /* udp length */
    u_int16_t udp_sum;   /* udp checksum */
};

#define PACKET_LEN 50

void spoof_icmp();
void spoof_udp();
void spoof_tcp();
void send_raw_ip_packet(struct ipheader *);
unsigned short in_cksum(unsigned short *, int);
unsigned short calculate_tcp_checksum(struct ipheader *);

/******************************************************************
  Spoof an ICMP echo request using an arbitrary source IP Address
*******************************************************************/
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("sage: sudo ./program [protocol: 1-icmp 2-udp 3-tcp]\n");
        return -1;
    }

    if (strcmp(argv[1], "1") == 0)
    {
        spoof_icmp();
    }
    else if (strcmp(argv[1], "2") == 0)
        spoof_udp();
    else
        spoof_tcp();

    return 1;
}

void spoof_icmp()
{
    char buffer[1500];
    memset(buffer, 0, 1500);
    /*********************************************************
       Step 1: Fill in the ICMP header.
     ********************************************************/
    struct icmpheader *icmp = (struct icmpheader *)(buffer + sizeof(struct ipheader));
    icmp->icmp_type = 8; // ICMP Type: 8 is request, 0 is reply.

    // Calculate the checksum for integrity
    icmp->icmp_chksum = 0;
    icmp->icmp_chksum = in_cksum((unsigned short *)icmp,
                                 sizeof(struct icmpheader));

    /*********************************************************
       Step 2: Fill in the IP header.
     ********************************************************/
    struct ipheader *ip = (struct ipheader *)buffer;
    ip->iph_ver = 4;
    ip->iph_ihl = 5;
    ip->iph_ttl = 20;
    ip->iph_sourceip.s_addr = inet_addr("6.6.6.6");
    ip->iph_destip.s_addr = inet_addr("5.5.5.5");
    ip->iph_protocol = IPPROTO_ICMP;
    ip->iph_len = htons(sizeof(struct ipheader) +
                        sizeof(struct icmpheader));

    /*********************************************************
       Step 3: Finally, send the spoofed packet
     ********************************************************/
    send_raw_ip_packet(ip);
}

void spoof_udp()
{
    char buffer[1500];

    memset(buffer, 0, 1500);
    struct ipheader *ip = (struct ipheader *)buffer;
    struct udpheader *udp = (struct udpheader *)(buffer +
                                                 sizeof(struct ipheader));

    /*********************************************************
       Step 1: Fill in the UDP data field.
     ********************************************************/
    char *data = buffer + sizeof(struct ipheader) +
                 sizeof(struct udpheader);
    const char *msg = "Hello Server!\n";
    int data_len = strlen(msg);
    strncpy(data, msg, data_len);

    /*********************************************************
       Step 2: Fill in the UDP header.
     ********************************************************/
    udp->udp_sport = htons(12345);
    udp->udp_dport = htons(96);
    udp->udp_ulen = htons(sizeof(struct udpheader) + data_len);
    udp->udp_sum = 0; /* Many OSes ignore this field, so we do not
                         calculate it. */

    /*********************************************************
       Step 3: Fill in the IP header.
     ********************************************************/

    /* Code omitted here; same as that in (*@Listing~\ref{snoof:list:icmpecho}@*) */
    ip->iph_ver = 4;
    ip->iph_ihl = 5;
    ip->iph_ttl = 20;
    ip->iph_sourceip.s_addr = inet_addr("6.6.6.6");
    ip->iph_destip.s_addr = inet_addr("5.5.5.5");
    ip->iph_protocol = IPPROTO_UDP; // The value is 17.
    ip->iph_len = htons(sizeof(struct ipheader) +
                        sizeof(struct udpheader) + data_len);

    /*********************************************************
       Step 4: Finally, send the spoofed packet
     ********************************************************/
    send_raw_ip_packet(ip);
}

void spoof_tcp()
{
    char buffer[1500];
    memset(buffer, 0, 1500);

    struct tcphdr *tcp = (struct tcphdr *)(buffer + sizeof(struct ipheader));

    /**
       Step 1: Fill in the TCP data field.
     **/
    char *data = buffer + sizeof(struct ipheader) + sizeof(struct tcphdr);
    char data_msg[8] = "yuvaloo";
    int data_len = strlen(data_msg);
    memcpy(data, data_msg, data_len);

    /**
       Step 2: Fill in the TCP header.
     **/
    tcp->source = htons(1234);
    tcp->dest = htons(4200);
    tcp->seq = htonl(1);
    tcp->ack = 0;
    tcp->doff = 5;
    tcp->psh = 1;
    tcp->th_win = htons(4200);
    tcp->th_urp = 0;
    tcp->th_sum = 0;
    tcp->th_sum = in_cksum((unsigned short *)tcp, sizeof(struct tcphdr) + data_len);

    /*********************************************************
       Step 3: Fill in the IP header.
     ********************************************************/
    struct ipheader *ip = (struct ipheader *)buffer;
    ip->iph_ver = 4;
    ip->iph_ihl = 5;
    ip->iph_ttl = 20;
    ip->iph_sourceip.s_addr = inet_addr("6.6.6.6");
    ip->iph_destip.s_addr = inet_addr("5.5.5.5");
    ip->iph_protocol = IPPROTO_TCP;
    ip->iph_len = htons(sizeof(struct ipheader) +
                        sizeof(struct tcphdr) + data_len);

    /*********************************************************
       Step 3: Finally, send the spoofed packet
     ********************************************************/
    send_raw_ip_packet(ip);
}

/**********************************************
 * Listing 12.9: Calculating Internet Checksum
 **********************************************/

unsigned short in_cksum(unsigned short *buf, int length)
{
    unsigned short *w = buf;
    int nleft = length;
    int sum = 0;
    unsigned short temp = 0;

    /*
     * The algorithm uses a 32 bit accumulator (sum), adds
     * sequential 16 bit words to it, and at the end, folds back all
     * the carry bits from the top 16 bits into the lower 16 bits.
     */
    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    /* treat the odd byte at the end, if any */
    if (nleft == 1)
    {
        *(u_char *)(&temp) = *(u_char *)w;
        sum += temp;
    }

    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    return (unsigned short)(~sum);
}

/*************************************************************
  Given an IP packet, send it out using a raw socket.
**************************************************************/
void send_raw_ip_packet(struct ipheader *ip)
{
    struct sockaddr_in dest_info;
    int enable = 1;

    // Step 1: Create a raw network socket.
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    // Step 2: Set socket option.
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL,
               &enable, sizeof(enable));

    // Step 3: Provide needed information about destination.
    dest_info.sin_family = AF_INET;
    dest_info.sin_addr = ip->iph_destip;

    // Step 4: Send the packet out.
    sendto(sock, ip, ntohs(ip->iph_len), 0,
           (struct sockaddr *)&dest_info, sizeof(dest_info));
    close(sock);
}
