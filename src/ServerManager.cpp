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
void ServerManager::_init_server_unit(ServerSetUp server) {
    // listen
    int fd = server.getFd();
    if (listen(fd, BACKLOG_SIZE) < 0) {
        logInfo("listen(%d) failed: %s", fd, strerror(errno));
        exit(ERROR);
    }
    // Add to the read set
    FD_SET(fd, &_read_fds);
    // Update _max_fd
    if (fd > _max_fd) _max_fd = fd;
    logInfo("🐡 Server started on port %d", server.getPort());

}

int ServerManager::_get_client_server_fd(int client_socket) const {
    std::map<int, int>::const_iterator it = _client_server_map.find(client_socket);
    if (it == _client_server_map.end()) {
        return -1;
    }
    return it->second;
}


void ServerManager::init()
{
    _running = true;
    signal(SIGINT, ServerManager::_handle_signal); // Handle Ctrl+C

    FD_ZERO(&_read_fds);
	FD_ZERO(&_write_fds);

    for (size_t i = 0; i < _servers.size(); ++i) 
        _init_server_unit(_servers[i]); // Initialize each server unit

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

    _client_server_map[client_sock] = listening_socket; // Map client socket to server socket
    _read_buffer[client_sock] = ""; // Initialize read buffer for the new client
    _write_buffer[client_sock] = ""; // Initialize write buffer for the new client
    _bytes_sent[client_sock] = 0; // Initialize bytes sent for the new client
    logInfo("🐠 New connection accepted on socket %d. Listening socket: %d", client_sock, listening_socket);
}

