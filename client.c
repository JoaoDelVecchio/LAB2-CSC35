#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 12345
#define BUF_SIZE 4096

int main(int argc, char **argv)
{
    int s, c, bytes;
    char buf[BUF_SIZE];
    struct hostent *h;
    struct sockaddr_in channel;

    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <server-name>\n", argv[0]);
        exit(1);
    }

    // ==============================
    h = gethostbyname(argv[1]);
    if (!h)
    {
        perror("gethostbyname");
        exit(1);
    }
    // ==============================
    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0)
    {
        perror("socket");
        exit(1);
    }
    // ==============================
    memset(&channel, 0, sizeof(channel));
    channel.sin_family = AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port = htons(SERVER_PORT);

    // ==============================
    c = connect(s, (struct sockaddr *)&channel, sizeof(channel));
    if (c < 0)
    {
        perror("Falhou a conexao");
        exit(1);
    }

    // ==============================
    while (1)
    {
        if (!fgets(buf, BUF_SIZE, stdin))
            break;

        buf[strcspn(buf, "\n")] = '\0';

        if (strcmp(buf, "quit") == 0)
        {
            break;
        }

        if (write(s, buf, strlen(buf) + 1) < 0)
        {
            perror("write");
            break;
        }

        bytes = read(s, buf, BUF_SIZE);
        if (bytes <= 0)
        {
            printf("Fim\n");
            break;
        }

        write(1, buf, bytes);
        printf("\n");
    }

    close(s);
    return 0;
}
