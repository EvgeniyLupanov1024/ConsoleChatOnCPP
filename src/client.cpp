#include <iostream>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_LEN 256

int main()
{
    int client_socket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(8011);
    sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    connect(
        client_socket,
        (struct sockaddr *) &sockaddr,
        sizeof sockaddr
    );

    if (fork() == 0) {
        while(true) // чтение
        {
            char recv_buffer[BUFFER_LEN] = {0};
            int recv_res = recv(
                client_socket, 
                recv_buffer, 
                BUFFER_LEN, 
                0
            );

            // if (recv_res == 0 && errno != EAGAIN) {
            //     shutdown(client_socket, SHUT_RDWR);
            //     close(client_socket);
            //     exit(0);
            // }

            printf("%s\n", recv_buffer);
        }
    }

    printf("Введите сообщение: ");
    char send_buffer[BUFFER_LEN] = {0};

    while(true) // отправка
    {
        scanf("%s", send_buffer);
        send(
            client_socket, 
            send_buffer, 
            BUFFER_LEN, 
            0
        );
    }  

    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);

    return 0;
}