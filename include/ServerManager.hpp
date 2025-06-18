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


        static bool running;
        // sets
        fd_set master_read_fds;
        fd_set master_write_fds;
        fd_set temp_read_fds;
        fd_set temp_write_fds;
        int max_fd; // Maximum file descriptor for select

        // Buffers
        std::map<int, std::string> read_buffer;
        std::map<int, std::string> write_buffer;
        std::map<int, size_t> bytes_sent;


        ServerManager(const ServerManager &other);
        ServerManager &operator=(const ServerManager &other);

        void handle_new_connection(int listening_socket);
        std::string prepare_response(const std::string& request);
        bool request_complete(const std::string& request);
        void handle_read(int client_sock);
        void handle_write(int client_sock);
        void cleanup_client(int client_sock);
        static void handle_signal(int signal);

    public:
        ServerManager();
        ~ServerManager();

        void setup(const std::vector<ServerSetUp>& servers);
        void init();
};

#endif
