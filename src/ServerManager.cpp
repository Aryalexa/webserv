#include "../include/WebServ.hpp"

bool ServerManager::_running = true; // Initialize the static running variable

ServerManager::ServerManager()
  : _max_fd(0)
{
}

ServerManager::~ServerManager(){}

void ServerManager::setup(const std::vector<ServerSetUp>& configs) {

    _servers = configs;
    
    for (size_t i = 0; i < _servers.size(); ++i) 
    {
        ServerSetUp &server = _servers[i];
        bool reused = false;

        for (size_t j = 0; j < i; ++j) {
            const ServerSetUp &prev = _servers[j];
            if (prev.getHost() == server.getHost() &&
                prev.getPort() == server.getPort())
            {
                server.setFd(prev.getFd());
                reused = true;
                break;
            }
        }

        if (!reused) {
            server.setUpIndividualServer();
        }

        _servers_map[server.getFd()] = server;

        char ipbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &server.getHost(), ipbuf, sizeof(ipbuf));
        logInfo(
            "Server %s bound to http://%s:%d, fd = %d",
            server.getServerName().c_str(),
            ipbuf,
            server.getPort(),
            server.getFd()
        );
    }
}

void ServerManager::init()
{
    _running = true;
    signal(SIGINT, ServerManager::_handle_signal); // Handle Ctrl+C

    FD_ZERO(&_read_fds);
	FD_ZERO(&_write_fds);

    for (size_t i = 0; i < _servers.size(); ++i) 
    {
        int fd = _servers[i].getFd();

        if (listen(fd, BACKLOG_SIZE) < 0) {
            logInfo("listen(%d) failed: %s", fd, strerror(errno));
            exit(ERROR);
        }

        FD_SET(fd, &_read_fds);    // Add to the read set
        if (fd > _max_fd) _max_fd = fd;  // Update _max_fd

        logInfo("🐡 Server started on port %d", _servers[i].getPort());
    }

    fd_set temp_read_fds;
    fd_set temp_write_fds;
    while (_running) {
        temp_read_fds = _read_fds;
        temp_write_fds = _write_fds;

        int activity = select(_max_fd + 1, &temp_read_fds, &temp_write_fds, NULL, NULL);
        if (activity < 0) {
            if (errno == EINTR) continue; // Interrupted by signal
            logInfo("Failed to select on sockets");
        }

        for (int fd = 0; fd <= _max_fd; ++fd) {
            if (FD_ISSET(fd, &temp_read_fds)) {
                if (_servers_map.find(fd) != _servers_map.end()) {
                    // The fd belongs to a server that has a new connection
                    _handle_new_connection(fd);
                } else {
                    // The fd belongs to a client that is sending data
                    _handle_read(fd);
                }
            }
            if (FD_ISSET(fd, &temp_write_fds)) {
                // The fd belongs to a client that is ready to write data
                _handle_write(fd);
            }
        }
    }
    logInfo("\nServer shutting down...");
}

void ServerManager::_handle_new_connection(int listening_socket) {
    int client_sock = accept(listening_socket, NULL, NULL);
    if (client_sock < 0) {
        logError("Failed to accept new connection on socket %d: %s", listening_socket, strerror(errno));
        return;
    }

    if (client_sock >= FD_SETSIZE) {
        logError("Too many open files, cannot accept new connection on socket %d", listening_socket);
        close(client_sock);
        return;
    }

    set_nonblocking(client_sock);
    FD_SET(client_sock, &_read_fds);
    if (client_sock > _max_fd) _max_fd = client_sock;

    _read_buffer[client_sock] = ""; // Initialize read buffer for the new client
    _write_buffer[client_sock] = ""; // Initialize write buffer for the new client
    _bytes_sent[client_sock] = 0; // Initialize bytes sent for the new client
    logInfo("🐠 New connection accepted on socket %d. Listening socket: %d", client_sock, listening_socket);
}

