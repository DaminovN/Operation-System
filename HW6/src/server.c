#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <unistd.h>

#include "local.h"
#include "multiplex.h"

void print_time()
{
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char* time_buf = asctime(timeinfo);
    size_t len = strlen(time_buf);
    time_buf[len - 1] = '\0';
    printf("[%s] ", time_buf);
}

int main(int argc, char* argv[])
{
    unsigned int ip;

    if (argc > 1)
    {
        ip = 0;
        unsigned int pos = 0;
        for (size_t i = 0; i < 4; ++i)
        {
            unsigned int byte = 0;
            do
            {
                if (pos >= strlen(argv[1]) || argv[1][pos] < 48 || argv[1][pos] > 57)
                {
                    fprintf(stderr, "Wrong format of IPv4 address.\n");
                    exit(EXIT_FAILURE);
                }
                byte *= 10;
                byte += (argv[1][pos] - 48);
                ++pos;
            }
            while (pos < strlen(argv[1]) && argv[1][pos] != '.');
            ++pos;
            if (byte > 255)
            {
                fprintf(stderr, "Wrong format of IPv4 address.\n");
                exit(EXIT_FAILURE);
            }
            ip <<= 8;
            ip += byte;
        }
    }
    else
    {
        ip = INADDR_ANY;
    }

    int listener;
    struct sockaddr_in addr, accepted_addr;
    socklen_t sockaddr_len = sizeof(struct sockaddr_in);
    char buf[BUF_SIZE];
    int bytes_read;

    ASSIGN_HANDLE_ERROR(listener, socket(AF_INET, SOCK_STREAM, 0));
    print_time();
    printf("Listener created\n");
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(ip);
    HANDLE_ERROR(bind(listener, (struct sockaddr *)&addr, sizeof(addr)));
    print_time();
    printf("Listener bound to port: %u\n", SERVER_PORT);

    HANDLE_ERROR(set_nonblock(listener));

    listen(listener, 100);
    print_time();
    printf("Start to listen port: %u\n", SERVER_PORT);

    int client, mxfd, events_count;
    int read_fds[MULTIPLEX_SIZE];

    ASSIGN_HANDLE_ERROR(mxfd, multiplex());
    print_time();
    printf("MX created on fd: %d\n", mxfd);

    HANDLE_ERROR(mx_add(mxfd, listener));
    print_time();
    printf("Listener added to MX\n");

    while (1)
    {
        ASSIGN_HANDLE_ERROR(events_count, mx_wait(mxfd, read_fds, MULTIPLEX_SIZE));
        for (size_t i = 0; i < events_count; ++i)
        {
            if (read_fds[i] == listener)
            {
                ASSIGN_HANDLE_ERROR(client, 
                        accept(listener, (struct sockaddr *)&accepted_addr, &sockaddr_len));
                print_time();
                printf("Connection accepted from: %s:%d, client assigned to fd: %d\n", 
                        inet_ntoa(accepted_addr.sin_addr), 
                        ntohs(accepted_addr.sin_port), 
                        client);
                
                HANDLE_ERROR(set_nonblock(client));

                HANDLE_ERROR(mx_add(mxfd, client));
            }
            else
            {
                client = read_fds[i];
                
                print_time();
                printf("New event on client: %d\n", client);
                
                bytes_read = 0;
                while (1)
                {
                    bytes_read = recv(client, buf, BUF_SIZE, 0);
                    
                    if (bytes_read <= 0)
                    {
                        break;
                    }

                    send(client, buf, bytes_read, 0);

                    if (buf[bytes_read - 1] == '\n')
                    {
                        break;
                    }
                }

                if (bytes_read <= 0)
                {
                    //note: epoll and kqueue remove closed fd automatically
                    close(client);
                    print_time();
                    printf("%d disconnected\n", client);
                }
            }
        }
    }

    close(listener);
    close(mxfd);

    return 0;
}
