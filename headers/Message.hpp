#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#define RESET "\x1B[0m"
#define RED   "\x1B[31m"

class Message {
    public:
        static void logError(const char* msg, ...);
};

#endif
