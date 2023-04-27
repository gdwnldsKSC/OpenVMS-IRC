#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_ADDRESS "IRC.SERVER.IP.ADDRESS.HERE"
#define SERVER_PORT 6667

int main(int argc, char **argv) {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }
    if (connect(client_socket, (struct sockaddr *)&server_addr,
sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    char nick[] = "IRCNICKHERE";
    char user[] = "IDENTUSERNAMEHERE";
    char join[] = "JOIN #CHANNELNAMEHERE\r\n";
    char buf[1024];
    int len = sprintf(buf, "NICK %s\r\nUSER %s 0 * :%s\r\n%s", nick, user,
user, join);
    send(client_socket, buf, len, 0);

    while (1) {
        int n = recv(client_socket, buf, sizeof(buf) - 1, 0);
        if (n < 0) {
            perror("recv");
            exit(1);
        } else if (n == 0) {
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);
        fflush(stdout);
    }

    close(client_socket);
	
    return 0;
}
