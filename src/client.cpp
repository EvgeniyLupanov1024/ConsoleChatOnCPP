#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <thread>

#define BUFFER_LEN 256
#define FIFO_SCREEN_NAME "fifo_client_msg"
#define APP_SCREEN_NAME "client_messages_screen"

struct ListenServerArgs
{
    int socket;
};

void * listenServer(void * args)
{
    unlink(FIFO_SCREEN_NAME);
    if (mkfifo(FIFO_SCREEN_NAME, 0666) == -1) {
        fprintf(stderr, "Невозможно создать fifo\n");
        exit(EXIT_FAILURE);
    }

    int msg_print_fd = open(FIFO_SCREEN_NAME, O_RDWR);
    if(msg_print_fd == - 1) {
        fprintf(stderr, "Невозможно открыть fifo\n");
        exit(EXIT_FAILURE);

    }

    system("gnome-terminal -- bash -c \"./client_messages_screen\" ");

    int socket = ((ListenServerArgs *) args)->socket;
    char recv_buffer[BUFFER_LEN] = {0};
    while(true)
    {
        int recv_res = recv(
            socket, 
            recv_buffer, 
            BUFFER_LEN, 
            0
        );

        if (recv_res == 0) {
            fprintf(stderr, "server is gone ._.\n");
            break;
        }

        write(msg_print_fd, recv_buffer, recv_res); // отображение
    }

    close(msg_print_fd);
    return NULL;
}

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

    printf("=== подключение к серверу\n");
    while (true)
    {
        int connect_res = connect(
            client_socket,
            (struct sockaddr *) &sockaddr,
            sizeof sockaddr
        );

        if (connect_res == 0) {
            break;
        } else if (errno == EAGAIN) {
            continue;
        } 
        
        switch (errno)
        {
            case ENETUNREACH:
                fprintf(stderr, "Нет сети");
                break;

            case ECONNREFUSED:
                fprintf(stderr, "Нет сервера по указанному адресу");
                break;

            case ETIMEDOUT:
                fprintf(stderr, "Время ожидания подключения к серверу вышло");
                break;
            
            default:
                fprintf(stderr, "Ошибка подключения к серверу");
        }
        exit(EXIT_FAILURE);
    }

    printf("=== чтение сообщений сервера\n");
    struct ListenServerArgs listen_server_args = {client_socket};
    std::thread listen_server_thread(listenServer, &listen_server_args); // чтение
    listen_server_thread.detach();

    printf("=== отправка сообщений на сервер\n");
    char send_buffer[BUFFER_LEN] = {0};
    while(true)
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