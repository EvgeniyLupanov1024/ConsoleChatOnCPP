#pragma once

#include <string>

class Logger
{
private:
    std::string _log_file_path;
public:
    Logger(const char *log_file_path);
    void writeLog(const char *text);
};
