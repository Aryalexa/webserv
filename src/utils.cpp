#include "../include/WebServ.hpp"

/**
 * Set the socket to non-blocking mode.
 * This is useful for handling multiple connections without blocking.
 * TODO: is this really necessary when using select?
 */
void set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
		logInfo(
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