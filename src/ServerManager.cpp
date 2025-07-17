#include "../include/WebServ.hpp"

bool ServerManager::running = true; // Initialize the static running variable

ServerManager::ServerManager()
  : max_fd(0)
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
        Message::logMessage(
            "Server %s bound to %s: %d, fd = %d",
            server.getServerName().c_str(),
            ipbuf,
            server.getPort(),
            server.getFd()
        );
    }
}

void ServerManager::init()
{
    ServerManager::running = true;
    signal(SIGINT, ServerManager::handle_signal); // Handle Ctrl+C

    FD_ZERO(&master_read_fds);
	FD_ZERO(&master_write_fds);

    for (size_t i = 0; i < _servers.size(); ++i) 
    {
        int fd = _servers[i].getFd();

        if (listen(fd, BACKLOG_SIZE) < 0) {
            Message::logMessage("listen(%d) failed: %s", fd, strerror(errno));
            exit(ERROR);
        }

        FD_SET(fd, &master_read_fds);    // Add to the read set
        if (fd > max_fd) max_fd = fd;  // Update max_fd

        std::cout << "🐡 Server started on port " << _servers[i].getPort() << std::endl;
    }


    while (running) {
        temp_read_fds = master_read_fds;
        temp_write_fds = master_write_fds;

        int activity = select(max_fd + 1, &temp_read_fds, &temp_write_fds, NULL, NULL);
        if (activity < 0) {
            if (errno == EINTR) continue; // Interrupted by signal
            Message::logMessage("Failed to select on sockets");
        }

        for (int i = 0; i <= max_fd; ++i) {
            if (FD_ISSET(i, &temp_read_fds)) {
                if (_servers_map.find(i) != _servers_map.end()) {
                    // Este fd pertenece a un servidor que está escuchando
                    handle_new_connection(i);
                } else {
                    // Handle readable client socket
                    handle_read(i);
                }
            }
            if (FD_ISSET(i, &temp_write_fds)) {
                // Handle writable client socket
                handle_write(i);
            }
        }
    }
}

void ServerManager::handle_new_connection(int listening_socket) {
    int client_sock = accept(listening_socket, NULL, NULL);
    if (client_sock < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
        return;
    }

    if (client_sock >= FD_SETSIZE) {
        std::cerr << "Too many clients connected. Rejecting connection." << std::endl;
        close(client_sock);
        return;
    }

    set_nonblocking(client_sock);
    FD_SET(client_sock, &master_read_fds);
    if (client_sock > max_fd) max_fd = client_sock;

    std::cout << "🐟 New client connected on socket " << listening_socket << std::endl;
}

void ServerManager::handle_write(int client_sock) {

    std::string remaining_response = write_buffer[client_sock].substr(bytes_sent[client_sock]);
    std::cout << "🐠 Sending response:\n" << remaining_response << std::endl;
    size_t n = send(client_sock, remaining_response.c_str(), remaining_response.size(), 0);

    if (n <= 0) {
        cleanup_client(client_sock);
        std::cerr << "Error sending data, closing socket." << std::endl;
        return;
    }
    bytes_sent[client_sock] += n;
    if (bytes_sent[client_sock] == write_buffer[client_sock].size())
        cleanup_client(client_sock);
}
void ServerManager::handle_read(int client_sock) {
    char buffer[BUFFER_SIZE];
    int n;
    std::string response;

    std::cout << "🐟 Client connected" << std::endl;

    n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (n < 0) {
        Message::logError("Failed to receive data from client");
        exit(1);
    } else if (n == 0) {
        cleanup_client(client_sock);
        return; // Client disconnecteda
        std::cerr << "Error receiving data, closing socket." << std::endl;
    }
    buffer[n] = '\0'; // Null-terminate the received data
    read_buffer[client_sock] += buffer; // Store the request in the read buffer
   
    if (!request_complete(read_buffer[client_sock])) {
        std::cout << "🐠 Partial request received, waiting for more data..." << std::endl;
        return; // Wait for more data to complete the request
    }
    // Request is complete, process it
    std::cout << "🐠 Request complete, preparing response..." << std::endl;
    write_buffer[client_sock] = prepare_response(read_buffer[client_sock]);
    bytes_sent[client_sock] = 0; // Reset bytes sent for this client
    //FD_CLR(client_sock, &master_read_fds); // only if client disconnects
    FD_SET(client_sock, &master_write_fds);
}

bool ServerManager::request_complete(const std::string& request) {
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

    return response;
}

void ServerManager::cleanup_client(int client_sock) {
    FD_CLR(client_sock, &master_read_fds);
    FD_CLR(client_sock, &master_write_fds);
    close(client_sock);
    read_buffer.erase(client_sock);
    write_buffer.erase(client_sock);
    bytes_sent.erase(client_sock);
    std::cout << "🐟 Client disconnected" << std::endl;
}

void ServerManager::handle_signal(int signal) {
	(void)signal;
    ServerManager::running = false;
}