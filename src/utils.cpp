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

HttpRequest parse_http_request(const std::string &request_str) {
    HttpRequest request;
    try {
        request = HttpRequest(request_str);
    } catch (const std::exception &e) {
        // bad request
    }
    return request;
}

int     ft_stoi(std::string str)
{
	std::stringstream ss(str);
	if (str.length() > 10)
		throw std::exception();
	for (size_t i = 0; i < str.length(); ++i)
	{
		if(!isdigit(str[i]))
			throw std::exception();
	}
	int res;
	ss >> res;
	return (res);
}