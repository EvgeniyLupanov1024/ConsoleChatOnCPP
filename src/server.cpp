#include <iostream>
#include <unistd.h>
#include <set>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <signal.h>

#define MAX_CONNECTIONS 5
#define BUFFER_LEN 256
#define MAX_EPOOL_EVENTS 32

void printfStatus(const char *status_text, ...)
{
    char buf[256];
    strcpy(buf, "-- ");
    strcat(buf, status_text);
    strcat(buf, "...\n");
    va_list args;

    printf(buf, args);
}

int master_socket;
std::set<int> slave_sockets;
int epoll_fd;
void closeServer()
{
    std::cout << "завершение работы сервера\n";
    close(epoll_fd);

    for (int slave_socket : slave_sockets) {
        shutdown(slave_socket, SHUT_RDWR);
        close(slave_socket);
    }
    shutdown(master_socket, SHUT_RDWR);
    close(master_socket);

    exit(EXIT_SUCCESS);
}

void closeServerHandler(int signum)
{
    closeServer();
}

void startServer()
{
    master_socket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(8011);

    int bind_res = bind(
        master_socket,
        (struct sockaddr *) &sockaddr,
        sizeof sockaddr
    );
    if (bind_res == -1) {
        fprintf(stderr, "Ошибка привязки сокета");
    }

    int listen_res = listen(
        master_socket,
        MAX_CONNECTIONS
    );
    if (listen_res == -1) {
        fprintf(stderr, "Ошибка при попытке слушать сокет");
    }

    epoll_fd = epoll_create1(0);

    struct epoll_event master_event;
    master_event.data.fd = master_socket;
    master_event.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, master_socket, &master_event);
}

void addSlaveSocket(int slave_socket)
{
    struct epoll_event slave_event;
    slave_event.data.fd = slave_socket;
    slave_event.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, slave_socket, &slave_event);
    slave_sockets.insert(slave_socket);
}

void removeSlaveSocket(int slave_socket)
{
    shutdown(slave_socket, SHUT_RDWR);
    close(slave_socket);
    slave_sockets.erase(slave_socket);
}

void sendMessageInChat(char *buffer)
{
    for (int slave_socket : slave_sockets)
    {
        send(slave_socket, buffer, BUFFER_LEN, 0);
    }
    
    printf("%s\n", buffer);
}

int main()
{
    printfStatus("запуск сервера");
    startServer();

    struct sigaction close_server_action;
    memset(&close_server_action, 0, sizeof(close_server_action));
    close_server_action.sa_handler = closeServerHandler;
    sigaction(SIGTERM, &close_server_action, NULL);
    sigaction(SIGINT, &close_server_action, NULL);

    printfStatus("сервер запущен");
    while (true)
    {
        struct epoll_event events[MAX_EPOOL_EVENTS];
        int events_num = epoll_wait(epoll_fd, events, MAX_EPOOL_EVENTS, -1);

        for (int i = 0; i < events_num; i++) 
        {
            if (events[i].data.fd == master_socket) {
                addSlaveSocket(accept(master_socket, 0, 0));
                continue;
            }

            int slave_socket = events[i].data.fd;
            char buffer[BUFFER_LEN] = {0};
            int recv_res = recv(slave_socket, buffer, BUFFER_LEN, 0);

            if (recv_res == 0) {
                removeSlaveSocket(slave_socket);
                continue;
            } 
            
            if (recv_res > 0) {
                sendMessageInChat(buffer);
                continue;
            }
        }
    }

    closeServer();

    return 0;
}