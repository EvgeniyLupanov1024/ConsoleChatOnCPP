#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <thread>

#define BUFFER_LEN 256
#define FIFO_SCREEN_NAME "fifo_client_msg"
#define APP_SCREEN_NAME "client_messages_screen"

void connectToSrver(int socket, sockaddr_in sockaddr)
{
    while (true)
    {
        int connect_res = connect(
            socket,
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
}

int openMessagesScreen()
{
    unlink(FIFO_SCREEN_NAME);
    if (mkfifo(FIFO_SCREEN_NAME, 0666) == -1) {
        fprintf(stderr, "Невозможно создать fifo\n");
        exit(EXIT_FAILURE);
    }

    int msg_scr_fd = open(FIFO_SCREEN_NAME, O_RDWR);
    if(msg_scr_fd == -1) {
        fprintf(stderr, "Невозможно открыть fifo\n");
        exit(EXIT_FAILURE);
    }

    system("gnome-terminal -- bash -c \"./client_messages_screen\" ");

    return msg_scr_fd;
}

int closeMessagesScreen(int msg_scr_fd)
{
    return close(msg_scr_fd);
}

int writeMessagesScreen(int msg_scr_fd, char *buf, int n)
{
    return write(msg_scr_fd, buf, n);
}

struct ListenServerArgs
{
    int socket;
};

void * listenServer(void *args)
{
    int msg_scr_fd = openMessagesScreen();

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

        writeMessagesScreen(msg_scr_fd, recv_buffer, recv_res);
    }

    closeMessagesScreen(msg_scr_fd);
    return NULL;
}

struct WriteToServerArgs
{
    int socket;
};

void * writeToServer(void *args)
{
    int socket = ((WriteToServerArgs *) args)->socket;

    char send_buffer[BUFFER_LEN] = {0};
    while(true)
    {
        fgets(send_buffer, BUFFER_LEN, stdin);
        int message_len = strlen(send_buffer);
        send_buffer[message_len - 1] = '\0';

        send(
            socket, 
            send_buffer, 
            message_len, 
            0
        );
    } 

    return NULL;
}

void printfStatus(const char *status_text, ...)
{
    char buf[256];
    strcpy(buf, "=== ");
    strcat(buf, status_text);
    strcat(buf, "...\n");
    va_list args;

    printf(buf, args);
}

int main()
{
    int server_socket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );
    
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(8011);
    sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    printfStatus("подключение к серверу");
    connectToSrver(server_socket, sockaddr);

    printfStatus("чтение сообщений с сервера");
    ListenServerArgs listen_server_args = {server_socket};
    std::thread listen_server_thread(listenServer, &listen_server_args);

    printfStatus("отправка сообщений на сервер");
    WriteToServerArgs write_to_server_args = {server_socket};
    std::thread write_to_server_thread(writeToServer, &write_to_server_args);

    write_to_server_thread.join();
    listen_server_thread.join();

    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);

    return 0;
}