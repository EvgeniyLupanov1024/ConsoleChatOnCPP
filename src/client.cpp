#include <iostream>
#include <unistd.h>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <thread>

#define BUFFER_LEN 256

struct ListenServerArgs
{
    int socket;
};

void * listenServer(void * args)
{
    int socket = ((ListenServerArgs *) args)->socket;
    char recv_buffer[BUFFER_LEN] = {0};
        
    while(true) // чтение
    {
        int recv_res = recv(
            socket, 
            recv_buffer, 
            BUFFER_LEN, 
            0
        );

        if (recv_res == 0) {
            printf("server is gone ._.\n");
            break;
        }

        printf("%s\n", recv_buffer);
    }

    return NULL;
}

int main()
{
    // system("gnome-terminal -- bash -c \"./server\" ");

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

    struct ListenServerArgs listen_server_args = {client_socket};
    std::thread listen_server_thread(listenServer, &listen_server_args);
    listen_server_thread.detach();

    printf("Введите сообщение: ");
    char send_buffer[BUFFER_LEN] = {0};

    while(true) // отправка
    {
        fgets(send_buffer, BUFFER_LEN, stdin);
        int message_len = strlen(send_buffer);
        send_buffer[message_len - 1] = '\0';

        send(
            client_socket, 
            send_buffer, 
            message_len, 
            0
        );
    } 

    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);

    return 0;
}