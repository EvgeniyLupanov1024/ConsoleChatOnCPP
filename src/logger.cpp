#include "logger.hpp"

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

Logger::Logger(const char *log_file_path)
{
    _log_file_path = std::string(log_file_path);
}

void Logger::writeLog(const char *text)
{
    int log_fd = open(_log_file_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    flock(log_fd, LOCK_EX);
    write(log_fd, text, strlen(text));
    flock(log_fd, LOCK_UN);
    close(log_fd);
}
