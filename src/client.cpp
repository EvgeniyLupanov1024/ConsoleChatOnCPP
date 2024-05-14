#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>

#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include "fifo_channel.hpp"

#define BUFFER_LEN 256
#define FIFO_SCREEN_PATH "./tmp/fifo_message_screen"
#define APP_SCREEN_NAME "client_message_screen"

FifoChannel *message_screen_channel;

bool close_client = false;
void closeClientHandler(int signum) 
{
    close_client = true;
}

pid_t message_screen_pid = -1;
void setMessageScreenPid(int signum, siginfo_t *siginfo, void *ptr)  
{
    message_screen_pid = siginfo->si_pid;
}

void openMessagesScreen()
{
    message_screen_channel = new FifoChannel(FIFO_SCREEN_PATH, getpid());

    struct sigaction set_message_screen_pid_action = {0};
    set_message_screen_pid_action.sa_sigaction = setMessageScreenPid;
    set_message_screen_pid_action.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGUSR1, &set_message_screen_pid_action, NULL);

    char command_str[256];
    sprintf(command_str, "gnome-terminal -- bash -c \"./%s %d\"", APP_SCREEN_NAME, getpid());
    system(command_str);
    while (message_screen_pid == -1) 
    {
        pause();
    }
}

void closeMessagesScreen()
{
    if (message_screen_pid == 0) {
        std::cerr << "Неверный pid процесса message screen\n";
    }
    kill(message_screen_pid, SIGTERM);

    delete message_screen_channel;
}

void connectToServer(int socket, sockaddr_in sockaddr)
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
        
        fprintf(stderr, "Ошибка подключения к серверу: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

struct ListenServerArgs
{
    int socket;
};

void * listenServer(void *args)
{
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

        if (close_client) {
            break;
        }

        if (recv_res == 0) {
            fprintf(stderr, "server is gone ._.\n");
            kill(getpid(), SIGTERM);
            break;
        }

        message_screen_channel->writeIn(recv_buffer, recv_res);
    }

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
        
        if (close_client) {
            break;
        }

        if (strcmp(send_buffer, "exit") == 0) {
            kill(getpid(), SIGTERM);
            break;
        }

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
    std::ostringstream buffer;
    buffer << "-- " << status_text << "...\n";
    std::string status_line = buffer.str();
    
    va_list args;
    printf(status_line.c_str(), args);
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
    connectToServer(server_socket, sockaddr);

    printfStatus("открытие окна чата");
    openMessagesScreen();

    ListenServerArgs listen_server_args = {server_socket};
    std::thread listen_server_thread(listenServer, &listen_server_args);

    std::cout << "Введите имя: ";
    WriteToServerArgs write_to_server_args = {server_socket};
    std::thread write_to_server_thread(writeToServer, &write_to_server_args);
    write_to_server_thread.detach();

    struct sigaction close_client_action = {0};
    close_client_action.sa_handler = closeClientHandler;
    sigaction(SIGTERM, &close_client_action, NULL);
    sigaction(SIGINT, &close_client_action, NULL);

    while(true) 
    {
        pause();
        if (close_client) {
            break;
        }
    }

    printfStatus("закрытие клиента");
    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);
    listen_server_thread.join();
    closeMessagesScreen();

    return 0;
}