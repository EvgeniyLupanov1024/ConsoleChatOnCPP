#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
#include <netinet/in.h>

#include <thread>

#define BUFFER_LEN 256
#define FIFO_SCREEN_NAME "fifo_message_screen"
#define APP_SCREEN_NAME "client_message_screen"

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

int openMessagesScreen()
{
    char fifo_path[256];
    sprintf(fifo_path, "./tmp/%s%d", FIFO_SCREEN_NAME, getpid());

    unlink(fifo_path);
    if (mkfifo(fifo_path, 0666) == -1) {
        fprintf(stderr, "Невозможно создать fifo канал: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int msg_scr_fd = open(fifo_path, O_RDWR);
    if(msg_scr_fd == -1) {
        fprintf(stderr, "Невозможно открыть fifo канал: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

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

    return msg_scr_fd;
}

void closeMessagesScreen(int msg_scr_fd)
{
    if (message_screen_pid == 0) {
        std::cerr << "Неверный pid процесса message screen\n";
    }
    kill(message_screen_pid, SIGTERM);

    close(msg_scr_fd);
    unlink(FIFO_SCREEN_NAME);
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
    int msg_scr_fd;
};

void * listenServer(void *args)
{
    int socket = ((ListenServerArgs *) args)->socket;
    int msg_scr_fd = ((ListenServerArgs *) args)->msg_scr_fd;

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

        write(msg_scr_fd, recv_buffer, recv_res);
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
    char buf[256];
    strcpy(buf, "-- ");
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
    connectToServer(server_socket, sockaddr);

    printfStatus("открытие окна чата");
    int msg_scr_fd = openMessagesScreen();

    ListenServerArgs listen_server_args = {server_socket, msg_scr_fd};
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
    closeMessagesScreen(msg_scr_fd);

    return 0;
}