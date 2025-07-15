#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../include/WebServ.hpp"

# define CONNECTION_TIMEOUT 5

class ServerSetUp;

class ServerManager {
    private:
        std::vector<ServerSetUp>           _servers;
        std::map<int,ServerSetUp>          _servers_map;
        // fd_set                              _recv_fd_pool;
        // fd_set                              _write_fd_pool;
        // int                                 _biggest_fd;


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
        std::string _generate_response(const std::string& request);
        bool _request_complete(const std::string& request);
        void _handle_read(int client_sock);
        void _handle_write(int client_sock);
        void _cleanup_client(int client_sock);
        static void _handle_signal(int signal);

    public:
        ServerManager();
        ~ServerManager();

        void setup(const std::vector<ServerSetUp>& servers);
        void init();
};

#endif
