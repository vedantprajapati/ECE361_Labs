#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <errno.h>

#include "../helpers.h"

#define MAX_DATA 1000
#define MAX_NAME 12
#define MAX_PASSWORD 16
#define BUFFER_SIZE 1024
#define PORT 8080

#define SA struct sockaddr

void textApp(int sockfd, char *client_id);

typedef struct message Message;

void process_input(char *input_buffer, Message *packet)
{
    // printf(input_buffer);
    char *token = strtok(input_buffer, ":");
    int i = 0;
    while (token != NULL)
    {
        switch (i)
        {
        case 0:
            packet->type = atoi(token);
            token = strtok(NULL, ":");
            i++;
            break;
        case 1:
            packet->size = atoi(token);
            token = strtok(NULL, ":");
            i++;
            break;
        case 2:
            strcpy(packet->source, token);
            token = strtok(NULL, ":");
            i++;
            break;
        case 3:
            strcpy(packet->data, token);
            token = strtok(NULL, ":");
            i++;
            break;
        default:
            printf("invalid packet - too many arguements\n");
            exit(0);
        }
    }
}

void login()
{
    char input_buffer[BUFFER_SIZE];
    char recv_buff[BUFFER_SIZE];
    bool invalid_command = true;
    char *client_id = NULL;
    char *password = NULL;
    char *ip = NULL;
    char *port = NULL;

    while (invalid_command)
    {
        printf("available commands:\n /login <client ID> <password> <server-IP> <server-port>\n");
        fgets(input_buffer, BUFFER_SIZE, stdin);

        char *token = strtok(input_buffer, " ");
        if (strcmp(token, "/login") == 0)
        {
            client_id = strtok(NULL, " ");
            password = strtok(NULL, " ");
            ip = strtok(NULL, " ");
            port = strtok(NULL, " ");
            invalid_command = false;
        }
        else
        {
            printf("invalid command\n");
        }
    }

    int sockfd;
    struct sockaddr_in serv_addr, client;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("socket creation error\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(atoi(port));

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("connection error\n");
        exit(0);
    }

    char *packet[MAX_DATA];
    strcpy(packet, "0:0:");
    strcat(packet, client_id);
    strcat(packet, ":");
    strcat(packet, password);
    // printf("%s",packet);

    write(sockfd, packet, strlen(packet));
    read(sockfd, recv_buff, BUFFER_SIZE);

    Message recvd_packet;
    process_input(recv_buff, &recvd_packet);
    printf("%s\n", recv_buff);

    if (recvd_packet.type == 1)
    {
        textApp(sockfd, client_id);
        close(sockfd);
    }
    else if (recvd_packet.type == 2)
    {
        printf("login failure: %s\n", recvd_packet.data);
        login();
    }
    else
    {
        printf("login error\n");
        exit(0);
    }
}

#define HELP_LOGOUT   " /logout\n"
#define HELP_JOIN     " /joinsession <session ID>\n"
#define HELP_LEAVE    " /leavesession\n"
#define HELP_CREATE   " /createsession <session ID>\n"
#define HELP_LIST     " /list\n"
#define HELP_QUIT     " /quit\n"

void textApp(int sockfd, char *client_id)
{
    char input_buffer[BUFFER_SIZE];
    size_t read_bytes;
    char server_buffer[BUFFER_SIZE];
    printf("log in successful\n \n");

    printf("available commands:\n" HELP_LOGOUT HELP_JOIN HELP_LEAVE HELP_CREATE HELP_LIST HELP_QUIT);

    while (1)
    {
        bzero(input_buffer, BUFFER_SIZE);
        fgets(input_buffer, BUFFER_SIZE, stdin);

        char *command = strtok(input_buffer, " ");
        if (strcmp(command, "/logout") == 0)
        {
            display_message(server_buffer, EXIT, 0, client_id, "");
            write(sockfd, server_buffer, strlen(server_buffer));
        }
        else if (strcmp(command, "/joinsession") == 0)
        {
            char *session_id = strtok(NULL, " ");
            if (session_id == NULL)
            {
                printf("usage: " HELP_JOIN);
                continue;
            }

            display_message(server_buffer, JOIN, strlen(session_id), client_id, session_id);
            write(sockfd, server_buffer, strlen(server_buffer));

            read_bytes = read(sockfd, server_buffer, BUFFER_SIZE);

            struct message ack; 
            convert_client_input_to_packet(server_buffer, &ack);
            if (ack.type == JN_ACK)
            {
                if (strcmp(ack.data, session_id) == 0)
                    printf("joined %s\n", session_id);
                else
                    printf("the server was bad (we joined %s)\n", session_id);
            }
            else
            {
                printf("failed to join %s\n", ack.data);
            }
        }
        else if (strcmp(command, "/leavesession") == 0)
        {
            display_message(server_buffer, LEAVE_SESS, 0, client_id, "");
        }
        else if (strcmp(command, "/createsession") == 0)
        {
            char *session_id = strtok(NULL, " ");
            if (session_id == NULL)
            {
                printf("usage: " HELP_CREATE);
                continue;
            }

            display_message(server_buffer, NEW_SESS, strlen(session_id), client_id, session_id);
            write(sockfd, server_buffer, strlen(server_buffer));
            /* read_bytes = read(sockfd, server_buffer, BUFFER_SIZE); */
        }
        else if (strcmp(command, "/list") == 0)
        {

        }
        else if (strcmp(command, "/quit") == 0)
        {

        }
    }
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        printf("usage: client\n");
        exit(0);
    }

    login();

    return 0;
}
