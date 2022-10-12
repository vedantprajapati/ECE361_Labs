#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 32
#define RECV_PACKET_SIZE 1100
#define MAX_PACKET_SIZE 1000
#define MIN(a, b) (a > b ? b : a)

struct packet { //packet format: "total_frag:frag_no:size:filename:filedata"
    unsigned int total_frag; //total number of fragments of the file
    unsigned int frag_no; //sequence number of fragment
    unsigned int size; //size of data, range [0,1000]
    char* filename;
    char filedata[MAX_PACKET_SIZE];
};

int stringToPacket(char* recvBuffer, struct packet *recvPacket){
    //packet format: "total_frag:frag_no:size:filename:filedata"
    char* token;
    token = strtok(recvBuffer,":");
    int count = 0;
    //     printf("total_frag : %s\n", strtok(fragment, ":"));
    printf("yooooooooo");
    printf("vuffer: %s\n", recvBuffer);

    printf("frag_no: %s\n", token);

    while(token != NULL){
        switch (count)
        {
        case 0:
            recvPacket->total_frag = token;
            count++;
            break;
        case 1:
            token = strtok(recvBuffer,":");
            recvPacket->frag_no = token;
            count++;
            break;
        case 2:
            token = strtok(recvBuffer,":");
            recvPacket->size = token;
            count++;
            break;
        case 3:
            token = strtok(recvBuffer,":");
            recvPacket->filename = token;
            count++;
            break;
        case 4:
            token = strtok(recvBuffer,":");
            printf("frag_no: %s\n", token);
            // recvPacket->filedata = token;
            count++;
            break;
        default:
            printf("string to packet error\n");
            exit(2);
            break;
        }
    }
    return 0;
}
int parsePacket(char* recvBuffer, struct packet *recvPacket){
    //packet format: "total_frag:frag_no:size:filename:filedata"
    char* token;
    token = strtok(recvBuffer,":");
    int count = 0;
    //     printf("total_frag : %s\n", strtok(fragment, ":"));
    printf("yooooooooo");
    printf("vuffer: %s\n", recvBuffer);

    printf("frag_no: %s\n", token);

}

int main(int argc, char const *argv[]){
    
    if(argc != 2){
        printf("usage: server <udp listen port>\n");
        exit(0);
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int sock;
    struct sockaddr_in server, client;
    char dataBuffer[RECV_PACKET_SIZE];
    socklen_t addr_size;

    char *ftp = "ftp";
    char *no = "no";
    char *yes = "yes";
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    bind(sock, (struct sockaddr *)&server, sizeof(server));

    printf("server started on %d\n", port);

    addr_size = sizeof(client);

    char **packetsStrings;
    bool packetsAllocated = false;
    int receivedPackets = 0;
    int fileSize = 0;
    while(1){
        recvfrom(sock, dataBuffer, sizeof(dataBuffer), 0, (struct sockaddr *) &client, &addr_size);

        if(strcmp(dataBuffer, ftp) == 0){
            printf("received ftp\n");
            sendto(sock, yes, (strlen(yes)+1), 0, (struct sockaddr *)&client, sizeof(client));
            printf("sent yes\n");
        }
        else{
             //create empty array of packets if not done yet
            int totalFrag = atoi(strtok(dataBuffer ,":"));

            if (!packetsAllocated && strcmp(dataBuffer, ftp) != 0){
                printf("%s\n",dataBuffer);

                printf("%d\n", totalFrag);
                packetsStrings = malloc(sizeof(char*) * totalFrag);
                packetsAllocated = true;
            }
            // save a new packet string
            // if (currPacketIndex == 0 && packetsAllocated != 0){

            // ** FOR SOME
            int fragNo = atoi(strtok(NULL,":"));

            printf("got %d\n", fragNo);
            int size = atoi(strtok(NULL,":"));
            char *fileName = strtok(NULL,":");
            char *filedata = fileName + strlen(fileName) + 1;

            packetsStrings[fragNo-1] = malloc(sizeof(char)*size);
            memcpy(packetsStrings[fragNo-1], filedata, sizeof(char)*size);

            sendto(sock, no, (strlen(no)+1), 0, (struct sockaddr *)&client, sizeof(client));
            printf("sent no\n");

            receivedPackets++;
            fileSize += size;

            if (receivedPackets == totalFrag) {
                FILE* fp = fopen(fileName, "wb");
                for (int i = 0; i < totalFrag; i++) {
                    int wrote = fwrite(packetsStrings[i], 1, MIN(fileSize, MAX_PACKET_SIZE), fp);
                    fileSize -= wrote;
                    free(packetsStrings[i]);
                }
                fclose(fp);

                receivedPackets = 0;
                fileSize = 0;
                packetsAllocated = false;
                free(packetsStrings);
                packetsStrings = NULL;
            }
        }
    }

    close(sock);
    return 0;
}