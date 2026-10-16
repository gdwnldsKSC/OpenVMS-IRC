#include "modules/modules.h"
#include "modules/constants.h"
#include "irc.h"

static volatile sig_atomic_t gtfotime = 0;
pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned int saved_cli_ctrl = 0;
static unsigned short tty_chan = 0;
static unsigned int exit_status;
static unsigned int ctrl_y_ef;
static int ctrl_y_ef_allocated = 0;
static int cli_ctrl_disabled = 0;
static int exit_handler_registered = 0;
static int screen_active = 0;
static volatile sig_atomic_t send_failed = 0;

static void restore_cli_ctrl(unsigned int *status) {
    unsigned int mask = LIB$M_CLI_CTRLY;

    if (cli_ctrl_disabled) {
        if (saved_cli_ctrl & LIB$M_CLI_CTRLY) {
            lib$enable_ctrl(&mask, 0);
        }
        cli_ctrl_disabled = 0;
    }
}

static struct {
    unsigned int flink;
    void (*handler)(unsigned int *);
    unsigned int arg_count;
    unsigned int *status;
} exit_block = {0, restore_cli_ctrl, 1, &exit_status};

static void request_quit(void) {
    gtfotime = 1;
    if (tty_chan != 0) {
        sys$cancel(tty_chan);
    }
}

static void ctrly_ast(int unused);

static void ctrly_ast(int unused) {
    request_quit();
}

static void ctrl_y_cleanup(void) {
    unsigned int ast_status = sys$setast(0);

    if (tty_chan != 0) {
        sys$dassgn(tty_chan); // removes the one-shot Ctrl-Y AST.
        tty_chan = 0;
    }
    if (ctrl_y_ef_allocated) {
        lib$free_ef(&ctrl_y_ef);
        ctrl_y_ef_allocated = 0;
    }
    restore_cli_ctrl(&exit_status);
    if (exit_handler_registered) {
        sys$canexh(&exit_block);
        exit_handler_registered = 0;
    }
    if (ast_status == SS$_WASSET) {
        sys$setast(1);
    }
}

static unsigned int ctrl_y_init(void) {
    $DESCRIPTOR(tty_name, "SYS$COMMAND");
    unsigned int mask = LIB$M_CLI_CTRLY;
    unsigned int status;
    struct {
        unsigned short status;
        unsigned short count;
        unsigned int unused;
    } iosb;

    status = sys$assign(&tty_name, &tty_chan, 0, 0);
    if (!(status & 1)) return status;

    status = lib$get_ef(&ctrl_y_ef);
    if (!(status & 1)) goto fail;
    ctrl_y_ef_allocated = 1;

    status = sys$dclexh(&exit_block);
    if (!(status & 1)) goto fail;
    exit_handler_registered = 1;

    // Check both submission and completion before taking over from DCL.
    status = sys$qiow(ctrl_y_ef, tty_chan, IO$_SETMODE | IO$M_CTRLYAST,
                      &iosb, 0, 0, (void *)ctrly_ast, 0, PSL$C_USER, 0, 0, 0);
    if (!(status & 1)) goto fail;
    status = iosb.status;
    if (!(status & 1)) goto fail;

    status = lib$disable_ctrl(&mask, &saved_cli_ctrl);
    if (!(status & 1)) goto fail;
    cli_ctrl_disabled = 1;
    return SS$_NORMAL;

fail:
    ctrl_y_cleanup();
    return status;
}

static int wait_for_socket(int client_socket, int writing) {
    // ensure quit is always checked
    while (!gtfotime) {
        fd_set ready;
        struct timeval timeout;
        int status;

        FD_ZERO(&ready);
        FD_SET(client_socket, &ready);
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;
        status = select(client_socket + 1, writing ? NULL : &ready,
                        writing ? &ready : NULL, NULL, &timeout);
        if (gtfotime) return 0;
        if (status > 0) return 1;
        if (status < 0 && errno != EINTR) return -1;
    }
    return 0;
}

static int connect_to_server(int client_socket, struct sockaddr_in *address) {
    int nonblocking = 1;
    int error = 0;
    unsigned int error_length = sizeof(error);
    int status;

    if (gtfotime) return 0;
    if (ioctl(client_socket, FIONBIO, &nonblocking) < 0) return -1;
    status = connect(client_socket, (struct sockaddr *)address, sizeof(*address));
    if (status < 0) {
        if (errno != EINPROGRESS && errno != EWOULDBLOCK && errno != EINTR) {
            return gtfotime ? 0 : -1;
        }
        status = wait_for_socket(client_socket, 1);
        if (status <= 0) return status;
        if (getsockopt(client_socket, SOL_SOCKET, SO_ERROR,
                       &error, &error_length) < 0) return -1;
        if (error != 0) {
            errno = error;
            return -1;
        }
    }
    if (gtfotime) return 0;
    return 1;
}

