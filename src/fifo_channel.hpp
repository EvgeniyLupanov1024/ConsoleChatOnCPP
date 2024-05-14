#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <string>
#include <sstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>

class FifoChannel
{
private:
    std::string _channel_path;
    int _channel_fd;
public:
    FifoChannel(const char *channel_path, int id = 0);
    ~FifoChannel();
    ssize_t writeIn(const char *text, size_t number_byte = 0);
    ssize_t readFrom(void *buffer, size_t number_byte);
};
