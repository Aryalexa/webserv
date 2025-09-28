#include "../include/WebServ.hpp"

/**
 Add traces to logging
To add traces (file, line, function) to your error messages and exceptions in C++98, you can use the predefined macros:

__FILE__ — current file name
__LINE__ — current line number
__FUNCTION__ — current function name (supported by most compilers, including GCC)
*/


void logInfo(const char* msg, ...)
{

    char buffer[1024];

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    std::cout << GREEN << buffer << RESET << "\n";
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