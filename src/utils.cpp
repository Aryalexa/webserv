#include "../include/WebServ.hpp"


void set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
		Message::logMessage(
            "Failed to get flags for socket %d: %s",
			sock,
			strerror(errno)
        );
        return;
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set non-blocking");
    }
}