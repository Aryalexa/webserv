#include "../headers/WebServ.hpp"

void Message::logError(const char* msg, ...)
{

    char buffer[1024];

    va_list args;
    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    std::cout << RED << buffer << RESET << "\n";
}
