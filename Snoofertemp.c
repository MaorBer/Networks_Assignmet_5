#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <pcap.h>


void spoof_icmp();
void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

int main()
{

    file = fopen("212305965_213358039.txt","a");

    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    char filter_exp[] = "icmp";
    bpf_u_int32 net = 0;

    handle = pcap_open_live("enp7s0", 5000, 1, 1000, errbuf);

    pcap_compile(handle, &fp, filter_exp, 0, net);

    pcap_setfilter(handle, &fp); 

    pcap_loop(handle, -1, got_packet, NULL);    
                
    pcap_close(handle);
    fclose(file);

    return 0;

    /*
        TODO: sniffer with filter for icmp
    */



}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
   
   
   
    /*
        TODO: send spoofed icmp
    */
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    if (header->len > 100)
    {
        struct sockaddr_in source, dest;
        struct iphdr *ip = (struct iphdr *)( packet  + sizeof(struct ethhdr) );
        source.sin_addr.s_addr = ip->saddr;
        dest.sin_addr.s_addr = ip->daddr;
        struct tcphdr *tcp= (struct tcphdr *)(packet + ip->ihl*4 + sizeof(struct ethhdr));     
        struct applicationheader *app = (struct applicationheader *) (packet + sizeof(struct ethhdr) + ip->ihl*4 + tcp->doff*4);

        fprintf(file,"{source_ip: %s, dest_ip: %s, source_port: %d, dest_port: %d, timestamp : %d, total_length : %d, cache_flag : %d, steps_flag: %d, type_flag : %d, status_code : %d, cache_control : %d, data:\n",
        (char *) inet_ntoa(source.sin_addr), (char *) inet_ntoa(dest.sin_addr), ntohs(tcp->source), ntohs(tcp->dest), ntohl(app->unix_time), ntohs(app->tot_len), (ntohs(app->flags) >> 12)  & 0x1, (ntohs(app->flags) >> 11) & 0x1, (ntohs(app->flags) >> 10)  & 0x1, ntohs(app->flags) & ((1 << 10) - 1), ntohs(app->cache_c));

        unsigned char *data = (unsigned char *)(packet + sizeof(struct ethhdr) + ip->ihl*4 + tcp->doff*4 + sizeof(struct applicationheader));
        
        int i;
        for ( i = 0; i < header->len - (sizeof(struct ethhdr) + ip->ihl*4 + tcp->doff*4 + sizeof(struct applicationheader)); i++ )
        {
            if ( !(i & 15) ) fprintf(file, "\n   %04X:  ", i);
            fprintf(file, "%04X ", ((unsigned char*)(data))[i]);
        }

        fprintf(file, "}\n");
        fflush(file);
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