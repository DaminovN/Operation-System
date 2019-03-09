#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    int socket_descriptor, port_number, sock, message_size;
    struct sockaddr_in serv_addr;
    struct hostent *host;

    char buffer[256];
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    // Задаем номер порта
    port_number = atoi(argv[1]);

    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor < 0) {
        fprintf(stderr, "Error - could not open socket");
        exit(0);
    }

    // Конвертирует имя хоста в IP адрес
    host = gethostbyname(argv[2]);
    if (host == NULL) {
        fprintf(stderr, "Error - no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    /*
    // Указаваем IP адрес сокета
    bcopy((char *) server->h_addr,
          (char *) &serv_addr.sin_addr.s_addr,
          server->h_length);
    // Но у нас тестовое соединение на 127.0.0.1 поэтому строка следующая
     */
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    serv_addr.sin_port = htons(port_number);

    if (bind(socket_descriptor, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Error with connection");
    }

    listen(socket_descriptor, 1);

    int status = 0;
    while (status != -1) {
        sock = accept(socket_descriptor, NULL, NULL);
        if (sock < 0) {
            perror("Something went wrong with connection");
            exit(0);
        }
        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork error");
            break;
        }
        if (pid == 0) {
            close(socket_descriptor);
            while (status != -1) {
                printf("Ожидаем сообщение...\n");

                message_size = recv(sock, buffer, 256, 0); // принимаем сообщение от клиента
                if (strcmp(buffer, "close") == 0) {
                    printf("Закрываем сокет\n");
                    close(sock);
                    status = -1;
                    break;
                }
                printf("Получено Сообщение: %s\n", buffer);
                printf("Отправляю принятое сообщение клиенту\n");
                send(sock, buffer, message_size, 0); // отправляем принятое сообщение клиенту
            }
        }
        if (pid > 0) {
            waitpid(pid, &status, 0);
            status = -1;
        }
        close(sock);
    }
    return 0;
}