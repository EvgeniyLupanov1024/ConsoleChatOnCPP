#include <iostream>
#include <unistd.h>
#include <set>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#define MAX_CONNECTIONS 5
#define BUFFER_LEN 256
#define MAX_EPOOL_EVENTS 32

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

    std::set<int> slave_sockets;

    int epoll_fd = epoll_create1(0);

    struct epoll_event master_event;
    master_event.data.fd = master_socket;
    master_event.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, master_socket, &master_event);

    while (true)
    {
        struct epoll_event events[MAX_EPOOL_EVENTS];
        int n = epoll_wait(epoll_fd, events, MAX_EPOOL_EVENTS, -1);

        for (int i = 0; i < n; i++) 
        {
            if (events[i].data.fd == master_socket) {
                int slave_socket = accept(master_socket, 0, 0);

                struct epoll_event slave_event;
                slave_event.data.fd = slave_socket;
                slave_event.events = EPOLLIN;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, slave_socket, &slave_event);
                slave_sockets.insert(slave_socket);

                continue;
            }

            int slave_socket = events[i].data.fd;
            char buffer[BUFFER_LEN] = {0};
            int recv_res = recv(slave_socket, buffer, BUFFER_LEN, 0);

            if (recv_res == 0 && errno != EAGAIN) {
                shutdown(slave_socket, SHUT_RDWR);
                close(slave_socket);
                slave_sockets.erase(slave_socket);
            } else if (recv_res > 0) {
                for (int slave_socket : slave_sockets)
                {
                    send(slave_socket, buffer, BUFFER_LEN, 0);
                }
                
                printf("%s\n", buffer);
            }
        }
    }

    shutdown(master_socket, SHUT_RDWR);
    close(master_socket);

    return 0;
}