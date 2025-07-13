#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#define RESET "\x1B[0m"
#define RED   "\x1B[31m"
#define BLUE  "\x1B[34m"

#define DEBUG 1


void logDebug(const char* msg, ...);
void logInfo(const char* msg, ...);
void logError(const char* msg, ...);


#endif
