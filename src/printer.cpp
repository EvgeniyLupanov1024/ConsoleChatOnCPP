#include "printer.hpp"

void printfStatus(const char *status_text, ...)
{
    std::ostringstream buffer;
    buffer << "-- " << status_text << "...\n";
    std::string status_line = buffer.str();
    
    va_list args;
    va_start(args, status_text);
    vprintf(status_line.c_str(), args);
    va_end(args);
}