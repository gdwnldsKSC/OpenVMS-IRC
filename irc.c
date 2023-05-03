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
#include "modules/constants.h"
#include "irc.h"

void process_incoming_message(int client_socket, char *message, WINDOW *output) {
    if (strncmp(message, "PING", 4) == 0) {
        char pong[BUFFER_SIZE];
        snprintf(pong, sizeof(pong), "PONG%s\r\n", message + 4);
        waddstr(output, "Sending PONG response\n");
        wrefresh(output);
        write(client_socket, pong, strlen(pong));
    } else if (strstr(message, " PRIVMSG ") != NULL) { // Check for PRIVMSG
        privmsg_handler(client_socket, message, output);
    } else {
        waddstr(output, message);
        waddstr(output, "\n");
        wrefresh(output);
        // Display unhandled input. Useful to find out what to implement next and/or see
        // unhandled server messages and types!
    }
}

int process_initial_messages(int client_socket, WINDOW *output) {
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
            process_incoming_message(client_socket, message_start, output);

            if (strstr(message_start, " 001 ") != NULL) { // Welcome message received
                registration_complete = 1;
            }

            message_start = message_end + 2;
        }
    }

    return registration_complete;
}

int main(int argc, char **argv) {
    WINDOW *output_window;
    WINDOW *entry_window;

    initscr();
    clear();

    output_window = newwin(LINES - 2, COLS, 0, 0);
    entry_window = newwin(2, COLS, LINES - 2, 0);

    scrollok(output_window, TRUE);
    wrapok(output_window, TRUE);

    char banner[BANNER_LENGTH];
    char banner_contents[512];
    extern char *BANNER_DEFINITION;
    extern char *BANNER_TOP;

    sprintf(banner_contents, BANNER_DEFINITION, "NICK PLACEHOLDER", "PLACEHOLDER 2", "PLACEHOLDER 3");
    sprintf(banner, BANNER_TOP, "placeholder", banner_contents);

    mvwaddstr(entry_window, 0, 0, banner);

    wrefresh(output_window);
    wrefresh(entry_window);

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
    mvwaddstr(entry_window, 1, 0, "Enter your nickname: ");
    wgetstr(entry_window, nickname);
    snprintf(buffer, BUFFER_SIZE, "NICK %s\r\n", nickname);
    write(client_socket, buffer, strlen(buffer));

    sprintf(banner_contents, BANNER_DEFINITION, nickname, "PLACEHOLDER 2", "PLACEHOLDER 3");
    sprintf(banner, BANNER_TOP, "placeholder", banner_contents);

    wclear(entry_window);
    mvwaddstr(entry_window, 0, 0, banner);

    wrefresh(entry_window);

    char username[BUFFER_SIZE];
    mvwaddstr(entry_window, 1, 0, "Enter your username: ");
    wrefresh(entry_window);
    wgetstr(entry_window, username);
    snprintf(buffer, BUFFER_SIZE, "USER %s 0 * :%s\r\n", username, username);
    write(client_socket, buffer, strlen(buffer)); 

    process_initial_messages(client_socket, output_window);

    wclear(entry_window);
    mvwaddstr(entry_window, 0, 0, banner);

    char channel[BUFFER_SIZE];
    mvwaddstr(entry_window, 1, 0, "Enter channel to join: ");
    wrefresh(entry_window);
    wgetstr(entry_window, channel);
    snprintf(buffer, BUFFER_SIZE, "JOIN %s\r\n", channel);
    write(client_socket, buffer, strlen(buffer));

    memset(banner_contents, 0, strlen(banner_contents));
    memset(banner, 0, strlen(banner));
    sprintf(banner_contents, BANNER_DEFINITION, nickname, "PLACEHOLDER 2", channel);
    sprintf(banner, BANNER_TOP, "placeholder", banner_contents);
    wclear(entry_window);
    mvwaddstr(entry_window, 0, 0, banner);

    wrefresh(entry_window);


    while (1) {
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
            process_incoming_message(client_socket, message_start, output_window);
            message_start = message_end + 2;
        }

        fflush(stdout);
    }


    close(client_socket);
	
    return 0;
}