void ServerManager::_handle_write(int client_sock) {

    std::string remaining_response = _write_buffer[client_sock].substr(_bytes_sent[client_sock]);
    logInfo("🐠 Sending response to client socket %d", client_sock);
    logDebug("🐠 Sending response: %s", remaining_response.c_str());
    size_t n = send(client_sock, remaining_response.c_str(), remaining_response.size(), 0);

    if (n <= 0) {
        _cleanup_client(client_sock);
        logError("Failed to send data to client socket %d: %s. Connection closed.", client_sock, strerror(errno));
        return;
    }
    _bytes_sent[client_sock] += n;
    if (_bytes_sent[client_sock] == _write_buffer[client_sock].size())
        _cleanup_client(client_sock);
}
void ServerManager::_handle_read(int client_sock) {
    char buffer[BUFFER_SIZE];
    int n;
    //HttpRequest request;
    std::string response;

    logInfo("🐟 Client connected on socket %d", client_sock);

    n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (n < 0) {
        logError("Failed to receive data from client");
        exit(1);
    } else if (n == 0) {
        _cleanup_client(client_sock);
        logError("Client disconnected on socket %d. Connection closed.", client_sock);
        return; // Client disconnected
    }
    buffer[n] = '\0'; // Null-terminate the received data
    _read_buffer[client_sock] += buffer; // Store the request in the read buffer
   
    if (!_request_complete(_read_buffer[client_sock])) {
        logInfo("🐠 Partial request received from client socket %d, waiting for more data...", client_sock);
        return; // Wait for more data to complete the request
    }
    // Request is complete, process it
    logInfo("🐠 Request complete from client socket %d", client_sock);
    logDebug("🐠 Request: %s", _read_buffer[client_sock].c_str());
    //request = parse_http_request(_read_buffer[client_sock]);
    _write_buffer[client_sock] = prepare_response(_read_buffer[client_sock]);

    _bytes_sent[client_sock] = 0; // Reset bytes sent for this client
    //FD_CLR(client_sock, &_read_fds); // only if client disconnects
    FD_SET(client_sock, &_write_fds);
}

bool ServerManager::_request_complete(const std::string& request) {
    // Check if the request is complete by looking for the end of headers
    // A complete HTTP request ends with "\r\n\r\n"
    return request.find("\r\n\r\n") != std::string::npos;
}

std::string ServerManager::prepare_response(const std::string& request) {

    Request req(request);
    std::cout << request << std::endl;
    std::cout << req << std::endl;

    std::cout << "Método: " << req.getMethod() << std::endl;
    std::cout << "Path: " << req.getPath() << std::endl;
    std::cout << "Request: " << req << std::endl;

    std::string file_path;
    if (req.getPath() == "/" || req.getPath().empty()) {
        file_path = "www/index.html";
    } else {
        file_path = "www" + req.getPath();
    }

    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        // Manejar error: archivo no encontrado
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string body = buffer.str();

    std::string content_type = "text/html";
    if (file_path.find(".css") != std::string::npos) content_type = "text/css";
    else if (file_path.find(".js") != std::string::npos) content_type = "application/javascript";
    else if (file_path.find(".jpg") != std::string::npos || file_path.find(".jpeg") != std::string::npos) content_type = "image/jpeg";
    else if (file_path.find(".png") != std::string::npos) content_type = "image/png";

    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    response += "\r\n";
    response += body;



    /*
    - Find the correct resource or action
    - Check HTTP method rules
    - Apply redirections or aliases
    - Locate the file or resource
    - Consider CGI (dynamic content)
    - Determine the response status
    */
    // generate a response
    //response = HttpResponse(); // TODO: args
    return response;
}

void ServerManager::_cleanup_client(int client_sock) {
    FD_CLR(client_sock, &_read_fds);
    FD_CLR(client_sock, &_write_fds);
    close(client_sock);
    _read_buffer.erase(client_sock);
    _write_buffer.erase(client_sock);
    _bytes_sent.erase(client_sock);
    logInfo("🐟 Client socket %d cleaned up", client_sock);
}

void ServerManager::_handle_signal(int signal) {
	(void)signal;
    ServerManager::_running = false;
}