void ServerManager::_handle_write(int client_sock) {

    std::string remaining_response = _write_buffer[client_sock].substr(_bytes_sent[client_sock]);
    logInfo("🐠 Sending response to client socket %d", client_sock);
    size_t n = send(client_sock, remaining_response.c_str(), remaining_response.size(), 0);

    if (n <= 0) {
        _cleanup_client(client_sock);
        logError("Failed to send data to client socket %d: %s. Connection closed.", client_sock, strerror(errno));
        return;
    }
    _bytes_sent[client_sock] += n;
    if (_bytes_sent[client_sock] == _write_buffer[client_sock].size()) {
       if (_should_close_connection(_read_buffer[client_sock], _write_buffer[client_sock])) {
            _cleanup_client(client_sock);
        } else {
            // Mantener la conexión: limpiar buffers y volver a modo lectura
            _read_buffer[client_sock].clear();
            _write_buffer[client_sock].clear();
            _bytes_sent[client_sock] = 0;
            FD_CLR(client_sock, &_write_fds);
            FD_SET(client_sock, &_read_fds);
        }
    }
}
bool ServerManager::_should_close_connection(const std::string& request, const std::string& response) {
    // Check if the request or response contains "Connection: close"
    bool closing = false;
    if (in_str(request, "Connection: close")){
        logError("🐠 Should close connection. From request");
        closing = true;
    } 
    if (in_str(response, "Connection: close")) {
        logError("🐠 Should close connection. From response");
        closing = true;
    }
    return closing;
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
            _write_buffer[client_sock] = prepare_response(client_sock, _read_buffer[client_sock]);
        } else {
            logError("Client disconnected before sending full request on socket %d. Sending 400.", client_sock);
            _write_buffer[client_sock] = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<h1>400 Bad Request</h1>";
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
    _write_buffer[client_sock] = prepare_response(client_sock, _read_buffer[client_sock]);
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

const Location* ServerManager::_find_best_location(const std::string& request_path, const std::vector<Location> &locations) const{
    const Location* best_match = NULL;
    size_t best_len = 0;

    for (size_t i = 0; i < locations.size(); ++i) {
        const Location& loc = locations[i];

        std::string loc_path = loc.getPathLocation();
        if (path_matches(loc_path, request_path)) {
            // Preferimos el que tenga el prefix MÁS LARGO
            if (loc_path.size() > best_len) {
                best_match = &loc;
                best_len = loc_path.size();
            }
        }
    }
    return best_match; // puede ser NULL si ninguno matchea
}
void ServerManager::_apply_location_config(
    const Location *loc,
    std::string &root,
    std::string &index,
    bool &autoindex,
    std::string &full_path,
    const std::string &request_path,
    const std::string &request_method,
    bool &used_alias
) {
    if (!loc)
        return;

    logDebug("🍍 Location matched: %s", loc->getPathLocation().c_str());

    // Verificar métodos permitidos
    if (!loc->getMethods()[method_toEnum(request_method)])
        throw HttpException(HttpStatusCode::MethodNotAllowed);

    // Root
    if (!loc->getRootLocation().empty())
        root = loc->getRootLocation();

    // Alias
    if (!loc->getAlias().empty()) { // ONLY ABSOLUTE ALIAS SUPPORTED
        full_path = loc->getAlias() + request_path.substr(loc->getPathLocation().size());
        used_alias = true;
        logDebug("🍍 Using alias: %s -> %s", request_path.c_str(), full_path.c_str());
    }

    // Index
    if (!loc->getIndexLocation().empty())
        index = loc->getIndexLocation();

    // Autoindex
    if (loc->getAutoindex() != autoindex)
        autoindex = loc->getAutoindex();
}

void ServerManager::_handle_directory_case(
    std::string &full_path,
    const std::string &request_path,
    const std::string &index,
    bool autoindex,
    Request &request
) {
    if (ConfigFile::getTypePath(full_path) != F_DIRECTORY)
        return;
    logDebug("🍍 Handling directory case for path: %s", full_path.c_str());

    // without trailing slash -> redirect
    if (request_path.back() != '/') {
        std::string new_location = request_path + "/";
        throw HttpExceptionRedirect(HttpStatusCode::MovedPermanently, new_location);
    }

    // index defined and exists -> use it
    if (!index.empty()) { // index defined
        std::string index_path = full_path + index;
        if (ConfigFile::getTypePath(index_path) == F_REGULAR_FILE) { 
            full_path = index_path;
            logDebug("🍍 Directory index found: %s", index_path.c_str());
            return;
        }
    }
    // no index defined or not found
    if (autoindex) {
        // autoindex ON
        logDebug("🍍 Autoindex enabled for directory %s", full_path.c_str());
        request.setAutoindex(true);
    }
    else {
        // No hay index y autoindex está deshabilitado → 403
        throw HttpException(HttpStatusCode::Forbidden);
    }
    
}

void ServerManager::_apply_redirection(const Location *loc) {
    if (loc && !loc->getReturn().empty()) {
        std::string new_location = loc->getReturn();
        throw HttpExceptionRedirect(HttpStatusCode::MovedPermanently, new_location);
    }
    // else if (server.hasReturn()) {...}

    /*
    NGINX does:

    location /old {
        return 301 /new;
    }
    location /other_old {
        return 302 http://example.com/other_new;
    }

    --> o sea, con codigo de redireccion 
    ademas!! acepta return en server block y nosotros no.
    */
}

/**
 * Resolve the request path based on server configuration.
 * index, root, alias, return, etc.
 */
void ServerManager::resolve_path(Request &request, int client_socket) {
    int server_fd = _get_client_server_fd(client_socket);
    if (server_fd == -1) {
        logError("resolve_path: client_socket %d not found in _client_server_map!", client_socket);
        throw HttpException(HttpStatusCode::InternalServerError);
    }

    ServerSetUp &server = _servers_map[server_fd];
    std::string path = request.getPath();
    path = path_normalization(clean_path(path));
    
    // server block config
    std::string root = server.getRoot();
    std::string index = server.getIndex();
    bool autoindex = server.getAutoindex();
    bool used_alias = false;
    std::string full_path;
    
    // 1. search best location and apply
    const Location *loc = _find_best_location(path, server.getLocations());
    _apply_redirection(loc);
    _apply_location_config(loc, root, index, autoindex, full_path, path, request.getMethod(), used_alias);

    if (!used_alias)
        full_path = root + path;

    // 4. Gestionar directorios, autoindex e index
    _handle_directory_case(full_path, path, index, autoindex, request);

    // checking the file existence is done by the methods resolver.
    
    // finally, set the resolved path
    request.setPath(full_path);
}

std::string ServerManager::prepare_response(int client_socket, const std::string &request_str) {
    std::string response_str;

    try {
        Request request(request_str);
        logDebug("🍅 Request parsed. Query: %s: %s",request.getMethod().c_str(),request.getPath().c_str());
        resolve_path(request, client_socket);
        logDebug("🍅 preparing response. client socket: %i. Query: %s %s",
            client_socket, request.getMethod().c_str(), request.getPath().c_str());
        HttpResponse response(&request);
        response_str = response.getResponse();
    } catch (const HttpExceptionRedirect &e) {
        int code = e.getStatusCode();
        std::string location = e.getLocation();
        std::cout << "Redirection Exception caught: " << e.what() << ", Location: " << location << std::endl;
        HttpResponse response(code, location);
        response_str = response.getResponse();
    } catch (const HttpException &e) {
        int code = e.getStatusCode();
        std::cout << "Excepción capturada: " << e.what() << std::endl;
        response_str = prepare_error_response(client_socket, code);
        
    } catch (const std::exception &e) {
        // raise exc?
        logError("Exception: %s", e.what());
        //int code = HttpStatusCode::InternalServerError; // Default to 500 Internal Server
        exit(1);
    }

    return response_str;
}

std::string ServerManager::prepare_error_response(int client_socket, int code) {
    logInfo("Preparing error response. client socket %i. error %d", client_socket, code);
    std::string response_str;
    // first: try error page in config
    int server_fd = _get_client_server_fd(client_socket); // TODO: get server fd from client socket
    if (server_fd == -1) {
        logError("prepare_error_response: client_socket %d not found in _client_server_map!", client_socket);
        // Devuelve una respuesta de error de servidor genérica
        HttpResponse response(HttpStatusCode::InternalServerError);
        return response.getResponse();
    }
    ServerSetUp &server = _servers_map[server_fd];
    std::string err_page_path = server.getPathErrorPage(code);
    if (!err_page_path.empty()) {
        logInfo("🍊 Acción: Mostrar página de error %d desde %s", code, err_page_path.c_str());
        HttpResponse response(code, WWW_ROOT + err_page_path);
        response_str = response.getResponse();
        return response_str;
    }
    logDebug("---prepare_error_response: error page for code %d not found in server config", code);
    // if not found, treat web server error
    std::string message = statusCodeString(code);
    switch (code) {
        case HttpStatusCode::NotFound:
        case HttpStatusCode::Forbidden:
            {
                // show error page
                logError("🍊 Acción: Mostrar página de error %d.", code);
                HttpResponse response(code);
                response_str = response.getResponse();
            }
            break;
        case HttpStatusCode::InternalServerError:
            logError("Error. %s. Acción: Revisar los registros del servidor.", message.c_str());
            exit(2);
            break;
        case HttpStatusCode::BadRequest:
            logError("Error. %s. Acción: Validar la solicitud del cliente.", message.c_str());
            exit(2);
            break;
        default:
            logError("Error no gestionado: %s. hacer algo!.", message.c_str());
            exit(2);
            break;
    }
    return response_str;
}

void ServerManager::_cleanup_client(int client_sock) {
    FD_CLR(client_sock, &_read_fds);
    FD_CLR(client_sock, &_write_fds);
    close(client_sock);
    _client_server_map.erase(client_sock);
    _read_buffer.erase(client_sock);
    _write_buffer.erase(client_sock);
    _bytes_sent.erase(client_sock);
    logInfo("🐟 Client socket %d cleaned up", client_sock);
}

void ServerManager::_handle_signal(int signal) {
	(void)signal;
    ServerManager::_running = false;
}
