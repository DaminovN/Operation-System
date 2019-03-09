#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <memory.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char **argv) {
    char message[256];
    char buf[sizeof(message)];
    int port;

    if (argc != 3) {
        printf("Wrong arguments: \"port, ip\" expected\n");
        exit(0);
    }

    int socket_descriptor;
    struct sockaddr_in addr;
    struct hostent *host;
    port = atoi(argv[1]);
    host = gethostbyname(argv[2]);
    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor < 0) {
        perror("socket");
        exit(0);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

//    снова эта штука с сервером тестовым
//    addr.sin_addr.s_addr = inet_addr(host);
//
     addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(socket_descriptor, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Could not connect");
        exit(0);
    }

    while (1) {
        printf("Введите сообщение серверу (Для выхода - \"exit\"): ");
        gets(message);
        if (!strcmp(message, "exit")) {
            printf("отправка сообщения о выходе на сервер...\n");
            send(socket_descriptor, message, sizeof(message), 0);
            close(socket_descriptor);
            return 0;
        }

        printf("отправка сообщения на сервер...\n");
        send(socket_descriptor, message, sizeof(message), 0);
        printf("Ожидание сообщения\n");
        recv(socket_descriptor, buf, sizeof(message), 0);
        printf("Получено сообщение: %s\n", buf);
    }
}