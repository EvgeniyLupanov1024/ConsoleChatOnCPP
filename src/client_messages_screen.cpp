#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUFFER_LEN 256
#define FIFO_SCREEN_NAME "fifo_client_msg"

int main(int argc, char **argv)
{
    int get_msg_fd = open(FIFO_SCREEN_NAME, O_RDWR);
    if (get_msg_fd == -1) {
        fprintf(stderr, "Невозможно открыть fifo канал");
        pause();
    }

    char read_buffer[BUFFER_LEN] = {0};
    int read_res;
    while(true) 
    {
        read_res = read(get_msg_fd, read_buffer, BUFFER_LEN);
        if (read_res == 0) {
            close(get_msg_fd);
            exit(EXIT_SUCCESS);
        }

        printf("%s\n", read_buffer);
    }

    pause();
    return 0;
}