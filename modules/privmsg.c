#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"
#include "modules.h"

void privmsg_handler(int client_socket, char *message, WINDOW *output) {
    char sender_nick[BUFFER_SIZE];
    char *sender_end = strchr(message, '!');
    if (sender_end == NULL) {
        return; // bail if no ! in message, if not nick!user@host invalid msg
    }
    strncpy(sender_nick, message + 1, sender_end - message - 1);
    sender_nick[sender_end - message - 1] = '\0';

    char *msg_start = strstr(message, " PRIVMSG ");
    if (msg_start == NULL) {
        return; // error handling - this isn't a privmsg, how did we get here?
    }
    msg_start += 9; //" PRIVMSG " - we want whats after this.

    char *msg_stuff_start = strchr(msg_start, ':');
    if (msg_stuff_start == NULL) {
        return; // error handling, probably not necessary, but if there's no : then there's nothing to do.....
    }
    msg_stuff_start++; 

    char *ctcp_start = strchr(msg_stuff_start, '\x01');
    if (ctcp_start != NULL) { // Check for CTCP messages
        ctcp_handler(client_socket, sender_nick, ctcp_start, output);
    } else {
        // Handle regular private or channel messages
        waddstr(output, message);
        waddstr(output, "\n");
        wrefresh(output);
    }
}
