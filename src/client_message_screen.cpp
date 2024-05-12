#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <iostream>

#include <map>

#define BUFFER_LEN 256
#define FIFO_SCREEN_NAME "fifo_client_msg"

typedef unsigned char user_id_t;

int get_msg_fd;
void closeMessageScreen()
{
    close(get_msg_fd);
    exit(EXIT_SUCCESS);
}

void closeMessageScreenHandler(int signum)
{
    closeMessageScreen();
} 

std::map<user_id_t, std::string> members;
void printMessage(user_id_t id, char *message)
{
    std::cout << '[' << members[id] << "] " << message << std::endl;
}

void addMember(user_id_t id, char *name)
{
    members.insert({id, std::string(name)});
}

void removeMember(user_id_t id)
{
    delete &members[id];
    members.erase(id);
}

int main(int argc, char **argv)
{
    if (argc < 1) {
        fprintf(stderr, "Недостатоно аргументов");
        pause();
    }
    
    int client_pid = atoi(argv[1]);
    kill(client_pid, SIGUSR1);

    get_msg_fd = open(FIFO_SCREEN_NAME, O_RDWR);
    if (get_msg_fd == -1) {
        fprintf(stderr, "Невозможно открыть fifo канал");
        pause();
    }

    struct sigaction close_message_screen_action = {0};
    close_message_screen_action.sa_handler = closeMessageScreenHandler;
    sigaction(SIGTERM, &close_message_screen_action, NULL);
    sigaction(SIGINT, &close_message_screen_action, NULL);

    while(true)
    {
        char read_buffer[sizeof(user_id_t) + BUFFER_LEN] = {0};
        int read_res = read(get_msg_fd, read_buffer, sizeof(user_id_t) + BUFFER_LEN);
        if (read_res == 0) {
            closeMessageScreen();
        }

        user_id_t id = *read_buffer;
        char *message = read_buffer + sizeof(user_id_t);

        if (members.find(id) == members.end()) {
            addMember(id, message);
            std::cout << "Пользователь \"" << message << "\" вошёл в чат" << std::endl;
            continue;
        }

        if (strcmp(message, "exit") == 0) {
            std::cout << "Пользователь \"" << members[id] << "\" вышёл из чата" << std::endl;
            removeMember(id);
            continue;
        }

        printMessage(id, message);
    }

    return 0;
}