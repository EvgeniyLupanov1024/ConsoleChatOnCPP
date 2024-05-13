#include "logger.hpp"

Logger::Logger(const char *log_file_path)
{
    _log_file_path = std::string(log_file_path);
    _log_fd = open(_log_file_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
}

Logger::~Logger()
{
    close(_log_fd);
}

void Logger::writeLog(const char *text)
{
    std::ostringstream buffer;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    buffer << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "]" << text << "\n";
    std::string log_line = buffer.str();

    flock(_log_fd, LOCK_EX);
    write(_log_fd, log_line.c_str(), log_line.size());
    flock(_log_fd, LOCK_UN);
}
