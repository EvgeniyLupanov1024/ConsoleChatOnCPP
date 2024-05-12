#include <iostream>
#include <unistd.h>
#include <map>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <signal.h>

#define MAX_CONNECTIONS 5
#define BUFFER_LEN 256
#define MAX_EPOOL_EVENTS 32
#define MEMBER_NAME_MAX_LEN 25

typedef unsigned char user_id_t;

user_id_t last_user_id = 0x10;
struct UserInfo
{
    in_addr_t ip;
    user_id_t id;
    std::string name;
};

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
std::map<int, UserInfo> slave_sockets;
int epoll_fd;
void closeServer()
{
    printfStatus("завершение работы сервера");
    shutdown(master_socket, SHUT_RDWR);
    close(master_socket);

    for (std::pair<int, UserInfo> client_info : slave_sockets) {
        int slave_socket = client_info.first;
        shutdown(slave_socket, SHUT_RDWR);
        close(slave_socket);
    }

    close(epoll_fd);
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
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int bind_res = bind(
        master_socket,
        (struct sockaddr *) &sockaddr,
        sizeof sockaddr
    );
    if (bind_res == -1) {
        fprintf(stderr, "Ошибка привязки сокета: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int listen_res = listen(
        master_socket,
        MAX_CONNECTIONS
    );
    if (listen_res == -1) {
        fprintf(stderr, "Ошибка при попытке слушать сокет: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    epoll_fd = epoll_create1(0);

    struct epoll_event master_event;
    master_event.data.fd = master_socket;
    master_event.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, master_socket, &master_event);
}

void addSlaveSocket(int slave_socket, sockaddr_in client_addr)
{
    struct epoll_event slave_event;
    slave_event.data.fd = slave_socket;
    slave_event.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, slave_socket, &slave_event);

    UserInfo client_info {client_addr.sin_addr.s_addr, last_user_id++, ""};
    slave_sockets.insert({slave_socket, client_info});
}

void removeSlaveSocket(int slave_socket)
{
    shutdown(slave_socket, SHUT_RDWR);
    close(slave_socket);
    delete &slave_sockets[slave_socket].name;
    slave_sockets.erase(slave_socket);
}

void sendMessageInChat(char *buffer, user_id_t id)
{
    char message[sizeof(user_id_t) + BUFFER_LEN] = {'\0'};
    memcpy(message, &id, sizeof(user_id_t));
    strcpy(message + sizeof(user_id_t), buffer);

    int message_len = strlen(message);
    for (std::pair<int, UserInfo> client_info : slave_sockets)
    {
        send(client_info.first, message, message_len, 0);
    }
}

int main()
{
    printfStatus("запуск сервера");
    startServer();

    struct sigaction close_server_action = {0};
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
            if (events[i].data.fd == master_socket) { // пользователь подключился
                sockaddr_in client_addr = {0};
                socklen_t client_addr_size;
                int slave_socket = accept(master_socket, (sockaddr *)&client_addr, &client_addr_size);

                addSlaveSocket(slave_socket, client_addr);
                continue;
            }

            int slave_socket = events[i].data.fd;
            char recv_buffer[BUFFER_LEN] = {0};
            int recv_res = recv(slave_socket, recv_buffer, BUFFER_LEN, 0);

            if (recv_res == 0) { // пользователь отключился
                removeSlaveSocket(slave_socket);
                continue;
            } 
            
            if (recv_res > 0) { // сообщение от пользователя
                if (slave_sockets[slave_socket].name == "") {
                    slave_sockets[slave_socket].name = recv_buffer;
                }

                sendMessageInChat(recv_buffer, slave_sockets[slave_socket].id);
                continue;
            }
        }
    }

    closeServer();

    return 0;
}