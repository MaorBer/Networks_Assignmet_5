#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define PORT 9998
#define PORT1 9999
#define BUFFER_SIZE 1024

int main(int argc, char **argv) {

    //get the ip to ping from the user
    char *destantion_ip=argv[1];
    
    // Create socket
    int socket_port = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_port == -1) {
        printf("Could not create socket : %d\n", errno);
        close(socket_port);
        return -1;
    }
    printf("socket_port created successfully!\n");

    // setup Server address structure with port 9998
    struct sockaddr_in serverAddress;
    memset((char *)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT); // using port 9998
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // create another socket to send with port 9999
    int socket_port_plus1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_port_plus1 == -1) {
        printf("Could not create socket : %d\n", errno);
        close(socket_port);
        close(socket_port_plus1);
        return -1;
    }
    printf("socket_port_plus1 created successfully!\n");

    // Initialize outgoing socket address with port 9999 - to send
    struct sockaddr_in socket_p1;
    memset(&socket_p1, 0, sizeof(socket_p1));
    socket_p1.sin_family = AF_INET;
    socket_p1.sin_port = htons(PORT1);
    int adr=inet_pton(AF_INET,(const char*)destantion_ip ,&socket_p1.sin_addr);
    if (adr<=0) {
        perror("inet_aton");
        close(socket_port);
        close(socket_port_plus1);
        return -1;
    }


    // Binding to socket_port
    int binding = bind(socket_port, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (binding == -1) {
        printf("bind() failed with error code : %d\n", errno);
        // cleanup the socket;
        close(socket_port);
        close(socket_port_plus1);
        return -1;
    }
    printf("After bind(). Waiting for clients\n");

    // setup Client address structure
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);

    memset((char *)&clientAddress, 0, sizeof(clientAddress));

    // keep listening for data
    while (1) {
        //fflush(stdout);

        // zero client address
        memset((char *)&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);

        char buffer[BUFFER_SIZE] = {'\0'};
        
        // clear the buffer by filling null, it might have previously received data
        memset(buffer, '\0', sizeof(buffer));

        // try to receive some data, this is a blocking call
        int CheckingReceive = recvfrom(socket_port, buffer, sizeof(buffer)-1, 0, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (CheckingReceive == -1) {
            printf("recvfrom() failed with error code : %d", errno);
            // cleanup the socket;
            close(socket_port);
            close(socket_port_plus1);
            break;
        }
        printf("recived packet successfully!\n");

        char clientIPAddrReadable[32] = {'\0'};
        inet_ntop(AF_INET, &clientAddress.sin_addr, clientIPAddrReadable, sizeof(clientIPAddrReadable));

        // print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", clientIPAddrReadable, ntohs(clientAddress.sin_port));
        printf("Data is: %s\n", buffer);

        if(CheckingReceive>0)
        {
            float random_number=((float)random())/((float)RAND_MAX);
            if(random_number>0.5)
            {
                int checking_send=sendto(socket_port_plus1,buffer,sizeof(buffer),0,(struct sockaddr*)&socket_p1,sizeof(socket_p1));
                if(checking_send==-1){
                    printf("sendto() failed with error code : %d\n", errno);
                    close(socket_port);
                    close(socket_port_plus1);
                    break;
                }
                else{
                    printf("send packet to port+1!\n");
                }
            }
        }
        printf("\n");
    }

    //closing sockets
    close(socket_port);
    close(socket_port_plus1);
    return 0;
}