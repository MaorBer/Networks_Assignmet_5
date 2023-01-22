#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/if_packet.h>
#include <pcap.h>

#define ETHER_ADDR_LEN	6

void got_packet(u_char *, const struct pcap_pkthdr *, const u_char *);

/* Ethernet header */
struct ethheader
{
  u_char ether_dhost[ETHER_ADDR_LEN]; /* destination host address */
  u_char ether_shost[ETHER_ADDR_LEN]; /* source host address */
  u_short ether_type;                 /* IP? ARP? RARP? etc */
};

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

FILE *file = NULL;

struct applicationheader {
    uint32_t unix_time;
    uint16_t tot_len;
    uint16_t flags;
    uint16_t cache_c;
    uint16_t padding;
};

int main()
{
    file = fopen("213358039_212305965.txt","w");
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    char filter_exp[] = "tcp";
    bpf_u_int32 net = 0;

    handle = pcap_open_live("lo", 5000, 1, 1000, errbuf);

    pcap_compile(handle, &fp, filter_exp, 0, net);

    pcap_setfilter(handle, &fp); 

    pcap_loop(handle, -1, got_packet, NULL);    
                
    pcap_close(handle);
    fclose(file);

    return 0;
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    if (header->len > 100)
    {
        struct sockaddr_in source, dest;
        struct iphdr *ip = (struct iphdr *)( packet  + sizeof(struct ethheader) );
        source.sin_addr.s_addr = ip->saddr;
        dest.sin_addr.s_addr = ip->daddr;
        struct tcphdr *tcp=(struct tcphdr*)(packet + sizeof(struct ethheader) + ip->ihl*4);     
        struct applicationheader *app = (struct applicationheader*) (packet + sizeof(struct ethheader) + ip->ihl*4 + tcp->doff*4);

        fprintf(file,"{source_ip: %s, dest_ip: %s, source_port: %d, dest_port: %d, timestamp : %d, total_length : %d, cache_flag : %d, steps_flag: %d, type_flag : %d, status_code : %d, cache_control : %d, data:\n",
        (char *) inet_ntoa(source.sin_addr), (char *) inet_ntoa(dest.sin_addr), ntohs(tcp->source), ntohs(tcp->dest), ntohl(app->unix_time), ntohs(app->tot_len), (ntohs(app->flags) >> 12)  & 0x1, (ntohs(app->flags) >> 11) & 0x1, (ntohs(app->flags) >> 10)  & 0x1, ntohs(app->flags) & ((1 << 10) - 1), ntohs(app->cache_c));

        unsigned char *data = (unsigned char *)(packet + sizeof(struct ethheader) + ip->ihl*4 + tcp->doff*4 + sizeof(struct applicationheader));

        for (int i = 0; i < header->len - (sizeof(struct ethhdr*) + ip->ihl*4 + tcp->doff*4 + sizeof(struct applicationheader*)); i++ )
        {
            if ( !(i & 15) ) fprintf(file, "\n   %04X:  ", i);
            fprintf(file, "%04X ", ((unsigned char*)(data))[i]);
        }

        fprintf(file, "}\n");
        fflush(file);
    }
}