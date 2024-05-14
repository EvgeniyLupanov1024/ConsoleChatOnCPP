#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <map>
#include <iostream>

#include "fifo_channel.hpp"

#define BUFFER_LEN 256
#define FIFO_SCREEN_PATH "./tmp/fifo_message_screen"

typedef unsigned char user_id_t;

FifoChannel *get_message_channel;

void closeMessageScreen()
{
    delete get_message_channel;
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

    get_message_channel = new FifoChannel(FIFO_SCREEN_PATH, client_pid);
    
    struct sigaction close_message_screen_action = {0};
    close_message_screen_action.sa_handler = closeMessageScreenHandler;
    sigaction(SIGTERM, &close_message_screen_action, NULL);
    sigaction(SIGINT, &close_message_screen_action, NULL);

    while(true)
    {
        char read_buffer[sizeof(user_id_t) + BUFFER_LEN] = {0};
        get_message_channel->readFrom(read_buffer, sizeof(user_id_t) + BUFFER_LEN);

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