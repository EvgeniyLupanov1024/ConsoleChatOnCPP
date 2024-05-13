#pragma once

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <ctime>
#include <iomanip>
#include <iostream>

class Logger
{
private:
    std::string _log_file_path;
    int _log_fd;
public:
    Logger(const char *log_file_path);
    ~Logger();
    void writeLog(const char *text);
};
