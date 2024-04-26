#include <iostream>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CONNECTIONS 5
#define BUFFER_LEN 256

int main()
{
    int master_socket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(8011);

    bind(
        master_socket,
        (struct sockaddr *) &sockaddr,
        sizeof sockaddr
    );

    listen(
        master_socket,
        MAX_CONNECTIONS
    );

    printf("Server started\n");

    while (true)
    {
        printf("accept started\n");
        int slave_socket = accept(
            master_socket,
            0, 
            0
        );

        char buffer[BUFFER_LEN] = {0};

        printf("recv started\n");
        recv(
            slave_socket,
            buffer,
            BUFFER_LEN,
            0
        );

        send(
            slave_socket,
            buffer,
            BUFFER_LEN,
            0
        );

        shutdown(slave_socket, SHUT_RDWR);
        close(slave_socket);
        printf("%s\n", buffer);
    }

    return 0;
}