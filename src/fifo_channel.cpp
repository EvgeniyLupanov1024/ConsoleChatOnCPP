#include "fifo_channel.hpp"

FifoChannel::FifoChannel(const char *channel_path, int id)
{
    if (id == 0) {
        _channel_path = channel_path;
    } else {
        std::ostringstream buffer;
        buffer << channel_path << id;
        _channel_path = buffer.str();
    }

    int mkfifo_res = mkfifo(_channel_path.c_str(), 0666);
    if (mkfifo_res == -1 && errno != EEXIST) {
        fprintf(stderr, "Невозможно создать fifo канал: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    _channel_fd = open(_channel_path.c_str(), O_RDWR);
    if(_channel_fd == -1) {
        fprintf(stderr, "Невозможно открыть fifo канал: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

FifoChannel::~FifoChannel()
{
    close(_channel_fd);
    unlink(_channel_path.c_str());
}

ssize_t FifoChannel::writeIn(const char *text, size_t number_byte)
{
    if (number_byte == 0) {
        number_byte = strlen(text);
    }
    return write(_channel_fd, text, number_byte);
}

ssize_t FifoChannel::readFrom(void *buffer, size_t number_byte)
{
    return read(_channel_fd, buffer, number_byte);
}