int send_irc(int client_socket, const char *message, size_t length) {
    size_t offset = 0;
    int result;

    // Keep writes nonblocking even if ctrl-y hits before write posts
    pthread_mutex_lock(&send_mutex);
    while (offset < length && !gtfotime) {
        int ready = wait_for_socket(client_socket, 1);
        int sent;
        if (ready == 0) break;
        if (ready < 0) break;
        sent = write(client_socket, message + offset, length - offset);
        if (sent > 0) {
            offset += sent;
        } else if (sent < 0 && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)) {
            continue;
        } else {
            if (sent == 0) errno = EPIPE;
            break;
        }
    }
    if (gtfotime) {
        result = 0;
    } else if (offset == length) {
        result = 1;
    } else {
        perror("write");
        send_failed = 1;
        request_quit();
        result = -1;
    }
    pthread_mutex_unlock(&send_mutex);
    return result;
}

void output_wrapped(WINDOW *output, const char *text) {
    int row, column;

    for (; *text != '\0'; ++text) {
        if (*text == '\r' && text[1] == '\n') continue;
        getyx(output, row, column);
        // Leave the final column free to avoid curses' implicit edge handling.
        if (*text == '\n' || ((unsigned char)*text >= ' ' && column >= COLS - 1)) {
            waddch(output, '\n');
            // OpenVMS newline keeps the old column; reset it after advancing.
            getyx(output, row, column);
            wmove(output, row, 0);
            if (*text == '\n') continue;
        }
        waddch(output, *text);
    }
}

void process_incoming_message(int client_socket, char *message, WINDOW *output) {
    if (strncmp(message, "PING", 4) == 0) {
        char pong[BUFFER_SIZE];
        snprintf(pong, sizeof(pong), "PONG%s\r\n", message + 4);
        pthread_mutex_lock(&screen_mutex);
        output_wrapped(output, "Sending PONG response\n");
        wrefresh(output);
        pthread_mutex_unlock(&screen_mutex);
        send_irc(client_socket, pong, strlen(pong));
    } else if (strstr(message, " PRIVMSG ") != NULL) { // Check for PRIVMSG
        privmsg_handler(client_socket, message, output);
    } else {
        pthread_mutex_lock(&screen_mutex);
        output_wrapped(output, message);
        output_wrapped(output, "\n");
        wrefresh(output);
        pthread_mutex_unlock(&screen_mutex);
        // Display unhandled input. Useful to find out what to implement next and/or see
        // unhandled server messages and types!
    }
}

typedef struct {
    int client_socket;
    WINDOW *entry_window;
    char *channel;
} thread_data_t;

void *user_input_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int client_socket = data->client_socket;
    WINDOW *entry_window = data->entry_window;
    char *channel = data->channel;
    char input_message[BUFFER_SIZE];
    char send_buffer[BUFFER_SIZE];

    while (!gtfotime) {
        pthread_mutex_lock(&screen_mutex);
        wmove(entry_window, 1, 0);
        wclrtoeol(entry_window);
        wrefresh(entry_window);
        pthread_mutex_unlock(&screen_mutex);
        if (read_input_line(tty_chan, entry_window, input_message,
                            sizeof(input_message), &gtfotime) == ERR) {
            request_quit();
            break;
        }
        if (gtfotime) break;

        if (strlen(input_message) > 0) {
            if (strcmp(input_message, "/quit") == 0) { 
                request_quit();
                break;
            } else if (strncmp(input_message, "/msg", 4) == 0 &&
                       (input_message[4] == ' ' || input_message[4] == '\0')) {
                char *recipient = input_message + 4;
                char *message;

                while (*recipient == ' ') ++recipient;
                message = strchr(recipient, ' ');
                // Incomplete /msg commands must never be sent to the channel.
                if (message == NULL) continue;
                *message++ = '\0';
                while (*message == ' ') ++message;
                if (*message == '\0') continue;
                // Include the IRC prefix, separator, CRLF, and terminating NUL.
                if (strlen(recipient) + strlen(message) + 13 > sizeof(send_buffer)) continue;

                snprintf(send_buffer, sizeof(send_buffer), "PRIVMSG %s :%s\r\n",
                         recipient, message);
                send_irc(client_socket, send_buffer, strlen(send_buffer));
            } else {
                snprintf(send_buffer, BUFFER_SIZE, "PRIVMSG %s :%s\r\n", channel, input_message);
                send_irc(client_socket, send_buffer, strlen(send_buffer));
                memset(input_message, 0, BUFFER_SIZE); 
            }
        }
    }

    return NULL;
}

