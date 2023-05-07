// General housekeeping header
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
#include <pthread.h>
#include <stdbool.h>
#include <ssdef.h>
#include <lib$routines.h>
#include <processes.h>
#include <stsdef.h>
#include <starlet.h>
#include <descrip.h>
#include <clidef.h>

// VAX (and pre 7.3-2 Alpha) don't have snprintf, so we bring one with us
// in snprintf_vms.c - right now we only handle VAX breakage
#ifdef __VAX
#define snprintf snprintf_vms
int snprintf_vms(char *str, size_t len, const char *fmt, ...);
#endif

char *BANNER_DEFINITION = "%s   %s %s ***";
char *BANNER_TOP = " *** stuff %s more stuff: %-42s";