#include "../include/WebServ.hpp"

void logInfo(const char* msg, ...)
{

    char buffer[1024];

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    std::cout << RESET << buffer << RESET << "\n";
}

void logError(const char* msg, ...)
{

    char buffer[1024];

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    std::cerr << RED << buffer << RESET << "\n";
}

void logDebug(const char* msg, ...)
{
    if (!DEBUG)
        return;

    char buffer[1024];

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    std::cout << BLUE << buffer << RESET << "\n";
}