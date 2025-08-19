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

    logInfo("🐟 Client connected on socket %d", client_sock);

    while ((n = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        _read_buffer[client_sock].append(buffer, n);
    }
    if (n < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
        logError("Failed to receive data from client");
        _cleanup_client(client_sock);
        return;
    }

    if (n == 0) {
        if (_request_complete(_read_buffer[client_sock])) {
            logInfo("🐠 Request complete from client socket %d (on close)", client_sock);
            _write_buffer[client_sock] = prepare_response(_read_buffer[client_sock]);
        } else {
            logError("Client disconnected before sending full request on socket %d. Sending 400.", client_sock);
            _write_buffer[client_sock] = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request</h1>";
        }
        _bytes_sent[client_sock] = 0;
        FD_SET(client_sock, &_write_fds);
        return;
    }

    if (!_request_complete(_read_buffer[client_sock])) {
        logInfo("🐠 Partial request received from client socket %d, waiting for more data...", client_sock);
        return;
    }
    logInfo("🐠 Request complete from client socket %d", client_sock);
    _write_buffer[client_sock] = prepare_response(_read_buffer[client_sock]);
    _bytes_sent[client_sock] = 0;
    FD_SET(client_sock, &_write_fds);
}

bool ServerManager::_request_complete(const std::string& request) {
    size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false; // Headers incompletos

    size_t cl_pos = request.find("Content-Length:");
    if (cl_pos == std::string::npos)
        return true; // No hay body, solo headers

    size_t cl_end = request.find("\r\n", cl_pos);
    std::string cl_str = request.substr(cl_pos + 15, cl_end - (cl_pos + 15));
    cl_str.erase(0, cl_str.find_first_not_of(" \t"));
    cl_str.erase(cl_str.find_last_not_of(" \t") + 1);
    int content_length = atoi(cl_str.c_str());

    size_t body_start = header_end + 4;
    size_t body_size = request.size() - body_start;

    std::cout << "[DEBUG] header_end: " << header_end << std::endl;
    std::cout << "[DEBUG] content_length: " << content_length << std::endl;
    std::cout << "[DEBUG] body_start: " << body_start << std::endl;
    std::cout << "[DEBUG] body_size: " << body_size << std::endl;

    return body_size >= (size_t)content_length;
}

std::string ServerManager::prepare_response(const std::string& request) {
    Request req(request);
    std::string file_path;

    if (req.getMethod() == "POST" && req.getPath() == "/upload") {
        std::cout << "[DEBUG] Body size: " << req.getBody().size() << std::endl;
        Cgi cgi("cgi-bin/saveFile.py");
        std::string cgi_output = cgi.run(req);

        // Redirección tras POST
        std::string response = "HTTP/1.1 303 See Other\r\n";
        response += "Location: /\r\n";
        response += "Content-Length: 0\r\n";
        response += "\r\n";
        return response;
    }
    else if (req.getMethod() == "DELETE" ) {
        std::cout << "[DEBUG] path: " << req.getPath() << std::endl;
        Cgi cgi("cgi-bin/deleteFile.py");
        std::string cgi_output = cgi.run(req);
        
        // Redirección tras POST
        std::string response = "HTTP/1.1 303 See Other\r\n";
        response += "Location: /\r\n";
        response += "Content-Length: 0\r\n";
        response += "\r\n";
        return response;
    }
    else {
        if (req.getPath() == "/" || req.getPath() == "/index.html") {
            return generate_index();
        } else {
            file_path = "www" + req.getPath();
        }
    }

    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
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

std::string ServerManager::generate_index() {
    std::ifstream file("www/index.html");
    if (!file.is_open()) {
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string html = buffer.str();

    Cgi cgi("cgi-bin/getIndex.py");
    std::string galeria = cgi.run(Request("GET / / HTTP/1.1\r\n\r\n")); // Puedes pasar un Request vacío o uno simulado

    size_t pos = html.find("<!--GALERIA-->");
    if (pos != std::string::npos)
        html.replace(pos, std::string("<!--GALERIA-->").length(), galeria);

    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + std::to_string(html.size()) + "\r\n";
    response += "\r\n";
    response += html;
    return response;
}