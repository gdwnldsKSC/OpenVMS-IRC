// different modules/functions used for various things
#include <curses.h>
#include <signal.h>
#include <stddef.h>
#include <pthread.h>

extern pthread_mutex_t screen_mutex;
int send_irc(int client_socket, const char *message, size_t length);
void ctcp_handler(int client_socket, const char *sender_nick, const char *ctcp_message, WINDOW *output);
void privmsg_handler(int client_socket, char *message, WINDOW *output);
int read_input_line(unsigned short terminal_channel, WINDOW *window,
                    char *buffer, unsigned int capacity,
                    volatile sig_atomic_t *quit_requested);
