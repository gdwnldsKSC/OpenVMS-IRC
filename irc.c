#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include "modules/modules.h"
#include "irc.h"

#define SERVER_PORT 6667
#define BUFFER_SIZE 1024
#define MESSAGE_SIZE 4096
#define MESSAGE_BUFFER_SIZE 3

void process_incoming_message(int client_socket, char *message) {
    printf("Received message: %s\n", message);

    if (strncmp(message, "PING", 4) == 0) {
        char pong[BUFFER_SIZE];
        snprintf(pong, sizeof(pong), "PONG%s\r\n", message + 4);
        printf("Sending PONG response: %s\n", pong);
        write(client_socket, pong, strlen(pong));
    } else {
        printf("Message is not a PING event\n");
    }
}

int process_initial_messages(int client_socket) {
    char buffer[BUFFER_SIZE];
    int registration_complete = 0;

    while (!registration_complete) {
        int n = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (n < 0) {
            perror("recv");
            exit(1);
        } else if (n == 0) {
            break;
        }
        buffer[n] = '\0';

        char *message_start = buffer;
        char *message_end;

        while ((message_end = strstr(message_start, "\r\n")) != NULL) {
            *message_end = '\0';
            process_incoming_message(client_socket, message_start);

            if (strstr(message_start, " 001 ") != NULL) { // Welcome message received
                registration_complete = 1;
                printf("registration success!");
            }

            message_start = message_end + 2;
        }
    }

    return registration_complete;
}

int main(int argc, char **argv) {
	struct hostent *server;
	struct sockaddr_in server_addr;
	char buffer[BUFFER_SIZE];
	
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("socket");
        exit(1);
    }
	
	server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

	memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(SERVER_PORT);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    char nickname[BUFFER_SIZE];
    printf("Enter your nickname: ");
    scanf("%s", nickname);
    snprintf(buffer, BUFFER_SIZE, "NICK %s\r\n", nickname);
    write(client_socket, buffer, strlen(buffer));

    char username[BUFFER_SIZE];
    printf("Enter your username: ");
    scanf("%s", username);
    snprintf(buffer, BUFFER_SIZE, "USER %s 0 * :%s\r\n", username, username);
    write(client_socket, buffer, strlen(buffer)); 

    usleep(50);

    process_initial_messages(client_socket);

    char channel[BUFFER_SIZE];
    printf("Enter channel to join: ");
    scanf("%s", channel);
    snprintf(buffer, BUFFER_SIZE, "JOIN %s\r\n", channel);
    write(client_socket, buffer, strlen(buffer));


    while (1) {
        int n = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (n < 0) {
            perror("recv");
            exit(1);
        } else if (n == 0) {
            break;
        }
        buffer[n] = '\0';
        printf("Received data: %s\n", buffer);

        char *message_start = buffer;
        char *message_end;

        while ((message_end = strstr(message_start, "\r\n")) != NULL) {
            *message_end = '\0';
            process_incoming_message(client_socket, message_start);
            printf("%s\n", message_start);
            message_start = message_end + 2;
        }

        fflush(stdout);
    }


    close(client_socket);
	
    return 0;
}
