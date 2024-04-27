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

    char send_buffer[BUFFER_LEN] = {0};
    printf("Введите сообщение: ");

    while(true) 
    {
        scanf("%s", send_buffer);

        send(
            client_socket, 
            send_buffer, 
            BUFFER_LEN, 
            0
        );

        char recv_buffer[BUFFER_LEN] = {0};
        recv(
            client_socket, 
            recv_buffer, 
            BUFFER_LEN, 
            0
        );

        printf("%s\n", recv_buffer);
    }

    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);

    return 0;
}