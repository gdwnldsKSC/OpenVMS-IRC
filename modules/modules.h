// different modules/functions used for various things
void ctcp_handler(int client_socket, const char *sender_nick, const char *ctcp_message, WINDOW *output);
void privmsg_handler(int client_socket, char *message, WINDOW *output);