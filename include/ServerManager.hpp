#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../include/WebServ.hpp"

# define CONNECTION_TIMEOUT 5

class ServerSetUp;

class ServerManager {
    private:
        std::vector<ServerSetUp>    _servers;
        std::map<int,ServerSetUp>   _servers_map;
        std::map<int, int>          _client_server_map; // Buffer for incoming requests


        static bool _running;
        // select sets
        fd_set _read_fds;
        fd_set _write_fds;
        int _max_fd; // Maximum file descriptor for select

        // Buffers
        std::map<int, std::string> _read_buffer;
        std::map<int, std::string> _write_buffer;
        std::map<int, size_t> _bytes_sent;


        ServerManager(const ServerManager &other);
        ServerManager &operator=(const ServerManager &other);

        void _handle_new_connection(int listening_socket);
        std::string prepare_response(int client_socket, const std::string& request);
        std::string prepare_error_response(int client_socket, int code, const Request &request);


        bool _request_complete(const std::string& request);
        bool _should_close_connection(const std::string& request, const std::string& response);
        void _handle_read(int client_sock);
        void _handle_write(int client_sock);
        void _cleanup_client(int client_sock);
        static void _handle_signal(int signal);
        void _init_server_unit(ServerSetUp server);
        int _get_client_server_fd(int client_socket) const;


    public:
        ServerManager();
        ~ServerManager();

        void setup(const std::vector<ServerSetUp>& servers);
        void init();
};

#endif
