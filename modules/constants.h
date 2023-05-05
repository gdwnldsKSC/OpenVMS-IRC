#define SERVER_PORT 6667
#define BUFFER_SIZE 1024
#define MESSAGE_SIZE 4096
#define MESSAGE_BUFFER_SIZE 3
#define BANNER_LENGTH 200
#define VERSION_MAJOR 0
#define VERSION_MINOR 01
#define VERSION_BUILD 1
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define VERSION_STRING STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_BUILD)
#define CLIENT_NAME "OpenVMS-IRC"