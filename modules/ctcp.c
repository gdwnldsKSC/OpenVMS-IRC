#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "modules.h"

void ctcp_handler(int client_socket, char *message, WINDOW *output) {
    if (strncmp(message, ":\x01VERSION", 9) == 0) {
        char version_response[BUFFER_SIZE];
        snprintf(version_response, sizeof(version_response), "NOTICE %s :\x01VERSION VMS-IRC:%s\x01\r\n", message + 1, "Version 0.00.1");
        waddstr(output, "Sending CTCP VERSION response\n");
        wrefresh(output);
        write(client_socket, version_response, strlen(version_response));
    }
}