int process_initial_messages(int client_socket, WINDOW *output) {
    char buffer[BUFFER_SIZE];
    int registration_complete = 0;

    while (!registration_complete && !gtfotime) {
        int ready = wait_for_socket(client_socket, 0);
        if (ready < 0) return -1;
        if (ready == 0) break;
        int n = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (gtfotime) break;
        if (n < 0) {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) continue;
            return -1;
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

void endwin_noreturn(void) {
    if (screen_active) {
        endwin();
        screen_active = 0;
    }
}

int main(int argc, char **argv) {
    WINDOW *output_window;
    WINDOW *entry_window;
    struct hostent *server;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int client_socket = -1;
    int result = EXIT_SUCCESS;
    int input_thread_started = 0;
    int status;
    unsigned int vms_status;
    pthread_t input_thread;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s server\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Resolve before changing terminal or CLI state. it's a blocking call
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return EXIT_FAILURE;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(SERVER_PORT);

    vms_status = ctrl_y_init();
    if (!(vms_status & 1)) {
        fprintf(stderr, "Cannot install Ctrl-Y handler (status %08X)\n", vms_status);
        return EXIT_FAILURE;
    }

    initscr();
    screen_active = 1;
    clear();
    atexit(endwin_noreturn);

    output_window = newwin(LINES - 2, COLS, 0, 0);
    entry_window = newwin(2, COLS, LINES - 2, 0);

    scrollok(output_window, TRUE);
    wrapok(output_window, TRUE);

    char banner[BANNER_LENGTH];
    char banner_contents[512];
    extern char *BANNER_DEFINITION;
    extern char *BANNER_TOP;

    sprintf(banner_contents, BANNER_DEFINITION, "NICK", "*PLACEHOLDER*", "CHANNEL");
    sprintf(banner, BANNER_TOP, "placeholder", banner_contents);

    mvwaddstr(entry_window, 0, 0, banner);

    wrefresh(output_window);
    wrefresh(entry_window);

    if (gtfotime) goto cleanup;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("socket");
        result = EXIT_FAILURE;
        goto cleanup;
    }
    status = connect_to_server(client_socket, &server_addr);
    if (gtfotime || status == 0) goto cleanup;
    if (status < 0) {
        perror("connect");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    char nickname[BUFFER_SIZE];
    mvwaddstr(entry_window, 1, 0, "Enter your nickname: ");
    if (read_input_line(tty_chan, entry_window, nickname,
                        sizeof(nickname), &gtfotime) == ERR || gtfotime) goto cleanup;
    snprintf(buffer, BUFFER_SIZE, "NICK %s\r\n", nickname);
    send_irc(client_socket, buffer, strlen(buffer));

    sprintf(banner_contents, BANNER_DEFINITION, nickname, "*PLACEHOLDER*", "CHANNEL");
    sprintf(banner, BANNER_TOP, "placeholder", banner_contents);

    wclear(entry_window);
    mvwaddstr(entry_window, 0, 0, banner);

    wrefresh(entry_window);

    char username[BUFFER_SIZE];
    mvwaddstr(entry_window, 1, 0, "Enter your username: ");
    wrefresh(entry_window);
    if (read_input_line(tty_chan, entry_window, username,
                        sizeof(username), &gtfotime) == ERR || gtfotime) goto cleanup;
    snprintf(buffer, BUFFER_SIZE, "USER %s 0 * :%s\r\n", username, username);
    send_irc(client_socket, buffer, strlen(buffer));

    status = process_initial_messages(client_socket, output_window);
    if (gtfotime) goto cleanup;
    if (status != 1) {
        if (status < 0) perror("recv");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    wclear(entry_window);
    mvwaddstr(entry_window, 0, 0, banner);

    char channel[BUFFER_SIZE];
    mvwaddstr(entry_window, 1, 0, "Enter channel to join: ");
    wrefresh(entry_window);
    if (read_input_line(tty_chan, entry_window, channel,
                        sizeof(channel), &gtfotime) == ERR || gtfotime) goto cleanup;
    snprintf(buffer, BUFFER_SIZE, "JOIN %s\r\n", channel);
    send_irc(client_socket, buffer, strlen(buffer));

    memset(banner_contents, 0, strlen(banner_contents));
    memset(banner, 0, strlen(banner));
    sprintf(banner_contents, BANNER_DEFINITION, nickname, "*PLACEHOLDER*", channel);
    sprintf(banner, BANNER_TOP, "placeholder", banner_contents);
    wclear(entry_window);
    mvwaddstr(entry_window, 0, 0, banner);

    wrefresh(entry_window);

    thread_data_t thread_data = {client_socket, entry_window, channel};
    if (gtfotime) goto cleanup;
    status = pthread_create(&input_thread, NULL, user_input_thread, &thread_data);
    if (status != 0) {
        fprintf(stderr, "Cannot start input thread (%d)\n", status);
        result = EXIT_FAILURE;
        goto cleanup;
    }
    input_thread_started = 1;


    while (!gtfotime) {
        status = wait_for_socket(client_socket, 0);
        if (status == 0) break;
        if (status < 0) {
            perror("select");
            result = EXIT_FAILURE;
            break;
        }
        int n = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (gtfotime) break;
        if (n < 0) {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) continue;
            perror("recv");
            result = EXIT_FAILURE;
            break;
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

cleanup:
    request_quit();
    if (input_thread_started) {
        pthread_join(input_thread, NULL);
    }
    if (client_socket >= 0) close(client_socket);
    endwin_noreturn();
    ctrl_y_cleanup();
    return send_failed ? EXIT_FAILURE : result;
}
