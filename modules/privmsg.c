#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "modules.h"

void privmsg_handler(int client_socket, char *message, WINDOW *output) {
    char *ctcp_start = strchr(message, '\x01');
    if (ctcp_start != NULL) { // Check for CTCP messages
        ctcp_handler(client_socket, message, output);
    } else {
        // Handle regular private or channel messages
        waddstr(output, message);
        waddstr(output, "\n");
        wrefresh(output);
    }
}
