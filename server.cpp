#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h> //memset
#include <unistd.h> //read
#include <cstdio> //perror
#include <thread>
#include <chrono>
#include <iostream>

#define SERVER_PORT 12345

#define BUF_SIZE 4096
#define QUEUE_SIZE 10

void tserver(int sa) {
    char buf[BUF_SIZE];
    char ansbuf[BUF_SIZE];

    std::time_t last_access;
    bool has_last_access = false;
    while (1) {
        int bytes = read(sa, buf, BUF_SIZE-1);
        if (bytes == 0) break;
        buf[BUF_SIZE-1] = '\0'; // prevent buffer overflow
        buf[bytes] = '\0';

        if (strncmp(buf, "MyGet ", 6) == 0) {
            last_access = time(0);
            has_last_access = true;

            char* filepath = buf + 6;

            printf("%s", filepath);
            if (access(filepath, R_OK)) {
                perror("open failed");
                const char* openfail = "open failed";
                strcpy(ansbuf, openfail);
                write(sa, ansbuf, strlen(ansbuf)+1);
            } else {
                int fd = open(filepath, O_RDONLY);
                while (1) {
                    bytes = read(fd, ansbuf, BUF_SIZE);
                    if (bytes <= 0) break;
                    write(sa, ansbuf, bytes);
                }
            }
        } else if(strncmp(buf, "MyLastAccess", 12) == 0) {
            const char* lastaccessstr = "Last Access=";
            strcpy(ansbuf, lastaccessstr);
            if (has_last_access) {
                strcat(ansbuf,std::ctime(&last_access));
            } else {
                const char* nullstr = "Null";
                strcat(ansbuf, nullstr);
            }
            write(sa, ansbuf, strlen(ansbuf)+1);
        } else {
            strcpy(ansbuf, "bad request");
            write(sa, ansbuf, strlen(ansbuf)+1);
            fprintf(stderr, "bad request: %s", buf);
        }
    }
    close(sa);
}

int main(int argc, char *argv[]) {
    int s, b, l, fd, sa, bytes, on = 1;
    char buf[BUF_SIZE];
    fd = open(buf, O_RDONLY);
    struct sockaddr_in channel;

    memset(&channel, 0, sizeof(channel));
    channel.sin_family = AF_INET;
    channel.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.sin_port = htons(SERVER_PORT);

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0) perror("socket failed");
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));

    b = bind(s, (struct sockaddr *) &channel, sizeof(channel));
    if (b < 0) perror("bind failed");

    l = listen(s, QUEUE_SIZE);
    if (l < 0) perror("listen failed");

    while (1) {
        sa = accept(s, 0, 0);
        if (sa < 0) perror("accept failed");

        std::thread t(tserver, sa);
        t.detach();
    }

    return 0;
}
