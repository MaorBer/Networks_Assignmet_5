#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define PORT 9696
#define PORT1 9697
#define BUFFER_SIZE 1024

int main(int argc, char **argv) {

    //get the ip to ping from the user
    char *destIP=argv[1];
    
    // Create socket
    int sockA = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockA == -1) {
        printf("Could not create socket : %d\n", errno);
        close(sockA);
        return -1;
    }
    printf("sockA created successfully!\n");

    // setup Server address structure with port 9696
    struct sockaddr_in serverSide;
    memset((char *)&serverSide, 0, sizeof(serverSide));
    serverSide.sin_family = AF_INET;
    serverSide.sin_port = htons(PORT); // using port 9696
    serverSide.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // create another socket to send with port 9979
    int sockB = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockB == -1) {
        printf("Could not create socket : %d\n", errno);
        close(sockA);
        close(sockB);
        return -1;
    }
    printf("sockB created successfully!\n");

    // Initialize outgoing socket address with port 9979 - to send
    struct sockaddr_in socket_p1;
    memset(&socket_p1, 0, sizeof(socket_p1));
    socket_p1.sin_family = AF_INET;
    socket_p1.sin_port = htons(PORT1);
    int adr=inet_pton(AF_INET,(const char*)destIP ,&socket_p1.sin_addr);
    if (adr<=0) {
        perror("inet_aton");
        close(sockA);
        close(sockB);
        return -1;
    }


    // Binding to sock
    int binding = bind(sockA, (struct sockaddr *)&serverSide, sizeof(serverSide));
    if (binding == -1) {
        printf("bind() failed with error code : %d\n", errno);
        // cleanup the socket;
        close(sockA);
        close(sockB);
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
        int CheckingReceive = recvfrom(sockA, buffer, sizeof(buffer)-1, 0, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (CheckingReceive == -1) {
            printf("recvfrom() failed with error code : %d", errno);
            // cleanup the socket;
            close(sockA);
            close(sockB);
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
                int checking_send=sendto(sockB,buffer,sizeof(buffer),0,(struct sockaddr*)&socket_p1,sizeof(socket_p1));
                if(checking_send==-1){
                    printf("sendto() failed with error code : %d\n", errno);
                    close(sockA);
                    close(sockB);
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
    close(sockA);
    close(sockB);
    return 0;
}