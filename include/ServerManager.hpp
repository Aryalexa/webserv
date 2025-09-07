#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../include/WebServ.hpp"

# define CONNECTION_TIMEOUT 5

class Server;

struct ClientRequest {
    std::string buffer;       // Acumula los datos recibidos
    size_t max_size;          // client_max_body_size de la location / server
    size_t current_size;      // total bytes recibidos hasta ahora
    size_t content_length;    // Content-Length declarado por el cliente
    std::string request_path; // Para saber qué location aplica
    bool headers_parsed;      // Si ya leíste los headers

    ClientRequest();
    void append_to_buffer(std::string str);
};

class ServerManager {
    private:
        std::vector<Server>    _servers;
        std::map<int,Server>   _servers_map;
        std::map<int, int>          _client_server_map; // Buffer for incoming requests


        static bool _running;
        // select sets
        fd_set _read_fds;
        fd_set _write_fds;
        int _max_fd; // Maximum file descriptor for select

        // Buffers
        std::map<int, ClientRequest> _read_requests;
        std::map<int, std::string> _write_buffer;
        std::map<int, size_t> _bytes_sent;


        ServerManager(const ServerManager &other);
        ServerManager &operator=(const ServerManager &other);

        void _init_server_unit(Server server);
        int _get_client_server_fd(int client_socket) const;
        
        void resolve_path(Request &request, int client_socket);
        const Location* _find_best_location(const std::string& request_path, const std::vector<Location> &locations) const;
        void _apply_location_config(
            const Location *loc,
            std::string &root,
            std::string &index,
            bool &autoindex,
            std::string &full_path,
            const std::string &request_path,
            const std::string &request_method,
            bool &used_alias
        );
        void _handle_directory_case(
            std::string &full_path,
            const std::string &request_path,
            const std::string &index,
            bool autoindex,
            Request &request
        );
        void _apply_redirection(const Location *loc);


        std::string prepare_response(int client_socket, const std::string& request);
        std::string prepare_error_response(int client_socket, int code);
        
        
        static void _handle_signal(int signal);
        void _handle_new_connection(int listening_socket);
        void _handle_read(int client_sock);
        void _handle_write(int client_sock);
        void _cleanup_client(int client_sock);
        bool _request_complete(const std::string& request);
        bool _should_close_connection(const std::string& request, const std::string& response);

    public:
        ServerManager();
        ~ServerManager();

        void setup(const std::vector<Server>& servers);
        void init();
};

#endif
