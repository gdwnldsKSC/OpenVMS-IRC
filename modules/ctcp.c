#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "modules.h"

void ctcp_handler(int client_socket, const char *sender_nick, const char *ctcp_message, WINDOW *output) {
    if (strncmp(ctcp_message, "\x01VERSION\x01", 9) == 0) {
        char version_response[BUFFER_SIZE];
        snprintf(version_response, sizeof(version_response), "NOTICE %s :\x01VERSION %s:%s\x01\r\n", sender_nick, CLIENT_NAME, VERSION_STRING);
        waddstr(output, version_response);
        waddstr(output, "Sending CTCP VERSION response\n");
        wrefresh(output);
        write(client_socket, version_response, strlen(version_response));
    }
}