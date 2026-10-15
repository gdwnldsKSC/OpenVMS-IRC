#include <curses.h>
#include <ctype.h>
#include <lib$routines.h>
#include <starlet.h>
#include <iodef.h>
#include "modules.h"

/* The entry window spans COLS; keep long input on its final line. */
static void show_input_line(WINDOW *window, const char *buffer,
                            unsigned int length, int row, int column) {
    unsigned int visible = COLS > column + 1 ? COLS - column - 1 : 0;
    unsigned int first = length > visible ? length - visible : 0;
    unsigned int index;

    pthread_mutex_lock(&screen_mutex);
    wmove(window, row, column);
    wclrtoeol(window);
    for (index = first; index < length; ++index) {
        waddch(window, (unsigned char)buffer[index]);
    }
    wrefresh(window);
    pthread_mutex_unlock(&screen_mutex);
}

int read_input_line(unsigned short terminal_channel, WINDOW *window,
                    char *buffer, unsigned int capacity,
                    volatile sig_atomic_t *quit_requested) {
    unsigned int event_flag;
    unsigned int status;
    unsigned int length = 0;
    unsigned int no_terminators[2] = {0, 0};
    unsigned short iosb[4];
    unsigned char character;
    int row, column;
    int result = ERR;

    if (capacity == 0) {
        return ERR;
    }
    buffer[0] = '\0';
    if (*quit_requested || !(lib$get_ef(&event_flag) & 1)) {
        return ERR;
    }
    pthread_mutex_lock(&screen_mutex);
    getyx(window, row, column);
    wrefresh(window);
    pthread_mutex_unlock(&screen_mutex);

    while (!*quit_requested) {
        iosb[0] = iosb[1] = iosb[2] = iosb[3] = 0;
        /* No terminators makes every character, including CR, input data.
         * NOFILTR passes editing keys through without disabling Ctrl-Y ASTs. */
        status = sys$qio(event_flag, terminal_channel,
                         IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR,
                         iosb, 0, 0, &character, 1, 0,
                         no_terminators, 0, 0);
        if (!(status & 1)) {
            break;
        }

        /* A quit can arrive before QIO has queued the read. Cancel again
         * after queueing so that this case cannot leave SYNCH blocked. */
        if (*quit_requested) {
            sys$cancel(terminal_channel);
        }
        status = sys$synch(event_flag, iosb);
        if (!(status & 1)) {
            sys$cancel(terminal_channel);
            sys$synch(event_flag, iosb);
            break;
        }
        if (*quit_requested || !(iosb[0] & 1)) {
            break;
        }
        if (iosb[1] == 0) {
            continue;
        }

        if (character == '\r' || character == '\n') {
            result = OK;
            break;
        }
        if (character == 26) { /* Ctrl-Z: end of input. */
            break;
        }
        if (character == '\b' || character == 127) {
            if (length != 0) {
                buffer[--length] = '\0';
            }
        } else if (character == 21) { /* Ctrl-U: clear this input line. */
            length = 0;
            buffer[0] = '\0';
        } else if (isprint(character) && length < capacity - 1) {
            buffer[length++] = character;
            buffer[length] = '\0';
        } else {
            continue;
        }
        show_input_line(window, buffer, length, row, column);
    }

    lib$free_ef(&event_flag);
    if (*quit_requested || result == ERR) {
        buffer[0] = '\0';
        return ERR;
    }
    return result;
}
